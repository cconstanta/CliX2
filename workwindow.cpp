#include "workwindow.h"
#include "database.h"
#include <QtWidgets>
#include <QDebug>
#include <QSqlQuery>
#include <QTimer>

WorkWindow::WorkWindow(int userId, QWidget *parent) :
    QWidget(parent), m_userId(userId)
{
    qDebug() << "=== WORK WINDOW INIT ===";
    qDebug() << "Master ID:" << m_userId;

    // Получаем имя пользователя
    QSqlQuery userQuery;
    userQuery.prepare("SELECT first_name, last_name FROM users WHERE id = ?");
    userQuery.addBindValue(userId);
    if (userQuery.exec() && userQuery.next()) {
        m_userName = userQuery.value(0).toString() + " " + userQuery.value(1).toString();
    } else {
        m_userName = "Мастер";
    }

    qDebug() << "Master Name:" << m_userName;

    setupUI();
    refreshWorkData(); // Принудительное обновление при создании

    // Таймер автообновления каждые 10 секунд (вместо 30)
    QTimer *refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &WorkWindow::refreshWorkData);
    refreshTimer->start(10000); // 10 секунд
}

WorkWindow::~WorkWindow() {
}

double WorkWindow::getMasterPercentage() {
    QSqlQuery query;
    query.prepare("SELECT percentage FROM salary_settings WHERE user_id = ?");
    query.addBindValue(m_userId);

    if (query.exec() && query.next()) {
        double percentage = query.value(0).toDouble();
        qDebug() << "Master percentage for user" << m_userId << ":" << percentage << "%";
        return percentage;
    }

    qDebug() << "Using default percentage for user" << m_userId << "(30%)";
    return 30.0; // значение по умолчанию
}

