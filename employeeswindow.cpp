#include "employeeswindow.h"
#include "database.h"
#include <QtWidgets>
#include <QSqlQuery>
#include <QDebug>
#include <QClipboard>
#include <QRandomGenerator>
#include <QTimer>

EmployeesWindow::EmployeesWindow(QWidget *parent) : QWidget(parent)
{
    setupUI();
    refreshEmployeesList();
}

EmployeesWindow::~EmployeesWindow() {
}

void EmployeesWindow::setupUI() {
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
    cardLayout->setSpacing(15);

    // Top Bar
    QWidget *topBar = new QWidget(card);
    topBar->setFixedHeight(40);
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

    QLabel *title = new QLabel("Управление сотрудниками", topBar);
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #2d9cff;");

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

    // Add employee section
    QWidget *addEmployeeWidget = new QWidget(card);
    addEmployeeWidget->setFixedHeight(50);
    QHBoxLayout *addEmployeeLayout = new QHBoxLayout(addEmployeeWidget);
    addEmployeeLayout->setContentsMargins(0, 5, 0, 5);
    addEmployeeLayout->setSpacing(12);

    QPushButton *addEmployeeBtn = new QPushButton("Добавить сотрудника", addEmployeeWidget);
    addEmployeeBtn->setFixedHeight(38);
    addEmployeeBtn->setCursor(Qt::PointingHandCursor);
    addEmployeeBtn->setStyleSheet(R"(
        QPushButton {
            background: #2d9cff;
            color: white;
            border-radius: 6px;
            font-weight: 600;
            border: none;
            font-size: 13px;
            min-width: 150px;
        }
        QPushButton:hover {
            background: #268ce6;
        }
    )");

    m_inviteCodeInput = new QLineEdit(addEmployeeWidget);
    m_inviteCodeInput->setPlaceholderText("Код приглашения появится здесь");
    m_inviteCodeInput->setReadOnly(true);
    m_inviteCodeInput->setFixedHeight(38);
    m_inviteCodeInput->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #ddd;
            border-radius: 6px;
            padding: 0 12px;
            font-size: 13px;
            background: #f8f9fa;
        }
    )");

    QPushButton *copyBtn = new QPushButton("Копировать", addEmployeeWidget);
    copyBtn->setFixedSize(100, 38); // Увеличил ширину кнопки
    copyBtn->setCursor(Qt::PointingHandCursor);
    copyBtn->setObjectName("copyButton"); // Добавил objectName для поиска
    copyBtn->setStyleSheet(R"(
        QPushButton {
            background: #27ae60;
            color: white;
            border-radius: 6px;
            font-weight: 600;
            border: none;
            font-size: 13px;
        }
        QPushButton:hover {
            background: #219653;
        }
        QPushButton:disabled {
            background: #cccccc;
            color: #666666;
        }
    )");
    copyBtn->setEnabled(false);

    addEmployeeLayout->addWidget(addEmployeeBtn);
    addEmployeeLayout->addWidget(m_inviteCodeInput);
    addEmployeeLayout->addWidget(copyBtn);

    cardLayout->addWidget(addEmployeeWidget);

    // Employees list
    QScrollArea *scrollArea = new QScrollArea(card);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    m_employeesContainer = new QWidget();
    m_employeesLayout = new QVBoxLayout(m_employeesContainer);
    m_employeesLayout->setSpacing(10);
    m_employeesLayout->setContentsMargins(2, 2, 2, 2);
    m_employeesLayout->setAlignment(Qt::AlignTop);

    scrollArea->setWidget(m_employeesContainer);
    cardLayout->addWidget(scrollArea);

    root->addWidget(card, 0, Qt::AlignCenter);

    // Connections
    connect(backBtn, &QPushButton::clicked, this, &EmployeesWindow::onBackClicked);
    connect(closeBtn, &QPushButton::clicked, this, &EmployeesWindow::onCloseClicked);
    connect(addEmployeeBtn, &QPushButton::clicked, this, &EmployeesWindow::onAddEmployeeClicked);
    connect(copyBtn, &QPushButton::clicked, this, &EmployeesWindow::onCopyCodeClicked);

    setLayout(root);
}

