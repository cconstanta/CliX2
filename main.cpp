#include <QApplication>
#include <QMessageBox>
#include "login.h"
#include "registration.h"
#include "adminwindow.h"
#include "masterwindow.h"
#include "ownerwindow.h"
#include "payrollwindow.h"
#include "database.h"
#include "salarywindow.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug() << "=== APPLICATION STARTING ===";

    // Инициализация базы данных
    if (!Database::instance().initializeDatabase()) {
        QMessageBox::critical(nullptr, "Ошибка", "Не удалось инициализировать базу данных!");
        return -1;
    }

    qDebug() << "Database initialized successfully";

    // Создаем тестовые данные
    Database::instance().createTestData();
    qDebug() << "Test data created";

    // Создаем окна
    LoginForm *loginForm = new LoginForm();
    RegistrationForm *registrationForm = new RegistrationForm();
    AdminWindow *adminWindow = new AdminWindow();
    MasterWindow *masterWindow = new MasterWindow();
    OwnerWindow *ownerWindow = new OwnerWindow();
    PayrollWindow *payrollWindow = new PayrollWindow();

    qDebug() << "All windows created";

    // Подключения сигналов для навигации между окнами
    QObject::connect(loginForm, &LoginForm::registrationClicked, [=]() {
        qDebug() << "Registration clicked";
        loginForm->hide();
        registrationForm->show();
    });

    QObject::connect(registrationForm, &RegistrationForm::loginClicked, [=]() {
        qDebug() << "Login clicked from registration";
        registrationForm->hide();
        loginForm->show();
    });

    // Обновляем подключение для логина с проверкой БД
    QObject::connect(loginForm, &LoginForm::loginAttempt, [=](const QString &email, const QString &password) {
        qDebug() << "Login attempt for:" << email;

        if (Database::instance().validateUser(email, password)) {
            QString role = Database::instance().getUserRole(email);
            QString userName = Database::instance().getUserName(email);

            qDebug() << "Login successful. Role:" << role << "Name:" << userName;

            loginForm->hide();

            if (role == "Owner") {
                ownerWindow->show();
                qDebug() << "Owner window shown";
            } else if (role == "Admin") {
                adminWindow->show();
                qDebug() << "Admin window shown";
            } else if (role == "Master") {
                // Получаем ID мастера из базы данных
                QSqlQuery query;
                query.prepare("SELECT id FROM users WHERE email = ?");
                query.addBindValue(email);
                if (query.exec() && query.next()) {
                    int masterId = query.value(0).toInt();
                    masterWindow->setUserId(masterId);
                    qDebug() << "Master ID set to:" << masterId;
                }
                masterWindow->show();
                qDebug() << "Master window shown";
            } else {
                QMessageBox::warning(loginForm, "Ошибка", "Неизвестная роль пользователя!");
                loginForm->show();
            }
        } else {
            qDebug() << "Login failed for:" << email;
            QMessageBox::warning(loginForm, "Ошибка входа", "Неверный email или пароль!");
        }
    });

    // Подключения для выхода
    QObject::connect(adminWindow, &AdminWindow::loginClicked, [=]() {
        qDebug() << "Logout from admin";
        adminWindow->hide();
        loginForm->show();
    });

    QObject::connect(masterWindow, &MasterWindow::loginClicked, [=]() {
        qDebug() << "Logout from master";
        masterWindow->hide();
        loginForm->show();
    });

    QObject::connect(ownerWindow, &OwnerWindow::loginClicked, [=]() {
        qDebug() << "Logout from owner";
        ownerWindow->hide();
        loginForm->show();
    });

    // Подключения для PayrollWindow
    QObject::connect(ownerWindow, &OwnerWindow::openPayrollWindow, [=]() {
        qDebug() << "Opening payroll window";
        ownerWindow->hide();
        payrollWindow->show();
    });

    QObject::connect(payrollWindow, &PayrollWindow::backClicked, [=]() {
        qDebug() << "Back from payroll";
        payrollWindow->hide();
        ownerWindow->show();
    });

    // Подключение сигнала обновления данных из PayrollWindow
    QObject::connect(payrollWindow, &PayrollWindow::dataUpdated, [=]() {
        qDebug() << "Payroll data updated - refreshing all data";
        // Можно добавить дополнительную логику обновления если нужно
    });

    // Демо-сигналы (остаются для других окон)
    QObject::connect(adminWindow, &AdminWindow::openScheduleWindow, [](){
        QMessageBox::information(nullptr, "Администратор", "Открытие окна 'График'");
    });


    QObject::connect(masterWindow, &MasterWindow::openWorkWindow, [](){
        QMessageBox::information(nullptr, "Мастер", "Открытие окна 'Моя работа'");
    });


    QObject::connect(ownerWindow, &OwnerWindow::openEmployeesWindow, [](){
        QMessageBox::information(nullptr, "Владелец", "Открытие окна 'Сотрудники'");
    });

    qDebug() << "All connections established";
    qDebug() << "Showing login form...";

    loginForm->show();

    int result = a.exec();

    qDebug() << "=== APPLICATION EXITING ===";

    // Очистка
    qDebug() << "Cleaning up...";
    delete loginForm;
    delete registrationForm;
    delete adminWindow;
    delete masterWindow;
    delete ownerWindow;
    delete payrollWindow;

    qDebug() << "Cleanup completed. Exit code:" << result;
    return result;
}