void WorkWindow::setupUI() {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(1000, 700);

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
    card->setFixedSize(960, 660);

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

    QLabel *title = new QLabel("Моя работа - " + m_userName, topBar);
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

    // Statistics Cards
    QWidget *statsWidget = new QWidget(card);
    statsWidget->setFixedHeight(120);
    statsWidget->setStyleSheet("background: transparent; border: none;");
    QHBoxLayout *statsLayout = new QHBoxLayout(statsWidget);
    statsLayout->setContentsMargins(0, 10, 0, 10);
    statsLayout->setSpacing(15);

    // Earnings
    QWidget *earningsWidget = new QWidget(statsWidget);
    earningsWidget->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout *earningsLayout = new QVBoxLayout(earningsWidget);
    earningsLayout->setContentsMargins(0,0,0,0);
    earningsLayout->setSpacing(5);

    QLabel *earningsTitle = new QLabel("ЗАРАБОТОК", earningsWidget);
    earningsTitle->setStyleSheet("color: #27ae60; font-size: 11px; font-weight: bold; background: transparent; border: none;");
    m_todayEarningsLabel = new QLabel("0 ₽", earningsWidget);
    m_todayEarningsLabel->setStyleSheet("color: #27ae60; font-size: 20px; font-weight: bold; background: transparent; border: none;");

    earningsLayout->addWidget(earningsTitle, 0, Qt::AlignCenter);
    earningsLayout->addWidget(m_todayEarningsLabel, 0, Qt::AlignCenter);

    // Completed
    QWidget *completedWidget = new QWidget(statsWidget);
    completedWidget->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout *completedLayout = new QVBoxLayout(completedWidget);
    completedLayout->setContentsMargins(0,0,0,0);
    completedLayout->setSpacing(5);

    QLabel *completedTitle = new QLabel("ВЫПОЛНЕНО", completedWidget);
    completedTitle->setStyleSheet("color: #2d9cff; font-size: 11px; font-weight: bold; background: transparent; border: none;");
    m_completedCountLabel = new QLabel("0", completedWidget);
    m_completedCountLabel->setStyleSheet("color: #2d9cff; font-size: 20px; font-weight: bold; background: transparent; border: none;");

    completedLayout->addWidget(completedTitle, 0, Qt::AlignCenter);
    completedLayout->addWidget(m_completedCountLabel, 0, Qt::AlignCenter);

    // Pending
    QWidget *pendingWidget = new QWidget(statsWidget);
    pendingWidget->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout *pendingLayout = new QVBoxLayout(pendingWidget);
    pendingLayout->setContentsMargins(0,0,0,0);
    pendingLayout->setSpacing(5);

    QLabel *pendingTitle = new QLabel("ОЖИДАЮТ", pendingWidget);
    pendingTitle->setStyleSheet("color: #e67e22; font-size: 11px; font-weight: bold; background: transparent; border: none;");
    m_pendingCountLabel = new QLabel("0", pendingWidget);
    m_pendingCountLabel->setStyleSheet("color: #e67e22; font-size: 20px; font-weight: bold; background: transparent; border: none;");

    pendingLayout->addWidget(pendingTitle, 0, Qt::AlignCenter);
    pendingLayout->addWidget(m_pendingCountLabel, 0, Qt::AlignCenter);

    // Cancelled
    QWidget *cancelledWidget = new QWidget(statsWidget);
    cancelledWidget->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout *cancelledLayout = new QVBoxLayout(cancelledWidget);
    cancelledLayout->setContentsMargins(0,0,0,0);
    cancelledLayout->setSpacing(5);

    QLabel *cancelledTitle = new QLabel("ОТМЕНЕНО", cancelledWidget);
    cancelledTitle->setStyleSheet("color: #95a5a6; font-size: 11px; font-weight: bold; background: transparent; border: none;");
    m_cancelledCountLabel = new QLabel("0", cancelledWidget);
    m_cancelledCountLabel->setStyleSheet("color: #95a5a6; font-size: 20px; font-weight: bold; background: transparent; border: none;");

    cancelledLayout->addWidget(cancelledTitle, 0, Qt::AlignCenter);
    cancelledLayout->addWidget(m_cancelledCountLabel, 0, Qt::AlignCenter);

    // Progress
    QWidget *progressWidget = new QWidget(statsWidget);
    progressWidget->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout *progressLayout = new QVBoxLayout(progressWidget);
    progressLayout->setContentsMargins(0,0,0,0);
    progressLayout->setSpacing(5);

    QLabel *progressTitle = new QLabel("ПРОГРЕСС ДНЯ", progressWidget);
    progressTitle->setStyleSheet("color: #9b59b6; font-size: 11px; font-weight: bold; background: transparent; border: none;");

    m_progressBar = new QProgressBar(progressWidget);
    m_progressBar->setFixedSize(180, 20);
    m_progressBar->setStyleSheet(R"(
        QProgressBar {
            border: 1px solid #ddd;
            border-radius: 10px;
            background: #f8f9fa;
            text-align: center;
            font-size: 10px;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #9b59b6,
                stop:1 #8e44ad);
            border-radius: 9px;
        }
    )");
    m_progressBar->setFormat("%p%");

    progressLayout->addWidget(progressTitle, 0, Qt::AlignCenter);
    progressLayout->addWidget(m_progressBar, 0, Qt::AlignCenter);

    statsLayout->addStretch();
    statsLayout->addWidget(earningsWidget);
    statsLayout->addWidget(completedWidget);
    statsLayout->addWidget(pendingWidget);
    statsLayout->addWidget(cancelledWidget);
    statsLayout->addWidget(progressWidget);
    statsLayout->addStretch();

    cardLayout->addWidget(statsWidget);

    // Appointments List
    QScrollArea *scrollArea = new QScrollArea(card);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    m_appointmentsContainer = new QWidget();
    m_appointmentsContainer->setStyleSheet("background: transparent; border: none;");
    m_appointmentsLayout = new QVBoxLayout(m_appointmentsContainer);
    m_appointmentsLayout->setSpacing(10);
    m_appointmentsLayout->setContentsMargins(2, 2, 2, 2);
    m_appointmentsLayout->setAlignment(Qt::AlignTop);

    scrollArea->setWidget(m_appointmentsContainer);
    cardLayout->addWidget(scrollArea);

    root->addWidget(card, 0, Qt::AlignCenter);

    // Connections
    connect(backBtn, &QPushButton::clicked, this, &WorkWindow::onBackClicked);
    connect(closeBtn, &QPushButton::clicked, this, &WorkWindow::onCloseClicked);

    setLayout(root);
}

