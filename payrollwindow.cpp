#include "payrollwindow.h"
#include "database.h"
#include <QtWidgets>
#include <QDebug>
#include <QSqlQuery>
#include <QHeaderView>
#include <QMessageBox>
#include <QButtonGroup>

PayrollWindow::PayrollWindow(QWidget *parent) : QWidget(parent),
    m_selectedEmployeeId(-1),
    m_currentOperationType("")
{
    setupUI();
    refreshEmployeesList();
}

PayrollWindow::~PayrollWindow() {
}

void PayrollWindow::setupUI() {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(1400, 900);

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
    card->setFixedSize(1360, 860);

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

    QLabel *title = new QLabel("Зарплата сотрудникам", topBar);
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

    // Main Content - Split into two columns
    QWidget *contentWidget = new QWidget(card);
    QHBoxLayout *contentLayout = new QHBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(20);

    // Left Column - Employee List and Salary Settings
    QWidget *leftColumn = new QWidget(contentWidget);
    leftColumn->setFixedWidth(500);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftColumn);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(15);

    // Employee Selection
    QWidget *employeeSelectionWidget = new QWidget(leftColumn);
    employeeSelectionWidget->setFixedHeight(60);
    QVBoxLayout *employeeSelectionLayout = new QVBoxLayout(employeeSelectionWidget);
    employeeSelectionLayout->setContentsMargins(0, 0, 0, 0);
    employeeSelectionLayout->setSpacing(5);

    QLabel *employeeLabel = new QLabel("Выберите сотрудника:", employeeSelectionWidget);
    employeeLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px; background: transparent; border: none;");

    QWidget *employeeButtonWidget = new QWidget(employeeSelectionWidget);
    QHBoxLayout *employeeButtonLayout = new QHBoxLayout(employeeButtonWidget);
    employeeButtonLayout->setContentsMargins(0, 0, 0, 0);
    employeeButtonLayout->setSpacing(10);

    m_employeeButton = new QPushButton("Выбрать сотрудника", employeeButtonWidget);
    m_employeeButton->setFixedHeight(35);
    m_employeeButton->setCursor(Qt::PointingHandCursor);
    m_employeeButton->setStyleSheet(R"(
        QPushButton {
            border: 1px solid #ddd;
            border-radius: 6px;
            padding: 0 12px;
            font-size: 14px;
            background: white;
            text-align: left;
        }
        QPushButton:hover {
            border: 1px solid #2d9cff;
            background: #f8f9fa;
        }
    )");

    m_employeeLabel = new QLabel("", employeeButtonWidget);
    m_employeeLabel->setStyleSheet("font-size: 14px; color: #333; padding: 0 10px; background: transparent; border: none;");
    m_employeeLabel->setVisible(false);

    m_changeEmployeeButton = new QPushButton("Изменить", employeeButtonWidget);
    m_changeEmployeeButton->setFixedSize(80, 35);
    m_changeEmployeeButton->setCursor(Qt::PointingHandCursor);
    m_changeEmployeeButton->setStyleSheet(R"(
        QPushButton {
            border: 1px solid #2d9cff;
            border-radius: 6px;
            font-size: 12px;
            background: rgba(45, 156, 255, 0.1);
            color: #2d9cff;
            font-weight: 600;
        }
        QPushButton:hover {
            background: rgba(45, 156, 255, 0.2);
        }
    )");
    m_changeEmployeeButton->setVisible(false);

    employeeButtonLayout->addWidget(m_employeeButton);
    employeeButtonLayout->addWidget(m_employeeLabel);
    employeeButtonLayout->addWidget(m_changeEmployeeButton);
    employeeButtonLayout->addStretch();

    employeeSelectionLayout->addWidget(employeeLabel);
    employeeSelectionLayout->addWidget(employeeButtonWidget);

    // Salary Settings
    QFrame *salarySettingsFrame = new QFrame(leftColumn);
    salarySettingsFrame->setStyleSheet(R"(
        QFrame {
            background: #f8f9fa;
            border: 1px solid #e0e0e0;
            border-radius: 10px;
            padding: 0px;
        }
    )");
    QVBoxLayout *salarySettingsLayout = new QVBoxLayout(salarySettingsFrame);
    salarySettingsLayout->setContentsMargins(20, 15, 20, 15);
    salarySettingsLayout->setSpacing(15);

    QLabel *settingsTitle = new QLabel("Настройки зарплаты", salarySettingsFrame);
    settingsTitle->setStyleSheet("font-weight: bold; color: #2d9cff; font-size: 16px; background: transparent; border: none;");

    // Base Salary
    QLabel *baseSalaryLabel = new QLabel("Оклад (руб):", salarySettingsFrame);
    baseSalaryLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 13px; background: transparent; border: none;");

    m_baseSalaryInput = new QLineEdit(salarySettingsFrame);
    m_baseSalaryInput->setPlaceholderText("0.00");
    m_baseSalaryInput->setFixedHeight(35);
    m_baseSalaryInput->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #ddd;
            border-radius: 6px;
            padding: 0 12px;
            font-size: 13px;
        }
        QLineEdit:focus {
            border: 1px solid #2d9cff;
        }
    )");

    // Percentage
    QLabel *percentageLabel = new QLabel("Процент (%):", salarySettingsFrame);
    percentageLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 13px; background: transparent; border: none;");

    m_percentageInput = new QLineEdit(salarySettingsFrame);
    m_percentageInput->setPlaceholderText("0.0");
    m_percentageInput->setFixedHeight(35);
    m_percentageInput->setStyleSheet(m_baseSalaryInput->styleSheet());

    QPushButton *saveSettingsBtn = new QPushButton("Сохранить настройки", salarySettingsFrame);
    saveSettingsBtn->setFixedHeight(35);
    saveSettingsBtn->setCursor(Qt::PointingHandCursor);
    saveSettingsBtn->setStyleSheet(R"(
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

    salarySettingsLayout->addWidget(settingsTitle);
    salarySettingsLayout->addWidget(baseSalaryLabel);
    salarySettingsLayout->addWidget(m_baseSalaryInput);
    salarySettingsLayout->addWidget(percentageLabel);
    salarySettingsLayout->addWidget(m_percentageInput);
    salarySettingsLayout->addWidget(saveSettingsBtn);

    // Earnings Statistics
    QFrame *earningsFrame = new QFrame(leftColumn);
    earningsFrame->setStyleSheet(R"(
        QFrame {
            background: #f8f9fa;
            border: 1px solid #e0e0e0;
            border-radius: 10px;
            padding: 0px;
        }
    )");
    QVBoxLayout *earningsLayout = new QVBoxLayout(earningsFrame);
    earningsLayout->setContentsMargins(20, 15, 20, 15);
    earningsLayout->setSpacing(12);

    QLabel *earningsTitle = new QLabel("Заработок за текущий месяц", earningsFrame);
    earningsTitle->setStyleSheet("font-weight: bold; color: #2d9cff; font-size: 16px; background: transparent; border: none;");

    m_totalEarningsLabel = createStatLabel("Общий заработок:", "0 ₽", "#2d9cff");
    m_salaryEarningsLabel = createStatLabel("Оклад:", "0 ₽", "#27ae60");
    m_percentageEarningsLabel = createStatLabel("Процент:", "0 ₽", "#3498db");
    m_bonusEarningsLabel = createStatLabel("Премии:", "0 ₽", "#9b59b6");
    m_deductionEarningsLabel = createStatLabel("Вычеты:", "0 ₽", "#e74c3c");
    m_netEarningsLabel = createStatLabel("К выплате:", "0 ₽", "#e67e22");
    m_completedServicesLabel = createStatLabel("Выполнено услуг:", "0", "#95a5a6");

    earningsLayout->addWidget(earningsTitle);
    earningsLayout->addWidget(m_totalEarningsLabel);
    earningsLayout->addWidget(m_salaryEarningsLabel);
    earningsLayout->addWidget(m_percentageEarningsLabel);
    earningsLayout->addWidget(m_bonusEarningsLabel);
    earningsLayout->addWidget(m_deductionEarningsLabel);
    earningsLayout->addWidget(m_netEarningsLabel);
    earningsLayout->addWidget(m_completedServicesLabel);

    leftLayout->addWidget(employeeSelectionWidget);
    leftLayout->addWidget(salarySettingsFrame);
    leftLayout->addWidget(earningsFrame);
    leftLayout->addStretch();

    // Right Column - Financial Operations
    QWidget *rightColumn = new QWidget(contentWidget);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightColumn);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(15);

    // Add Financial Operation
    QFrame *addOperationFrame = new QFrame(rightColumn);
    addOperationFrame->setStyleSheet(R"(
        QFrame {
            background: #f8f9fa;
            border: 1px solid #e0e0e0;
            border-radius: 10px;
            padding: 0px;
        }
    )");
    QVBoxLayout *addOperationLayout = new QVBoxLayout(addOperationFrame);
    addOperationLayout->setContentsMargins(20, 15, 20, 15);
    addOperationLayout->setSpacing(12);

    QLabel *addOperationTitle = new QLabel("Добавить финансовую операцию", addOperationFrame);
    addOperationTitle->setStyleSheet("font-weight: bold; color: #2d9cff; font-size: 16px; background: transparent; border: none;");

    // Operation Type
    QWidget *typeWidget = new QWidget(addOperationFrame);
    QHBoxLayout *typeLayout = new QHBoxLayout(typeWidget);
    typeLayout->setContentsMargins(0,0,0,0);
    typeLayout->setSpacing(10);

    QLabel *typeLabel = new QLabel("Тип операции:", typeWidget);
    typeLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 13px; background: transparent; border: none;");

    m_operationTypeButton = new QPushButton("Выбрать тип", typeWidget);
    m_operationTypeButton->setFixedHeight(35);
    m_operationTypeButton->setCursor(Qt::PointingHandCursor);
    m_operationTypeButton->setStyleSheet(m_employeeButton->styleSheet());

    m_operationTypeLabel = new QLabel("", typeWidget);
    m_operationTypeLabel->setStyleSheet("font-size: 13px; color: #333; padding: 0 10px; background: transparent; border: none;");
    m_operationTypeLabel->setVisible(false);

    m_changeOperationTypeButton = new QPushButton("Изменить", typeWidget);
    m_changeOperationTypeButton->setFixedSize(80, 35);
    m_changeOperationTypeButton->setCursor(Qt::PointingHandCursor);
    m_changeOperationTypeButton->setStyleSheet(m_changeEmployeeButton->styleSheet());
    m_changeOperationTypeButton->setVisible(false);

    typeLayout->addWidget(typeLabel);
    typeLayout->addWidget(m_operationTypeButton);
    typeLayout->addWidget(m_operationTypeLabel);
    typeLayout->addWidget(m_changeOperationTypeButton);
    typeLayout->addStretch();

    // Amount only (без даты)
    QWidget *amountWidget = new QWidget(addOperationFrame);
    QHBoxLayout *amountLayout = new QHBoxLayout(amountWidget);
    amountLayout->setContentsMargins(0,0,0,0);
    amountLayout->setSpacing(10);

    QLabel *amountLabel = new QLabel("Сумма:", amountWidget);
    amountLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 13px; background: transparent; border: none;");

    m_operationAmountInput = new QLineEdit(amountWidget);
    m_operationAmountInput->setPlaceholderText("0.00");
    m_operationAmountInput->setFixedHeight(35);
    m_operationAmountInput->setStyleSheet(m_baseSalaryInput->styleSheet());

    amountLayout->addWidget(amountLabel);
    amountLayout->addWidget(m_operationAmountInput);
    amountLayout->addStretch();

    // Description
    QWidget *descWidget = new QWidget(addOperationFrame);
    QVBoxLayout *descLayout = new QVBoxLayout(descWidget);
    descLayout->setContentsMargins(0,0,0,0);
    descLayout->setSpacing(5);

    QLabel *descLabel = new QLabel("Описание:", descWidget);
    descLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 13px; background: transparent; border: none;");

    m_operationDescriptionInput = new QLineEdit(descWidget);
    m_operationDescriptionInput->setPlaceholderText("Описание операции");
    m_operationDescriptionInput->setFixedHeight(35);
    m_operationDescriptionInput->setStyleSheet(m_baseSalaryInput->styleSheet());

    descLayout->addWidget(descLabel);
    descLayout->addWidget(m_operationDescriptionInput);

    QPushButton *addOperationBtn = new QPushButton("Добавить операцию", addOperationFrame);
    addOperationBtn->setFixedHeight(35);
    addOperationBtn->setCursor(Qt::PointingHandCursor);
    addOperationBtn->setStyleSheet(R"(
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

    addOperationLayout->addWidget(addOperationTitle);
    addOperationLayout->addWidget(typeWidget);
    addOperationLayout->addWidget(amountWidget);
    addOperationLayout->addWidget(descWidget);
    addOperationLayout->addWidget(addOperationBtn);

    // Operations History
    QLabel *operationsTitle = new QLabel("История операций", rightColumn);
    operationsTitle->setStyleSheet("font-weight: bold; color: #2d9cff; font-size: 16px; background: transparent; border: none;");

    QScrollArea *operationsScrollArea = new QScrollArea(rightColumn);
    operationsScrollArea->setWidgetResizable(true);
    operationsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    operationsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    operationsScrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    m_operationsContainer = new QWidget();
    m_operationsLayout = new QVBoxLayout(m_operationsContainer);
    m_operationsLayout->setSpacing(10);
    m_operationsLayout->setContentsMargins(2, 2, 2, 2);
    m_operationsLayout->setAlignment(Qt::AlignTop);

    operationsScrollArea->setWidget(m_operationsContainer);

    rightLayout->addWidget(addOperationFrame);
    rightLayout->addWidget(operationsTitle);
    rightLayout->addWidget(operationsScrollArea);

    contentLayout->addWidget(leftColumn);
    contentLayout->addWidget(rightColumn);

    cardLayout->addWidget(contentWidget);

    root->addWidget(card, 0, Qt::AlignCenter);

    // Connections
    connect(backBtn, &QPushButton::clicked, this, &PayrollWindow::onBackClicked);
    connect(closeBtn, &QPushButton::clicked, this, &PayrollWindow::onCloseClicked);
    connect(m_employeeButton, &QPushButton::clicked, this, &PayrollWindow::onEmployeeButtonClicked);
    connect(m_changeEmployeeButton, &QPushButton::clicked, this, &PayrollWindow::onEmployeeButtonClicked);
    connect(m_operationTypeButton, &QPushButton::clicked, this, &PayrollWindow::onOperationTypeButtonClicked);
    connect(m_changeOperationTypeButton, &QPushButton::clicked, this, &PayrollWindow::onOperationTypeButtonClicked);
    connect(saveSettingsBtn, &QPushButton::clicked, this, &PayrollWindow::onSaveSalarySettings);
    connect(addOperationBtn, &QPushButton::clicked, this, &PayrollWindow::onAddFinancialOperation);

    setLayout(root);
}

