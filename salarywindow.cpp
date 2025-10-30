#include "salarywindow.h"
#include "database.h"
#include <QtWidgets>
#include <QDebug>
#include <QSqlQuery>
#include <QTimer>

SalaryWindow::SalaryWindow(int userId, QWidget *parent) :
    QWidget(parent), m_userId(userId)
{


    // Получаем информацию о пользователе
    QSqlQuery userQuery;
    userQuery.prepare("SELECT first_name, last_name, role FROM users WHERE id = ?");
    userQuery.addBindValue(userId);
    if (userQuery.exec() && userQuery.next()) {
        m_userName = userQuery.value(0).toString() + " " + userQuery.value(1).toString();
        m_userRole = userQuery.value(2).toString();
    } else {
        m_userName = "Сотрудник";
        m_userRole = "Master";
    }


    setupUI();
    refreshSalaryData();

    // Таймер автообновления каждые 30 секунд
    QTimer *refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &SalaryWindow::refreshSalaryData);
    refreshTimer->start(30000);
}

SalaryWindow::~SalaryWindow() {
}

void SalaryWindow::setupUI() {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(1000, 800);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setAlignment(Qt::AlignCenter);

    QFrame *card = new QFrame(this);
    card->setObjectName("card");
    card->setStyleSheet(R"(
        QFrame#card {
            background: white;
            border-radius: 15px;
            border: 1px solid #e0e0e0;
        }
    )");
    card->setFixedSize(960, 760);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(25);
    shadow->setOffset(0, 5);
    shadow->setColor(QColor(0,0,0,80));
    card->setGraphicsEffect(shadow);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(25, 20, 25, 20);
    cardLayout->setSpacing(20);

    // Top Bar
    QWidget *topBar = new QWidget(card);
    topBar->setFixedHeight(40);
    topBar->setStyleSheet("background: transparent; border: none;");
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(0,0,0,0);

    QPushButton *backBtn = new QPushButton("← Назад", topBar);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(R"(
        QPushButton {
            color: #2d9cff;
            border: none;
            background: transparent;
            font-weight: bold;
            font-size: 13px;
            padding: 6px 10px;
        }
        QPushButton:hover {
            background: rgba(45, 156, 255, 0.1);
            border-radius: 4px;
        }
    )");

    QLabel *title = new QLabel("Моя зарплата - " + m_userName, topBar);
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #2d9cff; background: transparent; border: none;");

    QPushButton *closeBtn = new QPushButton("✕", topBar);
    closeBtn->setFixedSize(28, 28);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            border: none;
            background: transparent;
            color: #777;
            font-weight: bold;
            font-size: 14px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background: rgba(244,67,54,0.1);
            color: #c0392b;
        }
    )");
    closeBtn->setCursor(Qt::PointingHandCursor);

    topBarLayout->addWidget(backBtn);
    topBarLayout->addStretch();
    topBarLayout->addWidget(title);
    topBarLayout->addStretch();
    topBarLayout->addWidget(closeBtn);

    cardLayout->addWidget(topBar);

    // Salary Settings Info
    QFrame *settingsFrame = new QFrame(card);
    settingsFrame->setStyleSheet(R"(
        QFrame {
            background: #f8f9fa;
            border: 1px solid #e0e0e0;
            border-radius: 10px;
            padding: 0px;
        }
    )");
    QVBoxLayout *settingsLayout = new QVBoxLayout(settingsFrame);
    settingsLayout->setContentsMargins(20, 15, 20, 15);
    settingsLayout->setSpacing(12);

    QLabel *settingsTitle = new QLabel("Настройки зарплаты", settingsFrame);
    settingsTitle->setStyleSheet("font-weight: bold; color: #2d9cff; font-size: 16px; background: transparent; border: none;");

    m_baseSalaryLabel = new QLabel("Оклад: 0 ₽", settingsFrame);
    m_baseSalaryLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px; background: transparent; border: none;");

    m_percentageLabel = new QLabel("Процент: 0%", settingsFrame);
    m_percentageLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px; background: transparent; border: none;");

    settingsLayout->addWidget(settingsTitle);
    settingsLayout->addWidget(m_baseSalaryLabel);
    settingsLayout->addWidget(m_percentageLabel);

    // Current Month Salary
    QFrame *salaryFrame = new QFrame(card);
    salaryFrame->setStyleSheet(R"(
        QFrame {
            background: #f8f9fa;
            border: 1px solid #e0e0e0;
            border-radius: 10px;
            padding: 0px;
        }
    )");
    QVBoxLayout *salaryLayout = new QVBoxLayout(salaryFrame);
    salaryLayout->setContentsMargins(20, 15, 20, 15);
    salaryLayout->setSpacing(10);

    QLabel *salaryTitle = new QLabel("Заработок за текущий месяц", salaryFrame);
    salaryTitle->setStyleSheet("font-weight: bold; color: #2d9cff; font-size: 16px; background: transparent; border: none;");

    m_totalEarningsLabel = new QLabel("Общий заработок: 0 ₽", salaryFrame);
    m_totalEarningsLabel->setStyleSheet("font-weight: bold; color: #2d9cff; font-size: 14px; background: transparent; border: none;");

    m_serviceEarningsLabel = new QLabel("Заработок с услуг: 0 ₽", salaryFrame);
    m_serviceEarningsLabel->setStyleSheet("font-weight: bold; color: #27ae60; font-size: 13px; background: transparent; border: none;");

    m_bonusEarningsLabel = new QLabel("Премии: 0 ₽", salaryFrame);
    m_bonusEarningsLabel->setStyleSheet("font-weight: bold; color: #9b59b6; font-size: 13px; background: transparent; border: none;");

    m_deductionsLabel = new QLabel("Вычеты: 0 ₽", salaryFrame);
    m_deductionsLabel->setStyleSheet("font-weight: bold; color: #e74c3c; font-size: 13px; background: transparent; border: none;");

    m_advancesLabel = new QLabel("Авансы: 0 ₽", salaryFrame);
    m_advancesLabel->setStyleSheet("font-weight: bold; color: #e67e22; font-size: 13px; background: transparent; border: none;");

    m_netSalaryLabel = new QLabel("Чистая зарплата: 0 ₽", salaryFrame);
    m_netSalaryLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px; background: transparent; border: none;");

    m_payableLabel = new QLabel("К выплате: 0 ₽", salaryFrame);
    m_payableLabel->setStyleSheet("font-weight: bold; color: #27ae60; font-size: 16px; background: transparent; border: none;");

    salaryLayout->addWidget(salaryTitle);
    salaryLayout->addWidget(m_totalEarningsLabel);
    salaryLayout->addWidget(m_serviceEarningsLabel);
    salaryLayout->addWidget(m_bonusEarningsLabel);
    salaryLayout->addWidget(m_deductionsLabel);
    salaryLayout->addWidget(m_advancesLabel);
    salaryLayout->addWidget(m_netSalaryLabel);
    salaryLayout->addWidget(m_payableLabel);

    // Salary History
    QLabel *historyTitle = new QLabel("История операций", card);
    historyTitle->setStyleSheet("font-weight: bold; color: #2d9cff; font-size: 16px; background: transparent; border: none;");

    QScrollArea *historyScrollArea = new QScrollArea(card);
    historyScrollArea->setWidgetResizable(true);
    historyScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    historyScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    historyScrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    m_historyContainer = new QWidget();
    m_historyContainer->setStyleSheet("background: transparent; border: none;");
    m_historyLayout = new QVBoxLayout(m_historyContainer);
    m_historyLayout->setSpacing(10);
    m_historyLayout->setContentsMargins(2, 2, 2, 2);
    m_historyLayout->setAlignment(Qt::AlignTop);

    historyScrollArea->setWidget(m_historyContainer);

    cardLayout->addWidget(settingsFrame);
    cardLayout->addWidget(salaryFrame);
    cardLayout->addWidget(historyTitle);
    cardLayout->addWidget(historyScrollArea);

    root->addWidget(card, 0, Qt::AlignCenter);

    // Connections
    connect(backBtn, &QPushButton::clicked, this, &SalaryWindow::onBackClicked);
    connect(closeBtn, &QPushButton::clicked, this, &SalaryWindow::onCloseClicked);

    setLayout(root);
}

