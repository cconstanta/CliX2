#include "registration.h"
#include "database.h"
#include <QtWidgets>
#include <QMessageBox>
#include <QDebug>

RegistrationForm::RegistrationForm(QWidget *parent) :
    QWidget(parent),
    m_emailInput(nullptr),
    m_passwordInput(nullptr),
    m_confirmPasswordInput(nullptr),
    m_firstNameInput(nullptr),
    m_lastNameInput(nullptr),
    m_middleNameInput(nullptr),
    m_inviteCodeInput(nullptr),
    m_roleButton(nullptr),
    m_registerButton(nullptr),
    m_loginButton(nullptr),
    m_inviteCodeLabel(nullptr)
{
    setupUI();
}

RegistrationForm::~RegistrationForm()
{
}

void RegistrationForm::setupUI()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(500, 700);

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
    card->setFixedSize(450, 650);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(25);
    shadow->setOffset(0, 5);
    shadow->setColor(QColor(0,0,0,80));
    card->setGraphicsEffect(shadow);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(35, 35, 35, 35);
    cardLayout->setSpacing(12);

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
    QLabel *titleLabel = new QLabel("Регистрация", card);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2d9cff; margin-bottom: 5px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // Form fields
    m_firstNameInput = new QLineEdit(card);
    m_firstNameInput->setPlaceholderText("Имя *");
    m_firstNameInput->setFixedHeight(40);
    m_firstNameInput->setStyleSheet(R"(
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

    m_lastNameInput = new QLineEdit(card);
    m_lastNameInput->setPlaceholderText("Фамилия *");
    m_lastNameInput->setFixedHeight(40);
    m_lastNameInput->setStyleSheet(m_firstNameInput->styleSheet());

    m_middleNameInput = new QLineEdit(card);
    m_middleNameInput->setPlaceholderText("Отчество");
    m_middleNameInput->setFixedHeight(40);
    m_middleNameInput->setStyleSheet(m_firstNameInput->styleSheet());

    m_emailInput = new QLineEdit(card);
    m_emailInput->setPlaceholderText("Email *");
    m_emailInput->setFixedHeight(40);
    m_emailInput->setStyleSheet(m_firstNameInput->styleSheet());

    m_passwordInput = new QLineEdit(card);
    m_passwordInput->setPlaceholderText("Пароль *");
    m_passwordInput->setEchoMode(QLineEdit::Password);
    m_passwordInput->setFixedHeight(40);
    m_passwordInput->setStyleSheet(m_firstNameInput->styleSheet());

    m_confirmPasswordInput = new QLineEdit(card);
    m_confirmPasswordInput->setPlaceholderText("Подтвердите пароль *");
    m_confirmPasswordInput->setEchoMode(QLineEdit::Password);
    m_confirmPasswordInput->setFixedHeight(40);
    m_confirmPasswordInput->setStyleSheet(m_firstNameInput->styleSheet());

    // Role selection - КНОПКА ВЫБОРА РОЛИ
    QLabel *roleLabel = new QLabel("Желаемая роль:", card);
    roleLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px; margin-top: 5px;");

    // Кнопка для выбора роли вместо комбобокса
    m_roleButton = new QPushButton("Владелец", card);
    m_roleButton->setFixedHeight(40);
    m_roleButton->setCursor(Qt::PointingHandCursor);
    m_roleButton->setProperty("role", "Owner");
    m_roleButton->setStyleSheet(R"(
        QPushButton {
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 0 15px;
            font-size: 14px;
            background: white;
            text-align: left;
        }
        QPushButton:hover {
            border: 1px solid #2d9cff;
            background: #f8f9fa;
        }
    )");

    // Стрелка справа
    QLabel *arrowLabel = new QLabel("▼", card);
    arrowLabel->setStyleSheet("color: #666; font-size: 12px; background: transparent;");
    arrowLabel->setFixedSize(20, 20);

    QHBoxLayout *roleLayout = new QHBoxLayout();
    roleLayout->setContentsMargins(0, 0, 0, 0);
    roleLayout->setSpacing(0);
    roleLayout->addWidget(m_roleButton);
    roleLayout->addWidget(arrowLabel);

    // Invite code field
    m_inviteCodeLabel = new QLabel("Код приглашения *", card);
    m_inviteCodeLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px; margin-top: 5px;");

    m_inviteCodeInput = new QLineEdit(card);
    m_inviteCodeInput->setPlaceholderText("Введите код приглашения");
    m_inviteCodeInput->setFixedHeight(40);
    m_inviteCodeInput->setStyleSheet(m_firstNameInput->styleSheet());

    // Buttons
    m_registerButton = new QPushButton("Зарегистрироваться", card);
    m_registerButton->setFixedHeight(45);
    m_registerButton->setCursor(Qt::PointingHandCursor);
    m_registerButton->setStyleSheet(R"(
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

    m_loginButton = new QPushButton("Уже есть аккаунт? Войти", card);
    m_loginButton->setFixedHeight(35);
    m_loginButton->setCursor(Qt::PointingHandCursor);
    m_loginButton->setStyleSheet(R"(
        QPushButton {
            color: #2d9cff;
            border: none;
            background: transparent;
            font-weight: 500;
            font-size: 13px;
        }
        QPushButton:hover {
            background: rgba(45, 156, 255, 0.1);
            border-radius: 4px;
        }
    )");

    // Layout
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addStretch();
    topLayout->addWidget(closeBtn);

    cardLayout->addLayout(topLayout);
    cardLayout->addWidget(titleLabel);
    cardLayout->addSpacing(10);
    cardLayout->addWidget(m_firstNameInput);
    cardLayout->addWidget(m_lastNameInput);
    cardLayout->addWidget(m_middleNameInput);
    cardLayout->addWidget(m_emailInput);
    cardLayout->addWidget(m_passwordInput);
    cardLayout->addWidget(m_confirmPasswordInput);
    cardLayout->addWidget(roleLabel);
    cardLayout->addLayout(roleLayout);
    cardLayout->addWidget(m_inviteCodeLabel);
    cardLayout->addWidget(m_inviteCodeInput);
    cardLayout->addSpacing(10);
    cardLayout->addWidget(m_registerButton);
    cardLayout->addWidget(m_loginButton);

    root->addWidget(card, 0, Qt::AlignCenter);

    // Connections
    connect(m_roleButton, &QPushButton::clicked, this, &RegistrationForm::showRoleSelectionDialog);
    connect(m_registerButton, &QPushButton::clicked, this, &RegistrationForm::onRegisterClicked);
    connect(m_loginButton, &QPushButton::clicked, this, &RegistrationForm::onLoginClicked);
    connect(closeBtn, &QPushButton::clicked, qApp, &QApplication::quit);

    setLayout(root);

    // Изначально блокируем поле кода для владельца
    onRoleChanged("Owner");
}

void RegistrationForm::showRoleSelectionDialog()
{
    QDialog dialog(this);
    dialog.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dialog.setAttribute(Qt::WA_TranslucentBackground);
    dialog.setFixedSize(300, 300);

    QFrame *dialogCard = new QFrame(&dialog);
    dialogCard->setObjectName("dialogCard");
    dialogCard->setStyleSheet(R"(
        QFrame#dialogCard {
            background: white;
            border-radius: 12px;
            border: 1px solid #e0e0e0;
        }
    )");
    dialogCard->setFixedSize(300, 300);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(dialogCard);
    shadow->setBlurRadius(25);
    shadow->setOffset(0, 5);
    shadow->setColor(QColor(0,0,0,80));
    dialogCard->setGraphicsEffect(shadow);

    QVBoxLayout *dialogLayout = new QVBoxLayout(&dialog);
    dialogLayout->setContentsMargins(0,0,0,0);
    dialogLayout->addWidget(dialogCard);

    QVBoxLayout *cardLayout = new QVBoxLayout(dialogCard);
    cardLayout->setContentsMargins(20, 20, 20, 20);
    cardLayout->setSpacing(10);

    QLabel *titleLabel = new QLabel("Выберите роль", dialogCard);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2d9cff;");
    titleLabel->setAlignment(Qt::AlignCenter);

    cardLayout->addWidget(titleLabel);
    cardLayout->addSpacing(10);

    // Кнопки выбора роли
    QMap<QString, QString> roles = {
        {"Owner", "Владелец"},
        {"CoOwner", "Совладелец"},
        {"Admin", "Администратор"},
        {"Master", "Мастер"}
    };

    QString currentRole = m_roleButton->property("role").toString();

    for (auto it = roles.begin(); it != roles.end(); ++it) {
        QPushButton *roleBtn = new QPushButton(it.value(), dialogCard);
        roleBtn->setFixedHeight(45);
        roleBtn->setProperty("role", it.key());
        roleBtn->setCursor(Qt::PointingHandCursor);

        if (it.key() == currentRole) {
            roleBtn->setStyleSheet(R"(
                QPushButton {
                    background: #2d9cff;
                    color: white;
                    border: 2px solid #2d9cff;
                    border-radius: 8px;
                    font-weight: 600;
                    font-size: 14px;
                }
            )");
        } else {
            roleBtn->setStyleSheet(R"(
                QPushButton {
                    background: white;
                    color: #333;
                    border: 1px solid #ddd;
                    border-radius: 8px;
                    font-weight: 500;
                    font-size: 14px;
                }
                QPushButton:hover {
                    background: #f8f9fa;
                    border: 1px solid #2d9cff;
                }
            )");
        }

        connect(roleBtn, &QPushButton::clicked, [&, it]() {
            m_roleButton->setText(it.value());
            m_roleButton->setProperty("role", it.key());
            onRoleChanged(it.key());
            dialog.accept();
        });

        cardLayout->addWidget(roleBtn);
    }

    cardLayout->addStretch();

    // Кнопка отмены
    QPushButton *cancelBtn = new QPushButton("Отмена", dialogCard);
    cancelBtn->setFixedHeight(35);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton {
            background: rgba(0,0,0,0.05);
            color: #666;
            border: 1px solid rgba(0,0,0,0.1);
            border-radius: 6px;
            font-weight: 500;
            font-size: 13px;
        }
        QPushButton:hover {
            background: rgba(0,0,0,0.08);
        }
    )");

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    cardLayout->addWidget(cancelBtn);

    // Позиционируем диалог рядом с кнопкой выбора роли
    QPoint pos = m_roleButton->mapToGlobal(m_roleButton->rect().bottomLeft());
    dialog.move(pos.x(), pos.y() + 5);

    dialog.exec();
}