QLabel* PayrollWindow::createStatLabel(const QString &title, const QString &value, const QString &color) {
    QLabel *label = new QLabel(QString("%1: %2").arg(title).arg(value));
    label->setStyleSheet(QString("font-weight: bold; color: %1; font-size: 13px; background: transparent; border: none;").arg(color));
    return label;
}

void PayrollWindow::refreshEmployeesList() {
    // Метод теперь не используется, так как используем диалог выбора
}

void PayrollWindow::onEmployeeButtonClicked() {
    QDialog dialog(this);
    dialog.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dialog.setAttribute(Qt::WA_TranslucentBackground);
    dialog.setFixedSize(400, 400);

    QFrame *dialogCard = new QFrame(&dialog);
    dialogCard->setObjectName("dialogCard");
    dialogCard->setStyleSheet(R"(
        QFrame#dialogCard {
            background: white;
            border-radius: 12px;
            border: 1px solid #e0e0e0;
        }
    )");
    dialogCard->setFixedSize(400, 400);

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

    QLabel *titleLabel = new QLabel("Выберите сотрудника", dialogCard);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2d9cff;");
    titleLabel->setAlignment(Qt::AlignCenter);

    cardLayout->addWidget(titleLabel);
    cardLayout->addSpacing(10);

    QScrollArea *scrollArea = new QScrollArea(dialogCard);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    QWidget *scrollWidget = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setSpacing(5);
    scrollLayout->setAlignment(Qt::AlignTop);

    QSqlQuery query("SELECT id, first_name, last_name, role FROM users WHERE role IN ('Master', 'Admin') AND is_active = TRUE ORDER BY role, last_name, first_name");

    QButtonGroup *buttonGroup = new QButtonGroup(&dialog);

    while (query.next()) {
        int userId = query.value(0).toInt();
        QString firstName = query.value(1).toString();
        QString lastName = query.value(2).toString();
        QString role = query.value(3).toString();

        QString displayName = QString("%1 %2 (%3)").arg(lastName).arg(firstName).arg(role == "Master" ? "Мастер" : "Администратор");

        QPushButton *employeeBtn = new QPushButton(displayName, scrollWidget);
        employeeBtn->setFixedHeight(45);
        employeeBtn->setCheckable(true);
        employeeBtn->setProperty("userId", userId);
        employeeBtn->setStyleSheet(R"(
            QPushButton {
                border: 1px solid #ddd;
                border-radius: 6px;
                padding: 0 15px;
                font-size: 14px;
                background: white;
                text-align: left;
            }
            QPushButton:hover {
                border: 1px solid #2d9cff;
                background: #f8f9fa;
            }
            QPushButton:checked {
                border: 2px solid #2d9cff;
                background: rgba(45, 156, 255, 0.1);
            }
        )");

        buttonGroup->addButton(employeeBtn);
        scrollLayout->addWidget(employeeBtn);
    }

    scrollArea->setWidget(scrollWidget);
    cardLayout->addWidget(scrollArea);
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

    QPushButton *confirmBtn = new QPushButton("Выбрать", dialogCard);
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

    // Центрируем диалог относительно основного окна
    QPoint centerPos = mapToGlobal(rect().center());
    dialog.move(centerPos.x() - dialog.width()/2, centerPos.y() - dialog.height()/2);

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(confirmBtn, &QPushButton::clicked, [&]() {
        QAbstractButton *selected = buttonGroup->checkedButton();
        if (selected) {
            int userId = selected->property("userId").toInt();
            QString displayName = selected->text();

            m_selectedEmployeeId = userId;
            m_employeeButton->setVisible(false);
            m_employeeLabel->setText(displayName);
            m_employeeLabel->setVisible(true);
            m_changeEmployeeButton->setVisible(true);
            refreshSalaryData();
            refreshFinancialOperations();
            dialog.accept();
        } else {
            QMessageBox::warning(&dialog, "Ошибка", "Выберите сотрудника!");
        }
    });

    dialog.exec();
}