void WorkWindow::refreshWorkData() {
    refreshStats();
    refreshAppointmentsList();
}

void WorkWindow::refreshStats() {
    QDate today = QDate::currentDate();

    // Получаем статистику за сегодня
    QSqlQuery query;
    query.prepare(
        "SELECT "
        "COUNT(*) as total, "
        "SUM(CASE WHEN status = 'completed' THEN 1 ELSE 0 END) as completed, "
        "SUM(CASE WHEN status = 'cancelled' THEN 1 ELSE 0 END) as cancelled, "
        "SUM(CASE WHEN status = 'completed' THEN s.price ELSE 0 END) as total_earnings "
        "FROM appointments a "
        "JOIN services s ON a.service_id = s.id "
        "WHERE a.master_id = ? AND a.appointment_date = ?"
    );
    query.addBindValue(m_userId);
    query.addBindValue(today.toString("yyyy-MM-dd"));

    int total = 0, completed = 0, cancelled = 0;
    double totalEarnings = 0;

    if (query.exec() && query.next()) {
        total = query.value(0).toInt();
        completed = query.value(1).toInt();
        cancelled = query.value(2).toInt();
        totalEarnings = query.value(3).toDouble();
    }

    // Рассчитываем заработок мастера с учетом финансовых операций
    double masterPercentage = getMasterPercentage();
    double masterTotalEarnings = totalEarnings * (masterPercentage / 100.0);

    // Добавляем бонусы из финансовых операций за сегодня
    QSqlQuery bonusQuery;
    bonusQuery.prepare(
        "SELECT SUM(amount) FROM employee_financial_operations "
        "WHERE user_id = ? AND type = 'bonus' AND operation_date = ?"
    );
    bonusQuery.addBindValue(m_userId);
    bonusQuery.addBindValue(today.toString("yyyy-MM-dd"));

    if (bonusQuery.exec() && bonusQuery.next()) {
        double bonuses = bonusQuery.value(0).toDouble();
        masterTotalEarnings += bonuses;
        qDebug() << "Added bonuses for today:" << bonuses << "₽";
    }

    // Вычитаем вычеты за сегодня
    QSqlQuery deductionQuery;
    deductionQuery.prepare(
        "SELECT SUM(amount) FROM employee_financial_operations "
        "WHERE user_id = ? AND type = 'deduction' AND operation_date = ?"
    );
    deductionQuery.addBindValue(m_userId);
    deductionQuery.addBindValue(today.toString("yyyy-MM-dd"));

    if (deductionQuery.exec() && deductionQuery.next()) {
        double deductions = deductionQuery.value(0).toDouble();
        masterTotalEarnings -= deductions;
        qDebug() << "Subtracted deductions for today:" << deductions << "₽";
    }

    // Отмененные не считаются в ожидающих
    int pending = total - completed - cancelled;

    // Обновляем статистику
    m_todayEarningsLabel->setText(QString("%1 ₽").arg(masterTotalEarnings, 0, 'f', 2));
    m_completedCountLabel->setText(QString::number(completed));
    m_pendingCountLabel->setText(QString::number(pending));
    m_cancelledCountLabel->setText(QString::number(cancelled));

    // Обновляем прогресс-бар
    int totalForProgress = total - cancelled;
    int progress = totalForProgress > 0 ? (completed * 100) / totalForProgress : 0;
    m_progressBar->setValue(progress);

    qDebug() << "=== WORK WINDOW STATS ===";
    qDebug() << "Master ID:" << m_userId;
    qDebug() << "Total appointments:" << total;
    qDebug() << "Completed:" << completed;
    qDebug() << "Pending:" << pending;
    qDebug() << "Cancelled:" << cancelled;
    qDebug() << "Total services earnings:" << totalEarnings << "₽";
    qDebug() << "Master percentage:" << masterPercentage << "%";
    qDebug() << "Final master earnings:" << masterTotalEarnings << "₽";
}