void EmployeesWindow::refreshEmployeesList() {
    qDebug() << "=== REFRESHING EMPLOYEES LIST ===";

    // Clear existing employee cards
    QLayoutItem* item;
    while ((item = m_employeesLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->hide();
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Сортируем: сначала Owner, потом Admin, потом Master
    QSqlQuery query("SELECT id, first_name, last_name, middle_name, role, is_active FROM users ORDER BY "
                   "CASE role "
                   "WHEN 'Owner' THEN 1 "
                   "WHEN 'Admin' THEN 2 "
                   "WHEN 'Master' THEN 3 "
                   "ELSE 4 END, last_name, first_name");

    int employeeCount = 0;
    while (query.next()) {
        int id = query.value(0).toInt();
        QString firstName = query.value(1).toString();
        QString lastName = query.value(2).toString();
        QString middleName = query.value(3).toString();
        QString role = query.value(4).toString();
        bool isActive = query.value(5).toBool();

        QString fullName;
        if (!middleName.isEmpty()) {
            fullName = QString("%1 %2 %3").arg(lastName).arg(firstName).arg(middleName);
        } else {
            fullName = QString("%1 %2").arg(lastName).arg(firstName);
        }

        qDebug() << "Creating card for:" << fullName << "Role:" << role << "Active:" << isActive;
        createEmployeeCard(m_employeesContainer, id, fullName, role, isActive);
        employeeCount++;
    }

    qDebug() << "Total employees to display:" << employeeCount;
    qDebug() << "=== EMPLOYEES LIST REFRESHED ===";
}

void EmployeesWindow::createEmployeeCard(QWidget *parent, int id, const QString &name,
                                       const QString &role, bool isActive) {
    QFrame *card = new QFrame(parent);
    card->setFixedHeight(70); // Большая высота карточки
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
    cardLayout->setSpacing(20);

    // User info
    QWidget *infoWidget = new QWidget(card);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoWidget);
    infoLayout->setContentsMargins(0,0,0,0);
    infoLayout->setSpacing(15);
    infoLayout->setAlignment(Qt::AlignVCenter);

    // Avatar placeholder
    QLabel *avatarLabel = new QLabel(infoWidget);
    avatarLabel->setFixedSize(40, 40);
    avatarLabel->setStyleSheet(QString(
        "QLabel {"
        "    background: %1;"
        "    border-radius: 20px;"
        "    color: white;"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "}"
    ).arg(getRoleColor(role)));
    avatarLabel->setAlignment(Qt::AlignCenter);

    // Берем первую букву имени для аватара
    QString initial = name.left(1).toUpper();
    avatarLabel->setText(initial);

    QLabel *nameLabel = new QLabel(name, infoWidget);
    nameLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #333; min-width: 200px;");

    QLabel *roleLabel = new QLabel(getRoleDisplayName(role), infoWidget);
    roleLabel->setStyleSheet(QString(
        "QLabel {"
        "    color: %1;"
        "    font-size: 12px;"
        "    background: rgba(%2,0.1);"
        "    padding: 6px 12px;"
        "    border-radius: 10px;"
        "    font-weight: 600;"
        "    min-width: 100px;"
        "    border: 1px solid rgba(%2,0.2);"
        "}"
    ).arg(getRoleColor(role)).arg(getRoleColor(role).mid(1)));
    roleLabel->setAlignment(Qt::AlignCenter);

    // Active/Inactive toggle button
    QPushButton *activeBtn = new QPushButton(isActive ? "Активен" : "Неактивен", infoWidget);
    activeBtn->setFixedSize(90, 30);
    activeBtn->setCursor(Qt::PointingHandCursor);
    activeBtn->setProperty("userId", id);
    activeBtn->setProperty("active", isActive);
    activeBtn->setStyleSheet(isActive ?
        R"(
            QPushButton {
                background: #27ae60;
                color: white;
                border: none;
                border-radius: 15px;
                font-size: 11px;
                font-weight: 600;
            }
            QPushButton:hover {
                background: #219653;
            }
        )" :
        R"(
            QPushButton {
                background: #e74c3c;
                color: white;
                border: none;
                border-radius: 15px;
                font-size: 11px;
                font-weight: 600;
            }
            QPushButton:hover {
                background: #c0392b;
            }
        )"
    );

    infoLayout->addWidget(avatarLabel);
    infoLayout->addWidget(nameLabel);
    infoLayout->addWidget(roleLabel);
    infoLayout->addWidget(activeBtn);
    infoLayout->addStretch();

    // Action buttons
    QWidget *actionsWidget = new QWidget(card);
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(0,0,0,0);
    actionsLayout->setSpacing(8);
    actionsLayout->setAlignment(Qt::AlignVCenter);

    // Delete button
    QPushButton *deleteBtn = new QPushButton("Удалить", actionsWidget);
    deleteBtn->setFixedSize(80, 30);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setStyleSheet(R"(
        QPushButton {
            border: 1px solid rgba(231,76,60,0.3);
            background: rgba(231,76,60,0.1);
            border-radius: 6px;
            font-size: 11px;
            font-weight: 600;
            color: #c0392b;
        }
        QPushButton:hover {
            background: rgba(231,76,60,0.2);
            border: 1px solid rgba(231,76,60,0.5);
        }
    )");

    actionsLayout->addWidget(deleteBtn);

    cardLayout->addWidget(infoWidget);
    cardLayout->addWidget(actionsWidget);

    // Connect active toggle
    connect(activeBtn, &QPushButton::clicked, [this, activeBtn, id]() {
        bool currentActive = activeBtn->property("active").toBool();
        bool newActive = !currentActive;

        // Обновляем в базе данных
        if (Database::instance().toggleEmployeeActive(id, newActive)) {
            // Обновляем кнопку
            activeBtn->setText(newActive ? "Активен" : "Неактивен");
            activeBtn->setProperty("active", newActive);
            activeBtn->setStyleSheet(newActive ?
                R"(
                    QPushButton {
                        background: #27ae60;
                        color: white;
                        border: none;
                        border-radius: 15px;
                        font-size: 11px;
                        font-weight: 600;
                    }
                    QPushButton:hover {
                        background: #219653;
                    }
                )" :
                R"(
                    QPushButton {
                        background: #e74c3c;
                        color: white;
                        border: none;
                        border-radius: 15px;
                        font-size: 11px;
                        font-weight: 600;
                    }
                    QPushButton:hover {
                        background: #c0392b;
                    }
                )"
            );
        }
    });

    // Connect delete button
    connect(deleteBtn, &QPushButton::clicked, [this, id, name]() {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Подтверждение удаления",
            QString("Вы уверены, что хотите удалить сотрудника:\n\"%1\"?\n\nЭто действие нельзя отменить.").arg(name),
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            if (Database::instance().deleteEmployee(id)) {
                refreshEmployeesList();
            } else {
                QMessageBox::warning(this, "Ошибка", "Не удалось удалить сотрудника!");
            }
        }
    });

    m_employeesLayout->addWidget(card);
}