void PayrollWindow::onOperationTypeButtonClicked() {
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

    QLabel *titleLabel = new QLabel("Выберите тип операции", dialogCard);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2d9cff;");
    titleLabel->setAlignment(Qt::AlignCenter);

    cardLayout->addWidget(titleLabel);
    cardLayout->addSpacing(10);

    QMap<QString, QString> operationTypes = {
        {"bonus", "Премия"},
        {"advance", "Аванс"},
        {"deduction", "Вычет"},
        {"salary", "Зарплата"}
    };

    QButtonGroup *buttonGroup = new QButtonGroup(&dialog);

    for (auto it = operationTypes.begin(); it != operationTypes.end(); ++it) {
        QPushButton *typeBtn = new QPushButton(it.value(), dialogCard);
        typeBtn->setFixedHeight(45);
        typeBtn->setCheckable(true);
        typeBtn->setProperty("type", it.key());
        typeBtn->setStyleSheet(R"(
            QPushButton {
                border: 1px solid #ddd;
                border-radius: 6px;
                padding: 0 15px;
                font-size: 14px;
                background: white;
                text-align: left;
            }
            QPushButton:hover {
                border: 1px solid #2d9cff;
                background: #f8f9fa;
            }
            QPushButton:checked {
                border: 2px solid #2d9cff;
                background: rgba(45, 156, 255, 0.1);
            }
        )");

        buttonGroup->addButton(typeBtn);
        cardLayout->addWidget(typeBtn);
    }

    cardLayout->addStretch();

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    QPushButton *cancelBtn = new QPushButton("Отмена", dialogCard);
    cancelBtn->setFixedHeight(35);
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

    QPushButton *confirmBtn = new QPushButton("Выбрать", dialogCard);
    confirmBtn->setFixedHeight(35);
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

    // Центрируем диалог
    QPoint centerPos = mapToGlobal(rect().center());
    dialog.move(centerPos.x() - dialog.width()/2, centerPos.y() - dialog.height()/2);

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(confirmBtn, &QPushButton::clicked, [&]() {
        QAbstractButton *selected = buttonGroup->checkedButton();
        if (selected) {
            QString type = selected->property("type").toString();
            QString displayName = selected->text();

            m_operationTypeButton->setVisible(false);
            m_operationTypeLabel->setText(displayName);
            m_operationTypeLabel->setVisible(true);
            m_changeOperationTypeButton->setVisible(true);
            m_currentOperationType = type;

            dialog.accept();
        } else {
            QMessageBox::warning(&dialog, "Ошибка", "Выберите тип операции!");
        }
    });

    dialog.exec();
}