void WorkWindow::refreshAppointmentsList() {
    qDebug() << "Refreshing appointments list for master:" << m_userId;

    // Clear existing appointments
    QLayoutItem* item;
    while ((item = m_appointmentsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->hide();
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Get today's appointments for this master
    QSqlQuery query;
    query.prepare(
        "SELECT a.id, a.client_name, s.name as service_name, s.price, "
        "a.appointment_time, a.status "
        "FROM appointments a "
        "JOIN services s ON a.service_id = s.id "
        "WHERE a.master_id = ? AND a.appointment_date = ? "
        "ORDER BY a.appointment_time"
    );
    query.addBindValue(m_userId);
    query.addBindValue(QDate::currentDate().toString("yyyy-MM-dd"));

    if (!query.exec()) {
        qDebug() << "Error getting appointments:" << query.lastError().text();
        return;
    }

    int appointmentCount = 0;
    while (query.next()) {
        int appointmentId = query.value(0).toInt();
        QString clientName = query.value(1).toString();
        QString serviceName = query.value(2).toString();
        double price = query.value(3).toDouble();
        QTime time = query.value(4).toTime();
        QString status = query.value(5).toString();

        createAppointmentCard(m_appointmentsContainer, appointmentId,
                             clientName, serviceName, price, time, status);
        appointmentCount++;
    }

    qDebug() << "Loaded" << appointmentCount << "appointments";

    // If no appointments
    if (appointmentCount == 0) {
        QLabel *noDataLabel = new QLabel("На сегодня записей нет", m_appointmentsContainer);
        noDataLabel->setStyleSheet("color: #999; font-size: 14px; text-align: center; padding: 40px; background: transparent; border: none;");
        noDataLabel->setAlignment(Qt::AlignCenter);
        m_appointmentsLayout->addWidget(noDataLabel);
        qDebug() << "No appointments found for today";
    }
}

void WorkWindow::createAppointmentCard(QWidget *parent, int appointmentId,
                                      const QString &clientName, const QString &serviceName,
                                      double price, const QTime &time, const QString &status) {

    // РАСЧЕТ ЗАРАБОТКА МАСТЕРА
    double masterPercentage = getMasterPercentage();
    double masterEarning = price * (masterPercentage / 100.0);

    QFrame *card = new QFrame(parent);
    card->setFixedHeight(80); // Уменьшаем высоту, так как убрали лишнюю информацию
    card->setStyleSheet(R"(
        QFrame {
            background: white;
            border: 1px solid #e0e0e0;
            border-radius: 10px;
            padding: 0px;
        }
        QFrame:hover {
            background: #fafafa;
        }
    )");

    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(20, 15, 20, 15);
    cardLayout->setSpacing(15);

    // Time - УБИРАЕМ РАМКУ
    QLabel *timeLabel = new QLabel(time.toString("hh:mm"), card);
    timeLabel->setStyleSheet("color: #333; font-weight: bold; font-size: 16px; min-width: 60px; background: transparent; border: none;");

    // Client info - упрощаем, убираем заработок
    QWidget *infoWidget = new QWidget(card);
    infoWidget->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout *infoLayout = new QVBoxLayout(infoWidget);
    infoLayout->setContentsMargins(0,0,0,0);
    infoLayout->setSpacing(3);

    QLabel *clientLabel = new QLabel(clientName, infoWidget);
    clientLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #333; background: transparent; border: none;");

    QLabel *serviceLabel = new QLabel(serviceName, infoWidget);
    serviceLabel->setStyleSheet("font-size: 12px; color: #666; background: transparent; border: none;");

    infoLayout->addWidget(clientLabel);
    infoLayout->addWidget(serviceLabel);

    // Master earnings - ПЕРЕНЕСЕНО НА МЕСТО ЦЕНЫ УСЛУГИ
    QLabel *masterEarningLabel = new QLabel(QString("%1 ₽").arg(masterEarning, 0, 'f', 2), card);
    masterEarningLabel->setStyleSheet("color: #27ae60; font-weight: bold; font-size: 16px; min-width: 80px; background: transparent; border: none;");

    // Status badge - УБИРАЕМ РАМКУ
    QLabel *statusLabel = new QLabel(card);
    QString statusText, statusColor, bgColor;

    if (status == "completed") {
        statusText = "✓ ВЫПОЛНЕНО";
        statusColor = "#27ae60";
        bgColor = "rgba(39, 174, 96, 0.1)";
    } else if (status == "cancelled") {
        statusText = "✗ ОТМЕНЕНО";
        statusColor = "#95a5a6";
        bgColor = "rgba(149, 165, 166, 0.1)";
    } else {
        statusText = "⏰ ЗАПЛАНИРОВАНО";
        statusColor = "#3498db";
        bgColor = "rgba(52, 152, 219, 0.1)";
    }

    statusLabel->setText(statusText);
    statusLabel->setStyleSheet(QString(
        "QLabel {"
        "    color: %1;"
        "    font-size: 11px;"
        "    background: %2;"
        "    padding: 6px 12px;"
        "    border-radius: 12px;"
        "    font-weight: 600;"
        "    min-width: 120px;"
        "    border: none;"
        "}"
    ).arg(statusColor).arg(bgColor));
    statusLabel->setAlignment(Qt::AlignCenter);

    // Complete button (only for scheduled appointments)
    QPushButton *completeBtn = nullptr;
    if (status == "scheduled") {
        completeBtn = new QPushButton("Выполнить", card);
        completeBtn->setFixedSize(90, 30);
        completeBtn->setCursor(Qt::PointingHandCursor);
        completeBtn->setProperty("appointmentId", appointmentId);
        completeBtn->setStyleSheet(R"(
            QPushButton {
                background: #27ae60;
                color: white;
                border: none;
                border-radius: 6px;
                font-size: 11px;
                font-weight: 600;
            }
            QPushButton:hover {
                background: #219653;
            }
        )");

        connect(completeBtn, &QPushButton::clicked, [this, completeBtn]() {
            int appointmentId = completeBtn->property("appointmentId").toInt();
            onAppointmentStatusChanged(appointmentId, true);
        });
    }

    cardLayout->addWidget(timeLabel);
    cardLayout->addWidget(infoWidget);
    cardLayout->addStretch();
    cardLayout->addWidget(masterEarningLabel); // Теперь здесь заработок мастера
    cardLayout->addWidget(statusLabel);

    if (completeBtn) {
        cardLayout->addWidget(completeBtn);
    }

    m_appointmentsLayout->addWidget(card);

    qDebug() << "Created card for:" << clientName << serviceName << "at" << time.toString("hh:mm") <<
                "Master earning:" << masterEarning << "₽ (" << masterPercentage << "%)";
}

void WorkWindow::onAppointmentStatusChanged(int appointmentId, bool completed) {
    QSqlQuery query;
    query.prepare("UPDATE appointments SET status = ? WHERE id = ?");
    query.addBindValue(completed ? "completed" : "scheduled");
    query.addBindValue(appointmentId);

    if (query.exec()) {
        qDebug() << "Appointment status updated. ID:" << appointmentId << "Status:" << (completed ? "completed" : "scheduled");
        refreshWorkData();
    } else {
        qDebug() << "Error updating appointment status:" << query.lastError().text();
        QMessageBox::warning(this, "Ошибка", "Не удалось обновить статус записи!");
    }
}

void WorkWindow::onBackClicked() {
    emit backClicked();
}

void WorkWindow::onCloseClicked() {
    close();
}

void WorkWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void WorkWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}