void SalaryWindow::refreshSalaryData() {
    // Загружаем настройки зарплаты
    QSqlQuery settingsQuery;
    settingsQuery.prepare("SELECT base_salary, percentage FROM salary_settings WHERE user_id = ?");
    settingsQuery.addBindValue(m_userId);

    double baseSalary = 0, percentage = 0;
    if (settingsQuery.exec() && settingsQuery.next()) {
        baseSalary = settingsQuery.value(0).toDouble();
        percentage = settingsQuery.value(1).toDouble();
    }

    m_baseSalaryLabel->setText(QString("Оклад: %1 ₽").arg(baseSalary, 0, 'f', 2));
    m_percentageLabel->setText(QString("Процент: %1%").arg(percentage, 0, 'f', 1));

    // Рассчитываем зарплату за текущий месяц
    calculateCurrentMonthSalary();

    // Обновляем историю операций
    refreshSalaryHistory();
}

void SalaryWindow::calculateCurrentMonthSalary() {
    QDate currentDate = QDate::currentDate();
    QDate monthStart = QDate(currentDate.year(), currentDate.month(), 1);
    QDate monthEnd = QDate(currentDate.year(), currentDate.month(), currentDate.daysInMonth());

    // Заработок от услуг
    double serviceEarnings = calculateServiceEarnings(monthStart, monthEnd);

    // Финансовые операции
    double bonuses = getTotalBonuses(monthStart, monthEnd);
    double deductions = getTotalDeductions(monthStart, monthEnd);
    double advances = getTotalAdvances(monthStart, monthEnd);
    double salaryPayments = getTotalSalaryPayments(monthStart, monthEnd);

    // Итоговые расчеты
    double totalEarnings = serviceEarnings + bonuses;
    double netSalary = totalEarnings - deductions;
    double payable = netSalary - advances - salaryPayments;

    // Обновляем UI
    m_totalEarningsLabel->setText(QString("Общий заработок: %1 ₽").arg(totalEarnings, 0, 'f', 2));
    m_serviceEarningsLabel->setText(QString("Заработок с услуг: %1 ₽").arg(serviceEarnings, 0, 'f', 2));
    m_bonusEarningsLabel->setText(QString("Премии: %1 ₽").arg(bonuses, 0, 'f', 2));
    m_deductionsLabel->setText(QString("Вычеты: %1 ₽").arg(deductions, 0, 'f', 2));
    m_advancesLabel->setText(QString("Авансы: %1 ₽").arg(advances, 0, 'f', 2));
    m_netSalaryLabel->setText(QString("Чистая зарплата: %1 ₽").arg(netSalary, 0, 'f', 2));
    m_payableLabel->setText(QString("К выплате: %1 ₽").arg(payable, 0, 'f', 2));


}