void PayrollWindow::refreshSalaryData() {
    if (m_selectedEmployeeId == -1) return;

    // Загружаем настройки зарплаты
    QSqlQuery settingsQuery;
    settingsQuery.prepare("SELECT base_salary, percentage FROM salary_settings WHERE user_id = ?");
    settingsQuery.addBindValue(m_selectedEmployeeId);

    if (settingsQuery.exec() && settingsQuery.next()) {
        double baseSalary = settingsQuery.value(0).toDouble();
        double percentage = settingsQuery.value(1).toDouble();

        m_baseSalaryInput->setText(QString::number(baseSalary, 'f', 2));
        m_percentageInput->setText(QString::number(percentage, 'f', 1));
    } else {
        m_baseSalaryInput->setText("0.00");
        m_percentageInput->setText("0.0");
    }

    // Рассчитываем заработок за текущий месяц
    calculateCurrentMonthEarnings();
}

void PayrollWindow::calculateCurrentMonthEarnings() {
    if (m_selectedEmployeeId == -1) return;

    QDate currentDate = QDate::currentDate();
    QDate monthStart = QDate(currentDate.year(), currentDate.month(), 1);
    QDate monthEnd = QDate(currentDate.year(), currentDate.month(), currentDate.daysInMonth());

    // Заработок от услуг
    double serviceEarnings = calculateEmployeeEarnings(m_selectedEmployeeId, monthStart, monthEnd);

    // Финансовые операции за месяц
    double bonusAmount = 0, deductionAmount = 0, advanceAmount = 0, salaryAmount = 0;

    QSqlQuery operationsQuery;
    operationsQuery.prepare(
        "SELECT type, SUM(amount) as total "
        "FROM employee_financial_operations "
        "WHERE user_id = ? AND operation_date BETWEEN ? AND ? "
        "GROUP BY type"
    );
    operationsQuery.addBindValue(m_selectedEmployeeId);
    operationsQuery.addBindValue(monthStart.toString("yyyy-MM-dd"));
    operationsQuery.addBindValue(monthEnd.toString("yyyy-MM-dd"));

    if (operationsQuery.exec()) {
        while (operationsQuery.next()) {
            QString type = operationsQuery.value(0).toString();
            double amount = operationsQuery.value(1).toDouble();

            if (type == "bonus") bonusAmount = amount;
            else if (type == "deduction") deductionAmount = amount;
            else if (type == "advance") advanceAmount = amount;
            else if (type == "salary") salaryAmount = amount;
        }
    }

    // Количество выполненных услуг
    int completedServices = 0;
    QSqlQuery servicesQuery;
    servicesQuery.prepare(
        "SELECT COUNT(*) FROM appointments "
        "WHERE master_id = ? AND status = 'completed' AND appointment_date BETWEEN ? AND ?"
    );
    servicesQuery.addBindValue(m_selectedEmployeeId);
    servicesQuery.addBindValue(monthStart.toString("yyyy-MM-dd"));
    servicesQuery.addBindValue(monthEnd.toString("yyyy-MM-dd"));

    if (servicesQuery.exec() && servicesQuery.next()) {
        completedServices = servicesQuery.value(0).toInt();
    }

    // Итоговые расчеты
    double totalEarnings = serviceEarnings + bonusAmount + salaryAmount;
    double netEarnings = totalEarnings - deductionAmount - advanceAmount;

    // Обновляем UI напрямую
    if (m_totalEarningsLabel)
        m_totalEarningsLabel->setText(QString("Общий заработок: %1 ₽").arg(totalEarnings, 0, 'f', 2));

    if (m_salaryEarningsLabel)
        m_salaryEarningsLabel->setText(QString("Оклад: %1 ₽").arg(salaryAmount, 0, 'f', 2));

    if (m_percentageEarningsLabel)
        m_percentageEarningsLabel->setText(QString("Процент: %1 ₽").arg(serviceEarnings, 0, 'f', 2));

    if (m_bonusEarningsLabel)
        m_bonusEarningsLabel->setText(QString("Премии: %1 ₽").arg(bonusAmount, 0, 'f', 2));

    if (m_deductionEarningsLabel)
        m_deductionEarningsLabel->setText(QString("Вычеты: %1 ₽").arg(deductionAmount, 0, 'f', 2));

    if (m_netEarningsLabel)
        m_netEarningsLabel->setText(QString("К выплате: %1 ₽").arg(netEarnings, 0, 'f', 2));

    if (m_completedServicesLabel)
        m_completedServicesLabel->setText(QString("Выполнено услуг: %1").arg(completedServices));
}

