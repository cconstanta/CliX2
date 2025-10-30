#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMap>
#include <QVariant>
#include <QDate>

class Database : public QObject
{
    Q_OBJECT

public:
    static Database& instance() {
        static Database instance;
        return instance;
    }
    QList<QMap<QString, QVariant>> getMasterSchedule(int masterId, const QDate &date);
    bool updateAppointmentStatus(int appointmentId, const QString &status);
    QMap<QString, QVariant> getMasterStatistics(int masterId, const QDate &startDate, const QDate &endDate);
    bool initializeDatabase();
    void createTestData();

    // Методы для работы с пользователями
    bool createUser(const QString &email, const QString &password, const QString &firstName,
                   const QString &lastName, const QString &middleName, const QString &role,
                   const QString &inviteCode = "");
    bool validateUser(const QString &email, const QString &password);
    QString getUserRole(const QString &email);
    QString getUserName(const QString &email);
    bool userExists(const QString &email);
    void createTestAppointments();
    // Новые методы для управления сотрудниками
    QList<QMap<QString, QVariant>> getAllEmployees();
    bool updateEmployeeRole(int userId, const QString &newRole);
    bool deleteEmployee(int userId);
    bool toggleEmployeeActive(int userId, bool active);

    // Методы для работы с кодами приглашений
    bool createInviteCode(const QString &code, const QString &role);
    bool validateInviteCode(const QString &code, QString &role);

    // Методы для финансовых операций
    QList<QMap<QString, QVariant>> getFinancialOperations(const QDate &startDate, const QDate &endDate);
    bool addFinancialOperation(double amount, const QString &category, const QString &description,
                              const QDate &date, int createdBy);
    bool deleteFinancialOperation(int operationId);
    double getTotalIncome(const QDate &startDate, const QDate &endDate);
    double getTotalExpenses(const QDate &startDate, const QDate &endDate);

    // Методы для работы с зарплатой
    bool updateSalarySettings(int userId, double baseSalary, double percentage);
    QMap<QString, QVariant> getSalarySettings(int userId);
    QList<QMap<QString, QVariant>> getEmployeeFinancialOperations(int userId, const QDate &startDate, const QDate &endDate);
    bool addEmployeeFinancialOperation(int userId, double amount, const QString &type, const QString &description, const QDate &date, int createdBy);
    double getEmployeeServiceEarnings(int userId, const QDate &startDate, const QDate &endDate);

private:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    // Запрещаем копирование и присваивание
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    QSqlDatabase m_database;
    bool updateDatabaseSchema();
};

#endif // DATABASE_H