void RegistrationForm::onRoleChanged(const QString &role)
{
    bool isOwner = (role == "Owner");

    if (isOwner) {
        m_inviteCodeInput->setEnabled(false);
        m_inviteCodeInput->setPlaceholderText("Не требуется для владельца");
        m_inviteCodeInput->setText("");
        m_inviteCodeInput->setStyleSheet(R"(
            QLineEdit {
                border: 1px solid #ddd;
                border-radius: 8px;
                padding: 0 15px;
                font-size: 14px;
                background: #f5f5f5;
                color: #999;
            }
        )");
    } else {
        m_inviteCodeInput->setEnabled(true);
        m_inviteCodeInput->setPlaceholderText("Введите код приглашения");
        m_inviteCodeInput->setStyleSheet(m_firstNameInput->styleSheet());
    }
}

void RegistrationForm::onRegisterClicked()
{
    QString email = m_emailInput->text().trimmed();
    QString password = m_passwordInput->text();
    QString confirmPassword = m_confirmPasswordInput->text();
    QString firstName = m_firstNameInput->text().trimmed();
    QString lastName = m_lastNameInput->text().trimmed();
    QString middleName = m_middleNameInput->text().trimmed();
    QString selectedRole = m_roleButton->property("role").toString();
    QString inviteCode = m_inviteCodeInput->text().trimmed();

    // Валидация
    if (email.isEmpty() || password.isEmpty() || firstName.isEmpty() || lastName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все обязательные поля!");
        return;
    }
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
        if (!emailRegex.match(email).hasMatch()) {
            QMessageBox::warning(this, "Ошибка", "Введите корректный email адрес!\nПример: user@example.com");
            m_emailInput->setFocus();
            return;
        }
        if (password.length() < 6) {
                QMessageBox::warning(this, "Ошибка", "Пароль должен содержать минимум 6 символов!");
                m_passwordInput->setFocus();
                return;
            }
    if (password != confirmPassword) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают!");
        return;
    }

    // Определяем фактическую роль для системы
    QString actualRole = selectedRole;
    if (selectedRole == "CoOwner") {
        actualRole = "Owner"; // Совладельцы получают роль Owner в системе
    }

    if (actualRole != "Owner") {
        if (inviteCode.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Введите код приглашения!");
            return;
        }

        QString codeRole;
        if (!Database::instance().validateInviteCode(inviteCode, codeRole)) {
            QMessageBox::warning(this, "Ошибка", "Неверный код приглашения!");
            return;
        }

        if (codeRole != selectedRole) {
            QMessageBox::warning(this, "Ошибка",
                QString("Код приглашения предназначен для роли '%1'!").arg(getRoleDisplayName(codeRole)));
            return;
        }
    }

    // Создание пользователя
    if (Database::instance().createUser(email, password, firstName, lastName, middleName, actualRole, inviteCode)) {
        QMessageBox::information(this, "Успех", "Регистрация прошла успешно!");
        emit registrationSuccess();
        emit loginClicked();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось зарегистрировать пользователя!");
    }
}

void RegistrationForm::onLoginClicked()
{
    emit loginClicked();
}

QString RegistrationForm::getRoleDisplayName(const QString &role)
{
    if (role == "Owner") return "Владелец";
    if (role == "CoOwner") return "Совладелец";
    if (role == "Admin") return "Администратор";
    if (role == "Master") return "Мастер";
    return role;
}

void RegistrationForm::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void RegistrationForm::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}