double PayrollWindow::calculateEmployeeEarnings(int userId, const QDate &startDate, const QDate &endDate) {
    double percentage = getEmployeePercentage(userId);

    QSqlQuery query;
    query.prepare(
        "SELECT COALESCE(SUM(s.price), 0) as total_earnings "
        "FROM appointments a "
        "JOIN services s ON a.service_id = s.id "
        "WHERE a.master_id = ? AND a.status = 'completed' AND a.appointment_date BETWEEN ? AND ?"
    );
    query.addBindValue(userId);
    query.addBindValue(startDate.toString("yyyy-MM-dd"));
    query.addBindValue(endDate.toString("yyyy-MM-dd"));

    if (query.exec() && query.next()) {
        double totalServices = query.value(0).toDouble();
        return totalServices * (percentage / 100.0);
    }

    return 0.0;
}

double PayrollWindow::getEmployeePercentage(int userId) {
    QSqlQuery query;
    query.prepare("SELECT percentage FROM salary_settings WHERE user_id = ?");
    query.addBindValue(userId);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

double PayrollWindow::getEmployeeBaseSalary(int userId) {
    QSqlQuery query;
    query.prepare("SELECT base_salary FROM salary_settings WHERE user_id = ?");
    query.addBindValue(userId);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

void PayrollWindow::refreshFinancialOperations() {
    // Clear existing operations
    QLayoutItem* item;
    while ((item = m_operationsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->hide();
            item->widget()->deleteLater();
        }
        delete item;
    }

    if (m_selectedEmployeeId == -1) return;

    QSqlQuery query;
    query.prepare(
        "SELECT id, amount, type, description, operation_date "
        "FROM employee_financial_operations "
        "WHERE user_id = ? "
        "ORDER BY operation_date DESC, id DESC"
    );
    query.addBindValue(m_selectedEmployeeId);

    if (!query.exec()) {
        qDebug() << "Error getting financial operations:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        int operationId = query.value(0).toInt();
        double amount = query.value(1).toDouble();
        QString type = query.value(2).toString();
        QString description = query.value(3).toString();
        QDate date = query.value(4).toDate();

        // Create operation card
        QFrame *card = new QFrame(m_operationsContainer);
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

        if (type == "bonus" || type == "salary" || type == "advance") {
            amountLabel->setText("+" + amountText);
            amountColor = "#27ae60"; // зеленый для премий, зарплат и авансов
        } else if (type == "deduction") {
            amountLabel->setText("-" + amountText);
            amountColor = "#e74c3c"; // красный для вычетов
        } else {
            amountLabel->setText(amountText);
            amountColor = "#333333";
        }

        amountLabel->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 14px; min-width: 80px;").arg(amountColor));

        // Type badge
        QLabel *typeLabel = new QLabel(card);
        QString typeText, typeColor, typeBg;

        if (type == "bonus") {
            typeText = "ПРЕМИЯ";
            typeColor = "#9b59b6";
            typeBg = "rgba(155, 89, 182, 0.1)";
        } else if (type == "advance") {
            typeText = "АВАНС";
            typeColor = "#27ae60"; // зеленый для аванса
            typeBg = "rgba(39, 174, 96, 0.1)";
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
            "    border: 1px solid rgba(%3,0.2);"
            "}"
        ).arg(typeColor).arg(typeBg).arg(typeColor.mid(1)));
        typeLabel->setAlignment(Qt::AlignCenter);

        // Description - убираем серую рамку
        QLabel *descLabel = new QLabel(description, card);
        descLabel->setStyleSheet("color: #666; font-size: 12px; background: transparent; border: none; padding: 0;");
        descLabel->setWordWrap(true);

        // Date
        QLabel *dateLabel = new QLabel(date.toString("dd.MM.yyyy"), card);
        dateLabel->setStyleSheet("color: #999; font-size: 11px; min-width: 80px; background: transparent; border: none;");

        // Delete button
        QPushButton *deleteBtn = new QPushButton("✕", card);
        deleteBtn->setFixedSize(24, 24);
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setProperty("operationId", operationId);
        deleteBtn->setStyleSheet(R"(
            QPushButton {
                background: transparent;
                border: 1px solid #e0e0e0;
                border-radius: 4px;
                color: #999;
                font-size: 10px;
                font-weight: bold;
            }
            QPushButton:hover {
                background: rgba(231, 76, 60, 0.1);
                border: 1px solid #e74c3c;
                color: #e74c3c;
            }
        )");

        cardLayout->addWidget(amountLabel);
        cardLayout->addWidget(typeLabel);
        cardLayout->addWidget(descLabel);
        cardLayout->addStretch();
        cardLayout->addWidget(dateLabel);
        cardLayout->addWidget(deleteBtn);

        // Connect delete button
        connect(deleteBtn, &QPushButton::clicked, this, [this, operationId]() {
            onDeleteOperation(operationId);
        });

        m_operationsLayout->addWidget(card);
    }

    // If no operations
    if (!query.first()) {
        QLabel *noDataLabel = new QLabel("Нет финансовых операций", m_operationsContainer);
        noDataLabel->setStyleSheet("color: #999; font-size: 14px; text-align: center; padding: 40px; background: transparent; border: none;");
        noDataLabel->setAlignment(Qt::AlignCenter);
        m_operationsLayout->addWidget(noDataLabel);
    }
}

void PayrollWindow::onDeleteOperation(int operationId) {
    // Создаем диалог подтверждения в стиле приложения
    QDialog dialog(this);
    dialog.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dialog.setAttribute(Qt::WA_TranslucentBackground);
    dialog.setFixedSize(400, 200);

    QFrame *dialogCard = new QFrame(&dialog);
    dialogCard->setObjectName("dialogCard");
    dialogCard->setStyleSheet(R"(
        QFrame#dialogCard {
            background: white;
            border-radius: 12px;
            border: 1px solid #e0e0e0;
        }
    )");
    dialogCard->setFixedSize(400, 200);

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

    // Иконка предупреждения
    QLabel *iconLabel = new QLabel("⚠️", dialogCard);
    iconLabel->setStyleSheet("font-size: 32px; background: transparent; border: none;");
    iconLabel->setAlignment(Qt::AlignCenter);

    // Текст подтверждения
    QLabel *messageLabel = new QLabel("Вы уверены, что хотите удалить эту операцию?", dialogCard);
    messageLabel->setStyleSheet("font-size: 14px; color: #333; text-align: center; background: transparent; border: none;");
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setWordWrap(true);

    cardLayout->addWidget(iconLabel);
    cardLayout->addWidget(messageLabel);
    cardLayout->addStretch();

    // Кнопки
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

    QPushButton *deleteBtn = new QPushButton("Удалить", dialogCard);
    deleteBtn->setFixedHeight(35);
    deleteBtn->setCursor(Qt::PointingHandCursor);
    deleteBtn->setStyleSheet(R"(
        QPushButton {
            background: #e74c3c;
            color: white;
            border-radius: 6px;
            font-weight: 600;
            border: none;
            font-size: 13px;
        }
        QPushButton:hover {
            background: #c0392b;
        }
    )");

    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(deleteBtn);
    cardLayout->addLayout(buttonLayout);

    // Центрируем диалог
    QPoint centerPos = mapToGlobal(rect().center());
    dialog.move(centerPos.x() - dialog.width()/2, centerPos.y() - dialog.height()/2);

    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(deleteBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    // Показываем диалог и обрабатываем результат
    if (dialog.exec() == QDialog::Accepted) {
        QSqlQuery query;
        query.prepare("DELETE FROM employee_financial_operations WHERE id = ?");
        query.addBindValue(operationId);

        if (query.exec()) {
            qDebug() << "Операция удалена:" << operationId;
            // Обновляем данные
            refreshSalaryData();
            refreshFinancialOperations();
            emit dataUpdated(); // Уведомляем об обновлении
        } else {
            qDebug() << "Ошибка удаления операции:" << query.lastError().text();
            QMessageBox::warning(this, "Ошибка", "Не удалось удалить операцию!");
        }
    }
}

void PayrollWindow::onSaveSalarySettings() {
    if (m_selectedEmployeeId == -1) return;

    bool ok1, ok2;
    double baseSalary = m_baseSalaryInput->text().toDouble(&ok1);
    double percentage = m_percentageInput->text().toDouble(&ok2);

    if (!ok1 || !ok2) {
        QMessageBox::warning(this, "Ошибка", "Введите корректные числовые значения!");
        return;
    }

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM salary_settings WHERE user_id = ?");
    checkQuery.addBindValue(m_selectedEmployeeId);

    if (checkQuery.exec() && checkQuery.next()) {
        bool exists = checkQuery.value(0).toInt() > 0;

        QSqlQuery query;
        if (exists) {
            query.prepare("UPDATE salary_settings SET base_salary = ?, percentage = ?, updated_at = CURRENT_TIMESTAMP WHERE user_id = ?");
        } else {
            query.prepare("INSERT INTO salary_settings (user_id, base_salary, percentage) VALUES (?, ?, ?)");
        }

        query.addBindValue(baseSalary);
        query.addBindValue(percentage);
        query.addBindValue(m_selectedEmployeeId);

        if (query.exec()) {
            QMessageBox::information(this, "Успех", "Настройки зарплаты сохранены!");
            refreshSalaryData();
            emit dataUpdated(); // Уведомляем об обновлении
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось сохранить настройки зарплаты!");
        }
    }
}

void PayrollWindow::onAddFinancialOperation() {
    if (m_selectedEmployeeId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите сотрудника!");
        return;
    }

    if (m_currentOperationType.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите тип операции!");
        return;
    }

    bool ok;
    double amount = m_operationAmountInput->text().toDouble(&ok);
    QString description = m_operationDescriptionInput->text().trimmed();
    QDate date = QDate::currentDate();

    // Добавляем period_month (текущий месяц в формате YYYY-MM)
    QString periodMonth = date.toString("yyyy-MM");

    if (!ok || amount <= 0) {
        QMessageBox::warning(this, "Ошибка", "Введите корректную сумму!");
        return;
    }

    if (description.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите описание операции!");
        return;
    }

    QSqlQuery query;
    query.prepare(
        "INSERT INTO employee_financial_operations (user_id, amount, type, description, operation_date, created_by, period_month) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)"
    );

    query.addBindValue(m_selectedEmployeeId);
    query.addBindValue(amount);
    query.addBindValue(m_currentOperationType);
    query.addBindValue(description);
    query.addBindValue(date.toString("yyyy-MM-dd"));
    query.addBindValue(1); // owner user id
    query.addBindValue(periodMonth); // добавляем period_month

    if (query.exec()) {
        qDebug() << "Операция успешно добавлена в БД";
        // Очищаем поля ввода и сбрасываем выбор типа операции
        m_operationAmountInput->clear();
        m_operationDescriptionInput->clear();
        m_operationTypeButton->setVisible(true);
        m_operationTypeLabel->setVisible(false);
        m_changeOperationTypeButton->setVisible(false);
        m_currentOperationType = "";

        // Обновляем данные
        refreshSalaryData();
        refreshFinancialOperations();
        emit dataUpdated(); // Уведомляем об обновлении

        // Убрано всплывающее окно об успешном добавлении
    } else {
        qDebug() << "Ошибка SQL:" << query.lastError().text();
        qDebug() << "Запрос:" << query.lastQuery();
        QMessageBox::warning(this, "Ошибка", "Не удалось добавить операцию: " + query.lastError().text());
    }
}

void PayrollWindow::onRefreshData() {
    refreshSalaryData();
    refreshFinancialOperations();
}

void PayrollWindow::onBackClicked() {
    m_selectedEmployeeId = -1;
    m_employeeButton->setVisible(true);
    m_employeeLabel->setVisible(false);
    m_changeEmployeeButton->setVisible(false);
    emit backClicked();
}

void PayrollWindow::onCloseClicked() {
    close();
}

void PayrollWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void PayrollWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}
