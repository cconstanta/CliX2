#include "financeswindow.h"
#include "database.h"
#include <QtWidgets>
#include <QDebug>
#include <QHeaderView>

FinancesWindow::FinancesWindow(QWidget *parent) : QWidget(parent)
{
    setupUI();
    refreshFinanceData();
}

FinancesWindow::~FinancesWindow() {
}

void FinancesWindow::setupUI() {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(1200, 850);

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
    card->setFixedSize(1160, 810);

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

    QLabel *title = new QLabel("Управление финансами", topBar);
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

    // Statistics Cards
    QWidget *statsWidget = new QWidget(card);
    statsWidget->setFixedHeight(100);
    QHBoxLayout *statsLayout = new QHBoxLayout(statsWidget);
    statsLayout->setContentsMargins(0, 10, 0, 10);
    statsLayout->setSpacing(30);

    // Income
    QWidget *incomeWidget = new QWidget(statsWidget);
    QVBoxLayout *incomeLayout = new QVBoxLayout(incomeWidget);
    incomeLayout->setContentsMargins(0,0,0,0);
    incomeLayout->setSpacing(5);

    QLabel *incomeTitle = new QLabel("ДОХОДЫ", incomeWidget);
    incomeTitle->setStyleSheet("color: #27ae60; font-size: 14px; font-weight: bold;");
    m_totalIncomeLabel = new QLabel("0 ₽", incomeWidget);
    m_totalIncomeLabel->setStyleSheet("color: #27ae60; font-size: 22px; font-weight: bold;");

    incomeLayout->addWidget(incomeTitle, 0, Qt::AlignCenter);
    incomeLayout->addWidget(m_totalIncomeLabel, 0, Qt::AlignCenter);

    // Expenses
    QWidget *expensesWidget = new QWidget(statsWidget);
    QVBoxLayout *expensesLayout = new QVBoxLayout(expensesWidget);
    expensesLayout->setContentsMargins(0,0,0,0);
    expensesLayout->setSpacing(5);

    QLabel *expensesTitle = new QLabel("РАСХОДЫ", expensesWidget);
    expensesTitle->setStyleSheet("color: #c0392b; font-size: 14px; font-weight: bold;");
    m_totalExpensesLabel = new QLabel("0 ₽", expensesWidget);
    m_totalExpensesLabel->setStyleSheet("color: #c0392b; font-size: 22px; font-weight: bold;");

    expensesLayout->addWidget(expensesTitle, 0, Qt::AlignCenter);
    expensesLayout->addWidget(m_totalExpensesLabel, 0, Qt::AlignCenter);

    // Profit
    QWidget *profitWidget = new QWidget(statsWidget);
    QVBoxLayout *profitLayout = new QVBoxLayout(profitWidget);
    profitLayout->setContentsMargins(0,0,0,0);
    profitLayout->setSpacing(5);

    QLabel *profitTitle = new QLabel("ПРИБЫЛЬ", profitWidget);
    profitTitle->setStyleSheet("color: #2980b9; font-size: 14px; font-weight: bold;");
    m_balanceLabel = new QLabel("0 ₽", profitWidget);
    m_balanceLabel->setStyleSheet("color: #2980b9; font-size: 22px; font-weight: bold;");

    profitLayout->addWidget(profitTitle, 0, Qt::AlignCenter);
    profitLayout->addWidget(m_balanceLabel, 0, Qt::AlignCenter);

    statsLayout->addStretch();
    statsLayout->addWidget(incomeWidget);
    statsLayout->addWidget(expensesWidget);
    statsLayout->addWidget(profitWidget);
    statsLayout->addStretch();

    cardLayout->addWidget(statsWidget);

    // Filters and Add Button - упрощенный дизайн без категорий
    QWidget *filtersWidget = new QWidget(card);
    filtersWidget->setFixedHeight(60);
    QHBoxLayout *filtersLayout = new QHBoxLayout(filtersWidget);
    filtersLayout->setContentsMargins(0, 5, 0, 5);
    filtersLayout->setSpacing(15);

    QLabel *filterLabel = new QLabel("Период:", filtersWidget);
    filterLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px; margin-top: 12px;");

    // Дата начала
    QWidget *startDateWidget = new QWidget(filtersWidget);
    QVBoxLayout *startDateLayout = new QVBoxLayout(startDateWidget);
    startDateLayout->setContentsMargins(0,0,0,0);
    startDateLayout->setSpacing(2);

    QLabel *startDateLabel = new QLabel("С:", startDateWidget);
    startDateLabel->setStyleSheet("color: #666; font-size: 12px;");

    m_startDateFilter = new QDateEdit(filtersWidget);
    m_startDateFilter->setDate(QDate::currentDate().addMonths(-1));
    m_startDateFilter->setCalendarPopup(true);
    m_startDateFilter->setFixedSize(120, 35);
    m_startDateFilter->setStyleSheet(R"(
        QDateEdit {
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 0 12px;
            font-size: 13px;
            background: white;
        }
        QDateEdit:hover {
            border: 1px solid #2d9cff;
        }
        QDateEdit::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 25px;
            border-left: 1px solid #ddd;
        }
    )");

    startDateLayout->addWidget(startDateLabel);
    startDateLayout->addWidget(m_startDateFilter);

    // Дата окончания
    QWidget *endDateWidget = new QWidget(filtersWidget);
    QVBoxLayout *endDateLayout = new QVBoxLayout(endDateWidget);
    endDateLayout->setContentsMargins(0,0,0,0);
    endDateLayout->setSpacing(2);

    QLabel *endDateLabel = new QLabel("По:", endDateWidget);
    endDateLabel->setStyleSheet("color: #666; font-size: 12px;");

    m_endDateFilter = new QDateEdit(filtersWidget);
    m_endDateFilter->setDate(QDate::currentDate());
    m_endDateFilter->setCalendarPopup(true);
    m_endDateFilter->setFixedSize(120, 35);
    m_endDateFilter->setStyleSheet(m_startDateFilter->styleSheet());

    endDateLayout->addWidget(endDateLabel);
    endDateLayout->addWidget(m_endDateFilter);

    QPushButton *applyFilterBtn = new QPushButton("Применить фильтр", filtersWidget);
    applyFilterBtn->setFixedSize(140, 38);
    applyFilterBtn->setCursor(Qt::PointingHandCursor);
    applyFilterBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2d9cff,
                stop:1 #1a7cd9);
            color: white;
            border-radius: 8px;
            font-weight: 600;
            border: none;
            font-size: 13px;
            margin-top: 10px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #268ce6,
                stop:1 #1565c0);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #1a7cd9,
                stop:1 #0d47a1);
        }
    )");

    QPushButton *addOperationBtn = new QPushButton("+ Добавить операцию", filtersWidget);
    addOperationBtn->setFixedSize(180, 40);
    addOperationBtn->setCursor(Qt::PointingHandCursor);
    addOperationBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #27ae60,
                stop:1 #219653);
            color: white;
            border-radius: 8px;
            font-weight: 600;
            border: none;
            font-size: 14px;
            margin-top: 8px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #219653,
                stop:1 #1e8449);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #1e8449,
                stop:1 #196f3d);
        }
    )");

    filtersLayout->addWidget(filterLabel);
    filtersLayout->addWidget(startDateWidget);
    filtersLayout->addWidget(endDateWidget);
    filtersLayout->addWidget(applyFilterBtn);
    filtersLayout->addStretch();
    filtersLayout->addWidget(addOperationBtn);

    cardLayout->addWidget(filtersWidget);

    // Operations List
    QScrollArea *scrollArea = new QScrollArea(card);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(R"(
        QScrollArea {
            border: none;
            background: transparent;
        }
        QScrollBar:vertical {
            border: none;
            background: #f8f9fa;
            width: 12px;
            margin: 0px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background: #c0c0c0;
            border-radius: 6px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background: #a0a0a0;
        }
        QScrollBar::handle:vertical:pressed {
            background: #808080;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            border: none;
            background: none;
            height: 0px;
        }
        QScrollBar:left-arrow:vertical, QScrollBar::right-arrow:vertical {
            border: none;
            background: none;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }
    )");

    m_operationsContainer = new QWidget();
    m_operationsLayout = new QVBoxLayout(m_operationsContainer);
    m_operationsLayout->setSpacing(10);
    m_operationsLayout->setContentsMargins(2, 2, 2, 2);
    m_operationsLayout->setAlignment(Qt::AlignTop);

    scrollArea->setWidget(m_operationsContainer);
    cardLayout->addWidget(scrollArea);

    root->addWidget(card, 0, Qt::AlignCenter);

    // Connections
    connect(backBtn, &QPushButton::clicked, this, &FinancesWindow::onBackClicked);
    connect(closeBtn, &QPushButton::clicked, this, &FinancesWindow::onCloseClicked);
    connect(addOperationBtn, &QPushButton::clicked, this, &FinancesWindow::onAddOperationClicked);
    connect(applyFilterBtn, &QPushButton::clicked, this, &FinancesWindow::onFilterChanged);

    // Стилизация календаря
    QCalendarWidget *calendar = m_startDateFilter->calendarWidget();
    calendar->setStyleSheet(R"(
        QCalendarWidget {
            background: white;
            border: 1px solid #ddd;
            border-radius: 8px;
        }
        QCalendarWidget QToolButton {
            color: #333;
            font-weight: bold;
            background: #f8f9fa;
            border: none;
            border-radius: 4px;
            padding: 5px;
        }
        QCalendarWidget QToolButton:hover {
            background: #e9ecef;
        }
        QCalendarWidget QMenu {
            background: white;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        QCalendarWidget QSpinBox {
            background: white;
            border: 1px solid #ddd;
            border-radius: 4px;
            padding: 2px;
        }
        QCalendarWidget QWidget#qt_calendar_navigationbar {
            background: #f8f9fa;
            border-bottom: 1px solid #ddd;
        }
        QCalendarWidget QAbstractItemView:enabled {
            color: #333;
            background: white;
            selection-background-color: #2d9cff;
            selection-color: white;
            border: none;
        }
        QCalendarWidget QAbstractItemView:disabled {
            color: #ccc;
        }
    )");

    setLayout(root);
}