void EmployeesWindow::showAddEmployeeDialog() {
    QDialog dialog(this);
    dialog.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dialog.setAttribute(Qt::WA_TranslucentBackground);
    dialog.setFixedSize(400, 250);

    QFrame *dialogCard = new QFrame(&dialog);
    dialogCard->setObjectName("dialogCard");
    dialogCard->setStyleSheet(R"(
        QFrame#dialogCard {
            background: white;
            border-radius: 12px;
            border: 1px solid #e0e0e0;
        }
    )");
    dialogCard->setFixedSize(400, 250);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(dialogCard);
    shadow->setBlurRadius(25);
    shadow->setOffset(0, 5);
    shadow->setColor(QColor(0,0,0,80));
    dialogCard->setGraphicsEffect(shadow);

    QVBoxLayout *dialogLayout = new QVBoxLayout(&dialog);
    dialogLayout->setContentsMargins(0,0,0,0);
    dialogLayout->addWidget(dialogCard);

    QVBoxLayout *cardLayout = new QVBoxLayout(dialogCard);
    cardLayout->setContentsMargins(25, 20, 25, 20);
    cardLayout->setSpacing(15);

    // Title
    QLabel *titleLabel = new QLabel("Добавить сотрудника", dialogCard);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2d9cff;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // Role selection
    QLabel *roleLabel = new QLabel("Выберите роль:", dialogCard);
    roleLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 13px;");

    QComboBox *roleComboBox = new QComboBox(dialogCard);
    roleComboBox->addItem("Совладелец", "CoOwner");
    roleComboBox->addItem("Администратор", "Admin");
    roleComboBox->addItem("Мастер", "Master");
    roleComboBox->setFixedHeight(35);
    roleComboBox->setStyleSheet(R"(
        QComboBox {
            border: 1px solid #ddd;
            border-radius: 6px;
            padding: 0 10px;
            font-size: 13px;
        }
    )");

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(roleLabel);
    cardLayout->addWidget(roleComboBox);
    cardLayout->addStretch();

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    QPushButton *cancelBtn = new QPushButton("Отмена", dialogCard);
    cancelBtn->setFixedHeight(35);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton {
            background: rgba(0,0,0,0.05);
            color: #666;
            border: 1px solid rgba(0,0,0,0.1);
            border-radius: 6px;
            font-weight: 600;
            font-size: 13px;
        }
        QPushButton:hover {
            background: rgba(0,0,0,0.08);
        }
    )");

    QPushButton *confirmBtn = new QPushButton("Создать код", dialogCard);
    confirmBtn->setFixedHeight(35);
    confirmBtn->setCursor(Qt::PointingHandCursor);
    confirmBtn->setStyleSheet(R"(
        QPushButton {
            background: #2d9cff;
            color: white;
            border-radius: 6px;
            font-weight: 600;
            border: none;
            font-size: 13px;
        }
        QPushButton:hover {
            background: #268ce6;
        }
    )");

    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(confirmBtn);
    cardLayout->addLayout(buttonLayout);

    // Connections
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(confirmBtn, &QPushButton::clicked, [&]() {
        QString role = roleComboBox->currentData().toString();
        QString inviteCode = generateInviteCode();

        if (Database::instance().createInviteCode(inviteCode, role)) {
            m_inviteCodeInput->setText(inviteCode);

            // Включаем кнопку копирования
            QPushButton *copyBtn = this->findChild<QPushButton*>("copyButton");
            if (copyBtn) {
                copyBtn->setEnabled(true);
                qDebug() << "Кнопка копирования включена, код:" << inviteCode;
            } else {
                qDebug() << "Не удалось найти кнопку копирования!";
            }

            dialog.accept();
        } else {
            QMessageBox::warning(&dialog, "Ошибка", "Не удалось создать код приглашения!");
        }
    });

    dialog.exec();
}

