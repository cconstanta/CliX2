#include <iostream>
#include "adminwindow.h"
#include "financeswindow.h"
#include "schedulewindow.h"
#include <QtWidgets>
#include <QToolButton>
#include <QDebug>
#include <QDir>
#include "salarywindow.h"
AdminWindow::~AdminWindow() {}

AdminWindow::AdminWindow(QWidget *parent) : QWidget(parent) {
    // Проверка ресурсов
    checkResources();

    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(1100, 800); // Увеличил окно

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setAlignment(Qt::AlignCenter);

    QFrame *card = new QFrame(this);
    card->setObjectName("card");
    card->setStyleSheet(R"(
        QFrame#card {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                        stop:0 rgba(255,255,255,0.98),
                        stop:1 rgba(250,250,250,0.98));
            border-radius: 16px;
            border: 1px solid rgba(0,0,0,0.05);
        }
    )");
    card->setFixedSize(1060, 760);
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(30);
    shadow->setOffset(0, 12);
    shadow->setColor(QColor(0,0,0,120));
    card->setGraphicsEffect(shadow);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(30, 20, 30, 25);
    cardLayout->setSpacing(0);

    // Top Bar
    QWidget *topBar = new QWidget(card);
    topBar->setFixedHeight(50);
    topBar->setStyleSheet("background: transparent; border: none;");
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(0,0,0,0);

    QLabel *title = new QLabel("Панель администратора", topBar);
    QFont titleFont("Segoe UI", 20, QFont::Bold);
    title->setFont(titleFont);
    title->setStyleSheet("color: #2d9cff; background: transparent; border: none;");
    topBarLayout->addWidget(title, 0, Qt::AlignVCenter | Qt::AlignLeft);

    QToolButton *closeBtn = new QToolButton(topBar);
    closeBtn->setText("✕");
    closeBtn->setFixedSize(32,32);
    closeBtn->setStyleSheet(R"(
        QToolButton {
            border: none;
            background: transparent;
            color: #777;
            font-weight: bold;
            font-size: 16px;
            border-radius: 6px;
        }
        QToolButton:hover {
            background: rgba(244,67,54,0.1);
            color: #c0392b;
        }
    )");
    topBarLayout->addWidget(closeBtn, 0, Qt::AlignRight | Qt::AlignVCenter);
    cardLayout->addWidget(topBar);

    // Welcome message
    QLabel *welcomeLabel = new QLabel("Добро пожаловать в систему управления", card);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("color: #666; font-size: 14px; margin: 10px 0; background: transparent; border: none;");
    cardLayout->addWidget(welcomeLabel);

    cardLayout->addSpacing(40);

    // Buttons Grid - увеличенные кнопки по центру
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(30);
    buttonLayout->setContentsMargins(40, 0, 40, 0);

    // Schedule Button
    QToolButton *scheduleButton = new QToolButton(card);
    scheduleButton->setIcon(loadIcon(":/icons/schedule.png", "📅"));
    scheduleButton->setText("График\nработы");
    scheduleButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    scheduleButton->setFixedSize(220, 240);
    scheduleButton->setStyleSheet(R"(
        QToolButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(52, 152, 219, 0.15),
                stop:1 rgba(52, 152, 219, 0.08));
            color: #2980b9;
            border: 2px solid rgba(52, 152, 219, 0.3);
            border-radius: 20px;
            padding: 25px;
            font-weight: bold;
            font-size: 16px;
        }
        QToolButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(52, 152, 219, 0.25),
                stop:1 rgba(52, 152, 219, 0.15));
            border: 2px solid rgba(52, 152, 219, 0.5);
        }
        QToolButton:pressed {
            background: rgba(52, 152, 219, 0.3);
        }
    )");
    scheduleButton->setIconSize(QSize(100, 100));

    // Salary Button
    QToolButton *salaryButton = new QToolButton(card);
    salaryButton->setIcon(loadIcon(":/icons/salary.png", "💰"));
    salaryButton->setText("Моя\nзарплата");
    salaryButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    salaryButton->setFixedSize(220, 240);
    salaryButton->setStyleSheet(R"(
        QToolButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(231, 76, 60, 0.15),
                stop:1 rgba(231, 76, 60, 0.08));
            color: #c0392b;
            border: 2px solid rgba(231, 76, 60, 0.3);
            border-radius: 20px;
            padding: 25px;
            font-weight: bold;
            font-size: 16px;
        }
        QToolButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(231, 76, 60, 0.25),
                stop:1 rgba(231, 76, 60, 0.15));
            border: 2px solid rgba(231, 76, 60, 0.5);
        }
        QToolButton:pressed {
            background: rgba(231, 76, 60, 0.3);
        }
    )");
    salaryButton->setIconSize(QSize(100, 100));

    // Finances Button
    QToolButton *financesButton = new QToolButton(card);
    financesButton->setIcon(loadIcon(":/icons/finances.png", "📊"));
    financesButton->setText("Финансы\nкомпании");
    financesButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    financesButton->setFixedSize(220, 240);
    financesButton->setStyleSheet(R"(
        QToolButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(46, 204, 113, 0.15),
                stop:1 rgba(46, 204, 113, 0.08));
            color: #27ae60;
            border: 2px solid rgba(46, 204, 113, 0.3);
            border-radius: 20px;
            padding: 25px;
            font-weight: bold;
            font-size: 16px;
        }
        QToolButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(46, 204, 113, 0.25),
                stop:1 rgba(46, 204, 113, 0.15));
            border: 2px solid rgba(46, 204, 113, 0.5);
        }
        QToolButton:pressed {
            background: rgba(46, 204, 113, 0.3);
        }
    )");
    financesButton->setIconSize(QSize(100, 100));

    // Центрируем кнопки
    buttonLayout->addStretch();
    buttonLayout->addWidget(scheduleButton);
    buttonLayout->addWidget(salaryButton);
    buttonLayout->addWidget(financesButton);
    buttonLayout->addStretch();

    cardLayout->addLayout(buttonLayout);
    cardLayout->addStretch();

    // Bottom section with logout
    QWidget *bottomWidget = new QWidget(card);
    bottomWidget->setFixedHeight(60);
    bottomWidget->setStyleSheet("background: transparent; border: none;");
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(0, 10, 0, 0);

    QLabel *userInfo = new QLabel("Администратор • admin@company.com", bottomWidget);
    userInfo->setStyleSheet("color: #888; font-size: 12px; background: transparent; border: none;");

    QToolButton *logoutButton = new QToolButton(bottomWidget);
    logoutButton->setText("Выйти из системы");
    logoutButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    logoutButton->setFixedSize(160, 40);
    logoutButton->setCursor(Qt::PointingHandCursor);
    logoutButton->setStyleSheet(R"(
        QToolButton {
            color: #2d9cff;
            border: 1px solid rgba(45, 156, 255, 0.3);
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: bold;
            background: rgba(45, 156, 255, 0.05);
            font-size: 13px;
        }
        QToolButton:hover {
            background: rgba(45, 156, 255, 0.1);
            border: 1px solid rgba(45, 156, 255, 0.5);
        }
    )");

    bottomLayout->addWidget(userInfo);
    bottomLayout->addStretch();
    bottomLayout->addWidget(logoutButton);

    cardLayout->addWidget(bottomWidget);
    root->addWidget(card, 0, Qt::AlignCenter);

    // Connections
    connect(logoutButton, &QToolButton::clicked, this, &AdminWindow::onLogoutClicked);
    connect(closeBtn, &QToolButton::clicked, this, &AdminWindow::onCloseClicked);

    // Подключение кнопки "График работы"
    connect(scheduleButton, &QToolButton::clicked, this, [this]() {
        ScheduleWindow *scheduleWindow = new ScheduleWindow();
        scheduleWindow->show();
        this->hide();

        connect(scheduleWindow, &ScheduleWindow::backClicked, [this, scheduleWindow]() {
            scheduleWindow->hide();
            this->show();
            delete scheduleWindow;
        });
    });
    // Подключение кнопки "Моя зарплата" для админа
    connect(salaryButton, &QToolButton::clicked, this, [this]() {
        // Админ имеет ID 2 (из тестовых данных)
        int adminId = 2;
        qDebug() << "Opening SalaryWindow for admin ID:" << adminId;

        SalaryWindow *salaryWindow = new SalaryWindow(adminId, this);
        salaryWindow->show();
        this->hide();

        connect(salaryWindow, &SalaryWindow::backClicked, [this, salaryWindow]() {
            salaryWindow->hide();
            this->show();
            delete salaryWindow;
        });
    });
    // Подключение кнопки "Финансы компании"
    connect(financesButton, &QToolButton::clicked, this, [this]() {
        FinancesWindow *financesWindow = new FinancesWindow();
        financesWindow->show();
        this->hide();

        connect(financesWindow, &FinancesWindow::backClicked, [this, financesWindow]() {
            financesWindow->hide();
            this->show();
            delete financesWindow;
        });
    });

    setLayout(root);
}