void FinancesWindow::refreshFinanceData() {
    refreshOperationsList();
    refreshStats();
}

void FinancesWindow::refreshOperationsList() {
    // Clear existing operations
    QLayoutItem* item;
    while ((item = m_operationsLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->hide();
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Get operations from database
    QList<QMap<QString, QVariant>> operations = Database::instance().getFinancialOperations(
        m_startDateFilter->date(),
        m_endDateFilter->date()
    );

    for (const auto &operation : operations) {
        int id = operation["id"].toInt();
        double amount = operation["amount"].toDouble();
        QString category = operation["category"].toString();
        QString description = operation["description"].toString();
        QDate date = operation["operation_date"].toDate();
        QString createdByName = operation["created_by_name"].toString();

        // Create operation card
        QFrame *card = new QFrame(m_operationsContainer);
        card->setFixedHeight(70);
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

        // Amount with color
        QLabel *amountLabel = new QLabel(card);
        QString amountText = formatCurrency(amount);
        if (amount > 0) {
            amountLabel->setText("+" + amountText);
            amountLabel->setStyleSheet("color: #27ae60; font-weight: bold; font-size: 16px; min-width: 120px;");
        } else {
            amountLabel->setText(amountText);
            amountLabel->setStyleSheet("color: #c0392b; font-weight: bold; font-size: 16px; min-width: 120px;");
        }

        // Category with colored badge
        QLabel *categoryLabel = new QLabel(category, card);
        categoryLabel->setStyleSheet(QString(
            "QLabel {"
            "    color: %1;"
            "    font-size: 12px;"
            "    background: rgba(%2,0.1);"
            "    padding: 6px 12px;"
            "    border-radius: 10px;"
            "    font-weight: 600;"
            "    min-width: 80px;"
            "    border: 1px solid rgba(%2,0.2);"
            "}"
        ).arg(getCategoryColor(category)).arg(getCategoryColor(category).mid(1)));
        categoryLabel->setAlignment(Qt::AlignCenter);

        // Description
        QLabel *descLabel = new QLabel(description, card);
        descLabel->setStyleSheet("color: #333; font-size: 14px; min-width: 200px;");

        // Date
        QLabel *dateLabel = new QLabel(date.toString("dd.MM.yyyy"), card);
        dateLabel->setStyleSheet("color: #666; font-size: 13px; min-width: 100px;");

        // Created by
        QLabel *authorLabel = new QLabel(createdByName, card);
        authorLabel->setStyleSheet("color: #666; font-size: 13px; min-width: 150px;");

        // Delete button
        QPushButton *deleteBtn = new QPushButton("Удалить", card);
        deleteBtn->setFixedSize(80, 30);
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setProperty("operationId", id);
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

        cardLayout->addWidget(amountLabel);
        cardLayout->addWidget(categoryLabel);
        cardLayout->addWidget(descLabel);
        cardLayout->addWidget(dateLabel);
        cardLayout->addWidget(authorLabel);
        cardLayout->addStretch();
        cardLayout->addWidget(deleteBtn);

        m_operationsLayout->addWidget(card);

        // Connect delete button
        connect(deleteBtn, &QPushButton::clicked, this, &FinancesWindow::onDeleteOperationClicked);
    }

    // If no operations
    if (operations.isEmpty()) {
        QLabel *noDataLabel = new QLabel("Нет финансовых операций за выбранный период", m_operationsContainer);
        noDataLabel->setStyleSheet("color: #999; font-size: 14px; text-align: center; padding: 40px;");
        noDataLabel->setAlignment(Qt::AlignCenter);
        m_operationsLayout->addWidget(noDataLabel);
    }
}

void FinancesWindow::refreshStats() {
    double income = Database::instance().getTotalIncome(m_startDateFilter->date(), m_endDateFilter->date());
    double expenses = Database::instance().getTotalExpenses(m_startDateFilter->date(), m_endDateFilter->date());
    double profit = income + expenses;

    m_totalIncomeLabel->setText("+" + formatCurrency(income));
    m_totalExpensesLabel->setText("-" + formatCurrency(abs(expenses)));
    m_balanceLabel->setText(formatCurrency(profit));

    // Update profit color based on value
    if (profit > 0) {
        m_balanceLabel->setStyleSheet("color: #27ae60; font-size: 22px; font-weight: bold;");
    } else if (profit < 0) {
        m_balanceLabel->setStyleSheet("color: #c0392b; font-size: 22px; font-weight: bold;");
    } else {
        m_balanceLabel->setStyleSheet("color: #2980b9; font-size: 22px; font-weight: bold;");
    }
}

void FinancesWindow::showAddOperationDialog() {
    QDialog dialog(this);
    dialog.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dialog.setAttribute(Qt::WA_TranslucentBackground);
    dialog.setFixedSize(450, 450);

    QFrame *dialogCard = new QFrame(&dialog);
    dialogCard->setObjectName("dialogCard");
    dialogCard->setStyleSheet(R"(
        QFrame#dialogCard {
            background: white;
            border-radius: 12px;
            border: 1px solid #e0e0e0;
        }
    )");
    dialogCard->setFixedSize(450, 450);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(dialogCard);
    shadow->setBlurRadius(25);
    shadow->setOffset(0, 5);
    shadow->setColor(QColor(0,0,0,80));
    dialogCard->setGraphicsEffect(shadow);

    QVBoxLayout *dialogLayout = new QVBoxLayout(&dialog);
    dialogLayout->setContentsMargins(0,0,0,0);
    dialogLayout->addWidget(dialogCard);

    QVBoxLayout *cardLayout = new QVBoxLayout(dialogCard);
    cardLayout->setContentsMargins(30, 25, 30, 25);
    cardLayout->setSpacing(20);

    // Title
    QLabel *titleLabel = new QLabel("Добавить финансовую операцию", dialogCard);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2d9cff;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // Amount
    QLabel *amountLabel = new QLabel("Сумма:", dialogCard);
    amountLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px;");

    QLineEdit *amountInput = new QLineEdit(dialogCard);
    amountInput->setPlaceholderText("0.00");
    amountInput->setFixedHeight(40);
    amountInput->setStyleSheet(R"(
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

    // Category
    QLabel *categoryLabel = new QLabel("Категория:", dialogCard);
    categoryLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px;");

    QComboBox *categoryComboBox = new QComboBox(dialogCard);
    categoryComboBox->addItem("Услуги", "Услуги");
    categoryComboBox->addItem("Продукция", "Продукция");
    categoryComboBox->addItem("Зарплата", "Зарплата");
    categoryComboBox->addItem("Закупки", "Закупки");
    categoryComboBox->addItem("Аренда", "Аренда");
    categoryComboBox->addItem("Прочее", "Прочее");
    categoryComboBox->setFixedHeight(40);
    categoryComboBox->setStyleSheet(R"(
        QComboBox {
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 0 12px;
            font-size: 14px;
        }
        QComboBox:hover {
            border: 1px solid #2d9cff;
        }
        QComboBox QAbstractItemView {
            border: 1px solid #ddd;
            border-radius: 8px;
            background: white;
            selection-background-color: #2d9cff;
            selection-color: white;
            outline: none;
        }
    )");

    // Description
    QLabel *descLabel = new QLabel("Описание:", dialogCard);
    descLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px;");

    QLineEdit *descInput = new QLineEdit(dialogCard);
    descInput->setPlaceholderText("Описание операции");
    descInput->setFixedHeight(40);
    descInput->setStyleSheet(amountInput->styleSheet());

    // Date
    QLabel *dateLabel = new QLabel("Дата:", dialogCard);
    dateLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px;");

    QDateEdit *dateInput = new QDateEdit(dialogCard);
    dateInput->setDate(QDate::currentDate());
    dateInput->setCalendarPopup(true);
    dateInput->setFixedHeight(40);
    dateInput->setStyleSheet(amountInput->styleSheet());

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(amountLabel);
    cardLayout->addWidget(amountInput);
    cardLayout->addWidget(categoryLabel);
    cardLayout->addWidget(categoryComboBox);
    cardLayout->addWidget(descLabel);
    cardLayout->addWidget(descInput);
    cardLayout->addWidget(dateLabel);
    cardLayout->addWidget(dateInput);
    cardLayout->addStretch();

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    QPushButton *cancelBtn = new QPushButton("Отмена", dialogCard);
    cancelBtn->setFixedHeight(40);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton {
            background: rgba(0,0,0,0.05);
            color: #666;
            border: 1px solid rgba(0,0,0,0.1);
            border-radius: 8px;
            font-weight: 600;
            font-size: 14px;
        }
        QPushButton:hover {
            background: rgba(0,0,0,0.08);
        }
    )");

    QPushButton *confirmBtn = new QPushButton("Добавить операцию", dialogCard);
    confirmBtn->setFixedHeight(40);
    confirmBtn->setCursor(Qt::PointingHandCursor);
    confirmBtn->setStyleSheet(R"(
        QPushButton {
            background: #2d9cff;
            color: white;
            border-radius: 8px;
            font-weight: 600;
            border: none;
            font-size: 14px;
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
        bool ok;
        double amount = amountInput->text().toDouble(&ok);

        if (!ok || amount == 0) {
            QMessageBox::warning(&dialog, "Ошибка", "Введите корректную сумму!");
            return;
        }

        if (descInput->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Ошибка", "Введите описание операции!");
            return;
        }

        QString category = categoryComboBox->currentData().toString();
        QString description = descInput->text().trimmed();
        QDate date = dateInput->date();

        // For demo, use user ID 1 (owner)
        if (Database::instance().addFinancialOperation(amount, category, description, date, 1)) {
            refreshFinanceData();
            dialog.accept();
        } else {
            QMessageBox::warning(&dialog, "Ошибка", "Не удалось добавить операцию!");
        }
    });

    // Стилизация календаря в диалоге
    QCalendarWidget *calendar = dateInput->calendarWidget();
    calendar->setStyleSheet(R"(
        QCalendarWidget {
            background: white;
            border: 1px solid #ddd;
            border-radius: 8px;
        }
        QCalendarWidget QToolButton {
            color: #333;
            font-weight: bold;
            background: #f8f9fa;
            border: none;
            border-radius: 4px;
            padding: 5px;
        }
        QCalendarWidget QToolButton:hover {
            background: #e9ecef;
        }
        QCalendarWidget QMenu {
            background: white;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        QCalendarWidget QSpinBox {
            background: white;
            border: 1px solid #ddd;
            border-radius: 4px;
            padding: 2px;
        }
        QCalendarWidget QWidget#qt_calendar_navigationbar {
            background: #f8f9fa;
            border-bottom: 1px solid #ddd;
        }
        QCalendarWidget QAbstractItemView:enabled {
            color: #333;
            background: white;
            selection-background-color: #2d9cff;
            selection-color: white;
            border: none;
        }
        QCalendarWidget QAbstractItemView:disabled {
            color: #ccc;
        }
    )");

    dialog.exec();
}

QString FinancesWindow::getCategoryColor(const QString &category) {
    if (category == "Услуги") return "#3498db";
    if (category == "Продукция") return "#9b59b6";
    if (category == "Зарплата") return "#e74c3c";
    if (category == "Закупки") return "#e67e22";
    if (category == "Аренда") return "#f1c40f";
    if (category == "Прочее") return "#95a5a6";
    return "#2d9cff";
}

QString FinancesWindow::formatCurrency(double amount) {
    return QString("%1 ₽").arg(amount, 0, 'f', 2);
}

void FinancesWindow::onBackClicked() {
    emit backClicked();
}

void FinancesWindow::onCloseClicked() {
    close();
}

void FinancesWindow::onAddOperationClicked() {
    showAddOperationDialog();
}

void FinancesWindow::onFilterChanged() {
    refreshFinanceData();
}

void FinancesWindow::onDeleteOperationClicked() {
    QPushButton *deleteBtn = qobject_cast<QPushButton*>(sender());
    if (!deleteBtn) return;

    int operationId = deleteBtn->property("operationId").toInt();

    // Увеличенное стилизованное окно подтверждения
    QDialog confirmDialog(this);
    confirmDialog.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    confirmDialog.setAttribute(Qt::WA_TranslucentBackground);
    confirmDialog.setFixedSize(500, 280); // Увеличил размер

    QFrame *dialogCard = new QFrame(&confirmDialog);
    dialogCard->setObjectName("dialogCard");
    dialogCard->setStyleSheet(R"(
        QFrame#dialogCard {
            background: white;
            border-radius: 15px;
            border: 1px solid #e0e0e0;
        }
    )");
    dialogCard->setFixedSize(500, 280); // Увеличил размер

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(dialogCard);
    shadow->setBlurRadius(30);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0,0,0,100));
    dialogCard->setGraphicsEffect(shadow);

    QVBoxLayout *dialogLayout = new QVBoxLayout(&confirmDialog);
    dialogLayout->setContentsMargins(0,0,0,0);
    dialogLayout->addWidget(dialogCard);

    QVBoxLayout *cardLayout = new QVBoxLayout(dialogCard);
    cardLayout->setContentsMargins(35, 30, 35, 30); // Увеличил отступы
    cardLayout->setSpacing(25); // Увеличил расстояние между элементами

    // Title - увеличен
    QLabel *titleLabel = new QLabel("Подтверждение удаления", dialogCard);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #c0392b; margin-bottom: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // Message - увеличен
    QLabel *messageLabel = new QLabel("Вы уверены, что хотите удалить эту финансовую операцию?\n\nЭто действие нельзя отменить.", dialogCard);
    messageLabel->setStyleSheet("color: #333; font-size: 16px; text-align: center; line-height: 1.4;");
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setWordWrap(true);

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(messageLabel);
    cardLayout->addStretch();

    // Buttons - увеличены
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(20); // Увеличил расстояние между кнопками

    QPushButton *noBtn = new QPushButton("Отмена", dialogCard);
    noBtn->setFixedHeight(50); // Увеличил высоту кнопки
    noBtn->setCursor(Qt::PointingHandCursor);
    noBtn->setStyleSheet(R"(
        QPushButton {
            background: #f8f9fa;
            color: #666;
            border: 2px solid #dee2e6;
            border-radius: 10px;
            font-weight: 600;
            font-size: 16px;
        }
        QPushButton:hover {
            background: #e9ecef;
            border: 2px solid #adb5bd;
        }
        QPushButton:pressed {
            background: #dee2e6;
        }
    )");

    QPushButton *yesBtn = new QPushButton("Удалить", dialogCard);
    yesBtn->setFixedHeight(50); // Увеличил высоту кнопки
    yesBtn->setCursor(Qt::PointingHandCursor);
    yesBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #e74c3c,
                stop:1 #c0392b);
            color: white;
            border-radius: 10px;
            font-weight: 600;
            border: none;
            font-size: 16px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #c0392b,
                stop:1 #a93226);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #a93226,
                stop:1 #922b21);
        }
    )");

    buttonLayout->addWidget(noBtn);
    buttonLayout->addWidget(yesBtn);
    cardLayout->addLayout(buttonLayout);

    // Connections
    connect(noBtn, &QPushButton::clicked, &confirmDialog, &QDialog::reject);
    connect(yesBtn, &QPushButton::clicked, [&]() {
        if (Database::instance().deleteFinancialOperation(operationId)) {
            refreshFinanceData();
            confirmDialog.accept();

            // Показать уведомление об успешном удалении
            QMessageBox::information(this, "Успех", "Финансовая операция успешно удалена!");
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось удалить операцию!");
        }
    });

    confirmDialog.exec();
}

void FinancesWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void FinancesWindow::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}