QString EmployeesWindow::generateInviteCode() {
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QString code;

    for (int i = 0; i < 8; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        code.append(chars.at(index));
    }

    return code;
}

void EmployeesWindow::onBackClicked() {
    emit backClicked();
}

void EmployeesWindow::onCloseClicked() {
    close();
}

void EmployeesWindow::onAddEmployeeClicked() {
    showAddEmployeeDialog();
}

void EmployeesWindow::onCopyCodeClicked() {
    QString code = m_inviteCodeInput->text();
    if (!code.isEmpty()) {
        QApplication::clipboard()->setText(code);
        qDebug() << "Код скопирован в буфер обмена:" << code;

        // Визуальная обратная связь
        QPushButton *copyBtn = qobject_cast<QPushButton*>(sender());
        if (copyBtn) {
            QString originalText = copyBtn->text();
            copyBtn->setText("Скопировано!");
            copyBtn->setStyleSheet(R"(
                QPushButton {
                    background: #27ae60;
                    color: white;
                    border-radius: 6px;
                    font-weight: 600;
                    border: none;
                    font-size: 13px;
                }
            )");

            QTimer::singleShot(1500, [copyBtn, originalText]() {
                if (copyBtn) {
                    copyBtn->setText(originalText);
                    copyBtn->setStyleSheet(R"(
                        QPushButton {
                            background: #27ae60;
                            color: white;
                            border-radius: 6px;
                            font-weight: 600;
                            border: none;
                            font-size: 13px;
                        }
                        QPushButton:hover {
                            background: #219653;
                        }
                    )");
                }
            });
        }
    } else {
        qDebug() << "Нет кода для копирования!";
    }
}

QString EmployeesWindow::getRoleDisplayName(const QString &role) {
    if (role == "Owner") return "Владелец";
    if (role == "CoOwner") return "Совладелец";
    if (role == "Admin") return "Администратор";
    if (role == "Master") return "Мастер";
    return "Не определен";
}

QString EmployeesWindow::getRoleColor(const QString &role) {
    if (role == "Owner") return "#e74c3c";
    if (role == "CoOwner") return "#e67e22";
    if (role == "Admin") return "#3498db";
    if (role == "Master") return "#9b59b6";
    return "#95a5a6";
}

void EmployeesWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void EmployeesWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}