QIcon AdminWindow::loadIcon(const QString &path, const QString &fallbackText) {
    QIcon icon(path);
    if (icon.isNull()) {
        qDebug() << "Не удалось загрузить иконку:" << path;

        QPixmap pixmap(100, 100);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QColor(100, 100, 100));
        painter.setFont(QFont("Segoe UI", 24));
        painter.drawText(pixmap.rect(), Qt::AlignCenter, fallbackText);

        icon = QIcon(pixmap);
        qDebug() << "Использована fallback-иконка для:" << path;
    } else {
        qDebug() << "Иконка загружена успешно:" << path;
    }
    return icon;
}

void AdminWindow::checkResources() {
    qDebug() << "=== Проверка ресурсов AdminWindow ===";

    QStringList iconPaths = {
        ":/icons/schedule.png",
        ":/icons/salary.png",
        ":/icons/finances.png"
    };

    for (const QString &path : iconPaths) {
        QFile file(path);
        if (file.exists()) {
            qDebug() << "✓ Файл существует:" << path;
            QIcon icon(path);
            if (!icon.isNull()) {
                qDebug() << "✓ Иконка загружается:" << path;
            } else {
                qDebug() << "✗ Иконка НЕ загружается:" << path;
            }
        } else {
            qDebug() << "✗ Файл НЕ существует:" << path;
        }
    }
    qDebug() << "=== Конец проверки ресурсов ===";
}

void AdminWindow::onLogoutClicked() {
    emit loginClicked();
}

void AdminWindow::onCloseClicked() {
    close();
}

void AdminWindow::onScheduleButtonClicked() {
    // Уже реализовано через лямбду
}


void AdminWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void AdminWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}
