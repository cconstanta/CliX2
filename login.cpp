#include "login.h"
#include <QtWidgets>
#include <QDebug>

LoginForm::LoginForm(QWidget *parent) :
    QWidget(parent),
    m_emailInput(nullptr),
    m_passwordInput(nullptr),
    m_loginButton(nullptr),
    m_registerButton(nullptr)
{
    setupUI();
}

LoginForm::~LoginForm()
{
}

void LoginForm::setupUI()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(450, 500);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(25, 25, 25, 25);
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
    card->setFixedSize(400, 450);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(25);
    shadow->setOffset(0, 5);
    shadow->setColor(QColor(0,0,0,80));
    card->setGraphicsEffect(shadow);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(35, 35, 35, 35);
    cardLayout->setSpacing(20);

    // Close button
    QPushButton *closeBtn = new QPushButton("✕", card);
    closeBtn->setFixedSize(30, 30);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            border: none;
            background: transparent;
            color: #777;
            font-weight: bold;
            font-size: 16px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background: rgba(244,67,54,0.1);
            color: #c0392b;
        }
    )");
    closeBtn->setCursor(Qt::PointingHandCursor);

    // Title
    QLabel *titleLabel = new QLabel("Вход в систему", card);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2d9cff; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // Email input
    m_emailInput = new QLineEdit(card);
    m_emailInput->setPlaceholderText("Email");
    m_emailInput->setFixedHeight(42);
    m_emailInput->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 0 15px;
            font-size: 14px;
        }
        QLineEdit:focus {
            border: 1px solid #2d9cff;
        }
    )");

    // Password input
    m_passwordInput = new QLineEdit(card);
    m_passwordInput->setPlaceholderText("Пароль");
    m_passwordInput->setEchoMode(QLineEdit::Password);
    m_passwordInput->setFixedHeight(42);
    m_passwordInput->setStyleSheet(m_emailInput->styleSheet());

    // Login button
    m_loginButton = new QPushButton("Войти", card);
    m_loginButton->setFixedHeight(45);
    m_loginButton->setCursor(Qt::PointingHandCursor);
    m_loginButton->setStyleSheet(R"(
        QPushButton {
            background: #2d9cff;
            color: white;
            border-radius: 8px;
            font-weight: 600;
            border: none;
            font-size: 15px;
        }
        QPushButton:hover {
            background: #268ce6;
        }
        QPushButton:pressed {
            background: #1a7cd9;
        }
    )");

    // Register button
    m_registerButton = new QPushButton("Создать аккаунт", card);
    m_registerButton->setFixedHeight(40);
    m_registerButton->setCursor(Qt::PointingHandCursor);
    m_registerButton->setStyleSheet(R"(
        QPushButton {
            color: #2d9cff;
            border: 1px solid #2d9cff;
            background: transparent;
            border-radius: 8px;
            font-weight: 500;
            font-size: 14px;
        }
        QPushButton:hover {
            background: rgba(45, 156, 255, 0.1);
        }
    )");

    // Layout
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addStretch();
    topLayout->addWidget(closeBtn);

    cardLayout->addLayout(topLayout);
    cardLayout->addWidget(titleLabel);
    cardLayout->addSpacing(15);
    cardLayout->addWidget(m_emailInput);
    cardLayout->addWidget(m_passwordInput);
    cardLayout->addSpacing(10);
    cardLayout->addWidget(m_loginButton);
    cardLayout->addWidget(m_registerButton);

    root->addWidget(card, 0, Qt::AlignCenter);

    // Connections
    connect(m_loginButton, &QPushButton::clicked, this, &LoginForm::onLoginClicked);
    connect(m_registerButton, &QPushButton::clicked, this, &LoginForm::onRegisterClicked);
    connect(closeBtn, &QPushButton::clicked, qApp, &QApplication::quit);

    setLayout(root);
}

void LoginForm::onLoginClicked()
{
    QString email = m_emailInput->text().trimmed();
    QString password = m_passwordInput->text();

    if (email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля!");
        return;
    }

    emit loginAttempt(email, password);
}

void LoginForm::onRegisterClicked()
{
    emit registrationClicked();
}

void LoginForm::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void LoginForm::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}
