#include "masterwindow.h"
#include "workwindow.h"
#include <QDebug>
#include <QApplication>
#include <QtWidgets>
#include <QSqlQuery>
#include "salarywindow.h"
MasterWindow::~MasterWindow() {
    qDebug() << "MasterWindow destructor called";
}

MasterWindow::MasterWindow(QWidget *parent) : QWidget(parent), m_dragPosition() {
    // Устанавливаем ID мастера по умолчанию (в реальном приложении получаем из БД)
    m_userId = 3; // Например, master1@company.com


    if (QIcon(":/icons/salary.png").isNull()) {
        qDebug() << "Ошибка: Не удалось загрузить иконку salary.png!";
    } else {
        qDebug() << "Иконка salary.png загружена успешно.";
    }

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
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                        stop:0 rgba(255,255,255,0.98),
                        stop:1 rgba(250,250,250,0.98));
            border-radius: 16px;
            border: 1px solid rgba(0,0,0,0.05);
        }
    )");
    card->setFixedSize(960, 760);

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

    QLabel *title = new QLabel("Рабочее место мастера", topBar);
    QFont titleFont("Segoe UI", 20, QFont::Bold);
    title->setFont(titleFont);
    title->setStyleSheet("color: #2d9cff; background: transparent; border: none;");
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
    topBarLayout->addWidget(closeBtn, 0, Qt::AlignRight | Qt::AlignVCenter);
    cardLayout->addWidget(topBar);

    // Welcome message
    QLabel *welcomeLabel = new QLabel("Управление задачами и рабочей информацией", card);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet("color: #666; font-size: 14px; margin: 10px 0; background: transparent; border: none;");
    cardLayout->addWidget(welcomeLabel);

    cardLayout->addSpacing(60);

    // Buttons Grid - увеличенные кнопки по центру
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(60);
    buttonLayout->setContentsMargins(60, 0, 60, 0);

    // Work Button
    QToolButton *myWorkButton = new QToolButton(card);
    myWorkButton->setIcon(QIcon(":/icons/work.png"));
    myWorkButton->setText("Моя\nработа");
    myWorkButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    myWorkButton->setFixedSize(240, 260);
    myWorkButton->setStyleSheet(R"(
        QToolButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(52, 152, 219, 0.15),
                stop:1 rgba(52, 152, 219, 0.08));
            color: #2980b9;
            border: 2px solid rgba(52, 152, 219, 0.3);
            border-radius: 20px;
            padding: 30px;
            font-weight: bold;
            font-size: 18px;
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
    myWorkButton->setIconSize(QSize(110, 110));

    // Salary Button
    QToolButton *salaryButton = new QToolButton(card);
    salaryButton->setIcon(QIcon(":/icons/salary.png"));
    salaryButton->setText("Моя\nзарплата");
    salaryButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    salaryButton->setFixedSize(240, 260);
    salaryButton->setStyleSheet(R"(
        QToolButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(231, 76, 60, 0.15),
                stop:1 rgba(231, 76, 60, 0.08));
            color: #c0392b;
            border: 2px solid rgba(231, 76, 60, 0.3);
            border-radius: 20px;
            padding: 30px;
            font-weight: bold;
            font-size: 18px;
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
    salaryButton->setIconSize(QSize(110, 110));

    // Центрируем кнопки
    buttonLayout->addStretch();
    buttonLayout->addWidget(myWorkButton);
    buttonLayout->addWidget(salaryButton);
    buttonLayout->addStretch();

    cardLayout->addLayout(buttonLayout);
    cardLayout->addStretch();

    // Bottom section with logout
    QWidget *bottomWidget = new QWidget(card);
    bottomWidget->setFixedHeight(60);
    bottomWidget->setStyleSheet("background: transparent; border: none;");
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(0, 10, 0, 0);

    // Получаем информацию о текущем мастере
    QString userEmail = "master@company.com";
    QString userName = "Мастер";

    QSqlQuery userQuery;
    userQuery.prepare("SELECT email, first_name, last_name FROM users WHERE id = ?");
    userQuery.addBindValue(m_userId);
    if (userQuery.exec() && userQuery.next()) {
        userEmail = userQuery.value(0).toString();
        userName = userQuery.value(1).toString() + " " + userQuery.value(2).toString();
    }

    QLabel *userInfo = new QLabel(userName + " • " + userEmail, bottomWidget);
    userInfo->setStyleSheet("color: #888; font-size: 12px; background: transparent; border: none;");

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
    connect(logoutButton, &QPushButton::clicked, this, &MasterWindow::onLogoutClicked);
    connect(closeBtn, &QPushButton::clicked, this, &MasterWindow::onCloseClicked);

    // Подключение кнопки "Моя работа" - ОБНОВЛЕННАЯ ВЕРСИЯ
    connect(myWorkButton, &QToolButton::clicked, this, [this]() {

        WorkWindow *workWindow = new WorkWindow(m_userId, this);
        workWindow->show();
        this->hide();

        connect(workWindow, &WorkWindow::backClicked, [this, workWindow]() {
            workWindow->hide();
            this->show();
            delete workWindow;
        });
    });
    // Подключение кнопки "Моя зарплата"
    connect(salaryButton, &QToolButton::clicked, this, [this]() {


        SalaryWindow *salaryWindow = new SalaryWindow(m_userId, this);
        salaryWindow->show();
        this->hide();

        connect(salaryWindow, &SalaryWindow::backClicked, [this, salaryWindow]() {
            salaryWindow->hide();
            this->show();
            delete salaryWindow;
        });
    });
    setLayout(root);
}

void MasterWindow::onLogoutClicked() {
    emit loginClicked();
}

void MasterWindow::onCloseClicked() {
    qApp->quit();
}

void MasterWindow::onWorkButtonClicked() {
    // Уже реализовано через лямбду
}


void MasterWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void MasterWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}
