#include "ownerwindow.h"
#include "employeeswindow.h"
#include "financeswindow.h"
#include <QtWidgets>
#include <QDebug>

OwnerWindow::~OwnerWindow() {
}

OwnerWindow::OwnerWindow(QWidget *parent) : QWidget(parent) {
    // Проверка ресурсов
    if (QIcon(":/icons/employees.png").isNull()) {
        qDebug() << "Ошибка: Не удалось загрузить иконку employees.png!";
    } else {
        qDebug() << "Иконка employees.png загружена успешно.";
    }

    if (QIcon(":/icons/finances.png").isNull()) {
        qDebug() << "Ошибка: Не удалось загрузить иконку finances.png!";
    } else {
        qDebug() << "Иконка finances.png загружена успешно.";
    }

    if (QIcon(":/icons/payroll.png").isNull()) {
        qDebug() << "Ошибка: Не удалось загрузить иконку payroll.png!";
    } else {
        qDebug() << "Иконка payroll.png загружена успешно.";
    }

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
    card->setFixedSize(1060, 760); // Увеличил карточку

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
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(0,0,0,0);

    QLabel *title = new QLabel("Панель управления владельца", topBar);
    QFont titleFont("Segoe UI", 20, QFont::Bold);
    title->setFont(titleFont);
    title->setStyleSheet("color: #2d9cff; background: transparent;");
    topBarLayout->addWidget(title, 0, Qt::AlignVCenter | Qt::AlignLeft);

    QPushButton *closeBtn = new QPushButton("✕", topBar);
    closeBtn->setFixedSize(32, 32);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            border: none;
            background: transparent;
            color: #777;
            font-weight: bold;
            font-size: 16px;
            border-radius: 6px;
        }
        QPushButton:hover {
            background: rgba(244,67,54,0.1);
            color: #c0392b;
        }
    )");
    closeBtn->setCursor(Qt::PointingHandCursor);
    topBarLayout->addWidget(closeBtn, 0, Qt::AlignRight | Qt::AlignVCenter);
    cardLayout->addWidget(topBar);

    // Welcome message
    QLabel *welcomeLabel = new QLabel("Полный контроль над бизнес-процессами", card);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("color: #666; font-size: 14px; margin: 10px 0; background: transparent;");
    cardLayout->addWidget(welcomeLabel);

    cardLayout->addSpacing(40);

    // Buttons Grid - ОЧЕНЬ большие кнопки по центру
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(30);
    buttonLayout->setContentsMargins(40, 0, 40, 0);

    // Employees Button
    QToolButton *employeesButton = new QToolButton(card);
    employeesButton->setIcon(QIcon(":/icons/employees.png"));
    employeesButton->setText("Сотрудники");
    employeesButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    employeesButton->setFixedSize(220, 240);
    employeesButton->setStyleSheet(R"(
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
    employeesButton->setIconSize(QSize(100, 100));

    // Finances Button
    QToolButton *financesButton = new QToolButton(card);
    financesButton->setIcon(QIcon(":/icons/finances.png"));
    financesButton->setText("Финансы");
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

    // Payroll Button
    QToolButton *payrollButton = new QToolButton(card);
    payrollButton->setIcon(QIcon(":/icons/payroll.png"));
    payrollButton->setText("Зарплата\nсотрудникам");
    payrollButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    payrollButton->setFixedSize(220, 240);
    payrollButton->setStyleSheet(R"(
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
    payrollButton->setIconSize(QSize(100, 100));

    // Центрируем кнопки
    buttonLayout->addStretch();
    buttonLayout->addWidget(employeesButton);
    buttonLayout->addWidget(financesButton);
    buttonLayout->addWidget(payrollButton);
    buttonLayout->addStretch();

    cardLayout->addLayout(buttonLayout);
    cardLayout->addStretch();

    // Bottom section with logout
    QWidget *bottomWidget = new QWidget(card);
    bottomWidget->setFixedHeight(60);
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(0, 10, 0, 0);

    QLabel *userInfo = new QLabel("Владелец • owner@company.com", bottomWidget);
    userInfo->setStyleSheet("color: #888; font-size: 12px; background: transparent;");

    QPushButton *logoutButton = new QPushButton("Выйти из системы", bottomWidget);
    logoutButton->setFixedSize(160, 40);
    logoutButton->setCursor(Qt::PointingHandCursor);
    logoutButton->setStyleSheet(R"(
        QPushButton {
            color: #2d9cff;
            border: 1px solid rgba(45, 156, 255, 0.3);
            border-radius: 8px;
            padding: 8px 16px;
            font-weight: bold;
            background: rgba(45, 156, 255, 0.05);
            font-size: 13px;
        }
        QPushButton:hover {
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
    connect(logoutButton, &QPushButton::clicked, this, &OwnerWindow::onLogoutClicked);
    connect(closeBtn, &QPushButton::clicked, qApp, &QApplication::quit);
    connect(payrollButton, &QToolButton::clicked, this, &OwnerWindow::onPayrollButtonClicked);

    // Подключение кнопки "Сотрудники"
    connect(employeesButton, &QToolButton::clicked, this, [this]() {
        EmployeesWindow *employeesWindow = new EmployeesWindow();
        employeesWindow->show();
        this->hide();

        connect(employeesWindow, &EmployeesWindow::backClicked, [this, employeesWindow]() {
            employeesWindow->hide();
            this->show();
            delete employeesWindow;
        });
    });

    // Подключение кнопки "Финансы"
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

void OwnerWindow::onLogoutClicked() {
    emit loginClicked();
}

void OwnerWindow::onCloseClicked() {
    close();
}

void OwnerWindow::onEmployeesButtonClicked() {
    // Уже реализовано через лямбду
}

void OwnerWindow::onPayrollButtonClicked() {
    emit openPayrollWindow();
}

void OwnerWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void OwnerWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}