double SalaryWindow::calculateServiceEarnings(const QDate &startDate, const QDate &endDate) {
    if (m_userRole != "Master") return 0.0; // Только мастера получают процент с услуг

    // Получаем процент мастера
    QSqlQuery percentageQuery;
    percentageQuery.prepare("SELECT percentage FROM salary_settings WHERE user_id = ?");
    percentageQuery.addBindValue(m_userId);

    double percentage = 0;
    if (percentageQuery.exec() && percentageQuery.next()) {
        percentage = percentageQuery.value(0).toDouble();
    }

    // Рассчитываем заработок от услуг
    QSqlQuery query;
    query.prepare(
        "SELECT COALESCE(SUM(s.price), 0) as total_earnings "
        "FROM appointments a "
        "JOIN services s ON a.service_id = s.id "
        "WHERE a.master_id = ? AND a.status = 'completed' AND a.appointment_date BETWEEN ? AND ?"
    );
    query.addBindValue(m_userId);
    query.addBindValue(startDate.toString("yyyy-MM-dd"));
    query.addBindValue(endDate.toString("yyyy-MM-dd"));

    if (query.exec() && query.next()) {
        double totalServices = query.value(0).toDouble();
        return totalServices * (percentage / 100.0);
    }

    return 0.0;
}

double SalaryWindow::getTotalBonuses(const QDate &startDate, const QDate &endDate) {
    QSqlQuery query;
    query.prepare(
        "SELECT COALESCE(SUM(amount), 0) FROM employee_financial_operations "
        "WHERE user_id = ? AND type = 'bonus' AND operation_date BETWEEN ? AND ?"
    );
    query.addBindValue(m_userId);
    query.addBindValue(startDate.toString("yyyy-MM-dd"));
    query.addBindValue(endDate.toString("yyyy-MM-dd"));

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

double SalaryWindow::getTotalDeductions(const QDate &startDate, const QDate &endDate) {
    QSqlQuery query;
    query.prepare(
        "SELECT COALESCE(SUM(amount), 0) FROM employee_financial_operations "
        "WHERE user_id = ? AND type = 'deduction' AND operation_date BETWEEN ? AND ?"
    );
    query.addBindValue(m_userId);
    query.addBindValue(startDate.toString("yyyy-MM-dd"));
    query.addBindValue(endDate.toString("yyyy-MM-dd"));

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

double SalaryWindow::getTotalAdvances(const QDate &startDate, const QDate &endDate) {
    QSqlQuery query;
    query.prepare(
        "SELECT COALESCE(SUM(amount), 0) FROM employee_financial_operations "
        "WHERE user_id = ? AND type = 'advance' AND operation_date BETWEEN ? AND ?"
    );
    query.addBindValue(m_userId);
    query.addBindValue(startDate.toString("yyyy-MM-dd"));
    query.addBindValue(endDate.toString("yyyy-MM-dd"));

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

double SalaryWindow::getTotalSalaryPayments(const QDate &startDate, const QDate &endDate) {
    QSqlQuery query;
    query.prepare(
        "SELECT COALESCE(SUM(amount), 0) FROM employee_financial_operations "
        "WHERE user_id = ? AND type = 'salary' AND operation_date BETWEEN ? AND ?"
    );
    query.addBindValue(m_userId);
    query.addBindValue(startDate.toString("yyyy-MM-dd"));
    query.addBindValue(endDate.toString("yyyy-MM-dd"));

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

void SalaryWindow::refreshSalaryHistory() {
    // Clear existing history
    QLayoutItem* item;
    while ((item = m_historyLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->hide();
            item->widget()->deleteLater();
        }
        delete item;
    }

    QSqlQuery query;
    query.prepare(
        "SELECT id, amount, type, description, operation_date "
        "FROM employee_financial_operations "
        "WHERE user_id = ? "
        "ORDER BY operation_date DESC, id DESC"
    );
    query.addBindValue(m_userId);

    if (!query.exec()) {
        return;
    }

    while (query.next()) {
        int operationId = query.value(0).toInt();
        double amount = query.value(1).toDouble();
        QString type = query.value(2).toString();
        QString description = query.value(3).toString();
        QDate date = query.value(4).toDate();

        // Create operation card
        QFrame *card = new QFrame(m_historyContainer);
        card->setFixedHeight(60);
        card->setStyleSheet(R"(
            QFrame {
                background: white;
                border: 1px solid #e0e0e0;
                border-radius: 8px;
                padding: 0px;
            }
            QFrame:hover {
                background: #f8f9fa;
            }
        )");

        QHBoxLayout *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(15, 10, 15, 10);
        cardLayout->setSpacing(10);

        // Amount with color based on type
        QLabel *amountLabel = new QLabel(card);
        QString amountText = QString("%1 ₽").arg(amount, 0, 'f', 2);
        QString amountColor;

        if (type == "bonus" || type == "salary") {
            amountLabel->setText("+" + amountText);
            amountColor = "#27ae60"; // зеленый для премий и зарплат
        } else if (type == "deduction") {
            amountLabel->setText("-" + amountText);
            amountColor = "#e74c3c"; // красный для вычетов
        } else if (type == "advance") {
            amountLabel->setText("-" + amountText);
            amountColor = "#e67e22"; // оранжевый для авансов
        } else {
            amountLabel->setText(amountText);
            amountColor = "#333333";
        }

        amountLabel->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 14px; min-width: 80px; background: transparent; border: none;").arg(amountColor));

        // Type badge
        QLabel *typeLabel = new QLabel(card);
        QString typeText, typeColor, typeBg;

        if (type == "bonus") {
            typeText = "ПРЕМИЯ";
            typeColor = "#9b59b6";
            typeBg = "rgba(155, 89, 182, 0.1)";
        } else if (type == "advance") {
            typeText = "АВАНС";
            typeColor = "#e67e22";
            typeBg = "rgba(230, 126, 34, 0.1)";
        } else if (type == "deduction") {
            typeText = "ВЫЧЕТ";
            typeColor = "#e74c3c";
            typeBg = "rgba(231, 76, 60, 0.1)";
        } else {
            typeText = "ЗАРПЛАТА";
            typeColor = "#3498db";
            typeBg = "rgba(52, 152, 219, 0.1)";
        }

        typeLabel->setText(typeText);
        typeLabel->setStyleSheet(QString(
            "QLabel {"
            "    color: %1;"
            "    font-size: 10px;"
            "    background: %2;"
            "    padding: 4px 8px;"
            "    border-radius: 8px;"
            "    font-weight: 600;"
            "    min-width: 70px;"
            "    border: none;"
            "}"
        ).arg(typeColor).arg(typeBg));
        typeLabel->setAlignment(Qt::AlignCenter);

        // Description
        QLabel *descLabel = new QLabel(description, card);
        descLabel->setStyleSheet("color: #666; font-size: 12px; background: transparent; border: none;");
        descLabel->setWordWrap(true);

        // Date
        QLabel *dateLabel = new QLabel(date.toString("dd.MM.yyyy"), card);
        dateLabel->setStyleSheet("color: #999; font-size: 11px; min-width: 80px; background: transparent; border: none;");

        cardLayout->addWidget(amountLabel);
        cardLayout->addWidget(typeLabel);
        cardLayout->addWidget(descLabel);
        cardLayout->addStretch();
        cardLayout->addWidget(dateLabel);

        m_historyLayout->addWidget(card);
    }

    // If no operations
    if (!query.first()) {
        QLabel *noDataLabel = new QLabel("Нет операций по зарплате", m_historyContainer);
        noDataLabel->setStyleSheet("color: #999; font-size: 14px; text-align: center; padding: 40px; background: transparent; border: none;");
        noDataLabel->setAlignment(Qt::AlignCenter);
        m_historyLayout->addWidget(noDataLabel);
    }
}

void SalaryWindow::onBackClicked() {
    emit backClicked();
}

void SalaryWindow::onCloseClicked() {
    close();
}

void SalaryWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void SalaryWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}
