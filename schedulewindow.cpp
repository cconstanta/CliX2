#include "schedulewindow.h"
#include "database.h"
#include <QtWidgets>
#include <QDebug>
#include <QHeaderView>
#include <QSqlQuery>
#include <QTime>
#include <QMap>
#include <QVector>
#include <QTableWidgetItem>
#include <QScrollBar>

ScheduleWindow::ScheduleWindow(QWidget *parent) : QWidget(parent),
    m_tableDragging(false)
{
    setupUI();
    setupTimeSlots();
    refreshSchedule();
}

ScheduleWindow::~ScheduleWindow() {
}

void ScheduleWindow::setupTimeSlots() {
    m_timeSlots.clear();
    // Рабочий день с 9:00 до 21:00 с интервалом 30 минут
    QTime time(9, 0);
    while (time <= QTime(21, 0)) {
        m_timeSlots.append(time);
        time = time.addSecs(30 * 60); // +30 минут
    }
}

void ScheduleWindow::setupUI() {
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

    QLabel *title = new QLabel("График работы", topBar);
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

    // Filters section
    QWidget *filtersWidget = new QWidget(card);
    filtersWidget->setFixedHeight(60);
    QHBoxLayout *filtersLayout = new QHBoxLayout(filtersWidget);
    filtersLayout->setContentsMargins(0, 10, 0, 10);
    filtersLayout->setSpacing(15);

    QLabel *dateLabel = new QLabel("Дата:", filtersWidget);
    dateLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px;");

    m_dateFilter = new QDateEdit(filtersWidget);
    m_dateFilter->setDate(QDate::currentDate());
    m_dateFilter->setCalendarPopup(true);
    m_dateFilter->setFixedSize(140, 35);
    m_dateFilter->setStyleSheet(R"(
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
    )");

    QPushButton *addAppointmentBtn = new QPushButton("+ Новая запись", filtersWidget);
    addAppointmentBtn->setFixedSize(140, 38);
    addAppointmentBtn->setCursor(Qt::PointingHandCursor);
    addAppointmentBtn->setStyleSheet(R"(
        QPushButton {
            background: #2d9cff;
            color: white;
            border-radius: 8px;
            font-weight: 600;
            border: none;
            font-size: 13px;
        }
        QPushButton:hover {
            background: #268ce6;
        }
    )");

    filtersLayout->addWidget(dateLabel);
    filtersLayout->addWidget(m_dateFilter);
    filtersLayout->addStretch();
    filtersLayout->addWidget(addAppointmentBtn);

    cardLayout->addWidget(filtersWidget);

    // Schedule Table Container with custom scrollbars
    QWidget *tableContainer = new QWidget(card);
    QVBoxLayout *tableContainerLayout = new QVBoxLayout(tableContainer);
    tableContainerLayout->setContentsMargins(0, 0, 0, 0);
    tableContainerLayout->setSpacing(0);

    // Schedule Table
    m_scheduleTable = new QTableWidget(tableContainer);
    m_scheduleTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_scheduleTable->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_scheduleTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_scheduleTable->setFocusPolicy(Qt::NoFocus);

    // Включаем возможность перетаскивания для прокрутки
    m_scheduleTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_scheduleTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // Стилизация таблицы
    m_scheduleTable->setStyleSheet(R"(
        QTableWidget {
            background: white;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            gridline-color: #f0f0f0;
            outline: none;
        }
        QTableWidget::item {
            border: none;
            padding: 2px;
            border-bottom: 1px solid #f0f0f0;
            border-right: 1px solid #f0f0f0;
        }
        QTableWidget::item:selected {
            background: rgba(45, 156, 255, 0.1);
        }
        QHeaderView::section {
            background: #f8f9fa;
            padding: 12px 8px;
            border: none;
            border-right: 1px solid #e0e0e0;
            border-bottom: 1px solid #e0e0e0;
            font-weight: bold;
            color: #333;
        }
        QTableCornerButton::section {
            background: #f8f9fa;
            border: none;
            border-bottom: 1px solid #e0e0e0;
            border-right: 1px solid #e0e0e0;
        }
    )");

    // Кастомные скроллбары в стиле приложения
    m_scheduleTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scheduleTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Стилизация вертикального скроллбара
    m_scheduleTable->verticalScrollBar()->setStyleSheet(R"(
        QScrollBar:vertical {
            background: #f8f9fa;
            width: 14px;
            margin: 0px;
            border-radius: 7px;
        }
        QScrollBar::handle:vertical {
            background: #c0c0c0;
            border-radius: 7px;
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

    // Стилизация горизонтального скроллбара
    m_scheduleTable->horizontalScrollBar()->setStyleSheet(R"(
        QScrollBar:horizontal {
            background: #f8f9fa;
            height: 14px;
            margin: 0px;
            border-radius: 7px;
        }
        QScrollBar::handle:horizontal {
            background: #c0c0c0;
            border-radius: 7px;
            min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover {
            background: #a0a0a0;
        }
        QScrollBar::handle:horizontal:pressed {
            background: #808080;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            border: none;
            background: none;
            width: 0px;
        }
        QScrollBar:left-arrow:horizontal, QScrollBar::right-arrow:horizontal {
            border: none;
            background: none;
        }
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
            background: none;
        }
    )");

    tableContainerLayout->addWidget(m_scheduleTable);
    cardLayout->addWidget(tableContainer);

    root->addWidget(card, 0, Qt::AlignCenter);

    // Connections
    connect(backBtn, &QPushButton::clicked, this, &ScheduleWindow::onBackClicked);
    connect(closeBtn, &QPushButton::clicked, this, &ScheduleWindow::onCloseClicked);
    connect(addAppointmentBtn, &QPushButton::clicked, this, &ScheduleWindow::onAddAppointmentClicked);
    connect(m_dateFilter, &QDateEdit::dateChanged, this, &ScheduleWindow::onDateChanged);
    connect(m_scheduleTable, &QTableWidget::cellClicked, this, &ScheduleWindow::onCellClicked);
    connect(m_scheduleTable, &QTableWidget::cellDoubleClicked, this, &ScheduleWindow::onCellDoubleClicked);

    setLayout(root);
}

void ScheduleWindow::refreshSchedule() {
    refreshScheduleTable();
}

void ScheduleWindow::setupAppointmentWidget(int row, int column, const QString &clientName,
                                          const QString &serviceName, double price, const QString &status) {
    // Создаем виджет для красивого отображения записи
    QWidget *widget = new QWidget();
    widget->setObjectName("appointmentWidget");

    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(3);

    // Клиент (жирный, основной текст)
    QLabel *clientLabel = new QLabel(clientName);
    clientLabel->setWordWrap(true);
    clientLabel->setAlignment(Qt::AlignCenter);

    // Услуга (обычный текст)
    QLabel *serviceLabel = new QLabel(serviceName);
    serviceLabel->setWordWrap(true);
    serviceLabel->setAlignment(Qt::AlignCenter);

    // Цена (зеленый, жирный)
    QLabel *priceLabel = new QLabel(QString("%1 ₽").arg(price));
    priceLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(clientLabel);
    layout->addWidget(serviceLabel);
    layout->addWidget(priceLabel);

    // Устанавливаем цвет фона в зависимости от статуса
    QString backgroundColor, borderColor, textColor;

    // ИСПРАВЛЕНИЕ: только три статуса с четким разделением
    if (status == "completed") {
        backgroundColor = "#d4edda";  // светло-зеленый
        borderColor = "#27ae60";      // зеленый
        textColor = "#155724";        // темно-зеленый
    } else if (status == "cancelled") {
        backgroundColor = "#f8d7da";  // светло-красный
        borderColor = "#e74c3c";      // красный
        textColor = "#721c24";        // темно-красный
    } else {
        // ВСЕ остальные статусы (включая scheduled и любые другие) - синий
        backgroundColor = "#d1ecf1";  // светло-голубой
        borderColor = "#2d9cff";      // синий
        textColor = "#0c5460";        // темно-синий
    }

    // Устанавливаем стили для текста
    clientLabel->setStyleSheet(QString("font-weight: bold; font-size: 10px; color: %1; margin: 0; padding: 0;").arg(textColor));
    serviceLabel->setStyleSheet(QString("font-size: 9px; color: %1; margin: 0; padding: 0;").arg(textColor));
    priceLabel->setStyleSheet(QString("font-size: 9px; color: %1; font-weight: bold; margin: 0; padding: 0;").arg(textColor));

    // Стиль для виджета
    widget->setStyleSheet(QString(
        "QWidget#appointmentWidget {"
        "    background: %1;"
        "    border: 2px solid %2;"
        "    border-radius: 8px;"
        "}"
        "QWidget#appointmentWidget:hover {"
        "    border: 3px solid %2;"
        "}"
    ).arg(backgroundColor).arg(borderColor));

    // Устанавливаем виджет в ячейку таблицы
    m_scheduleTable->setCellWidget(row, column, widget);
}

void ScheduleWindow::refreshScheduleTable() {
    clearScheduleTable();
    m_appointmentCells.clear();

    QDate selectedDate = m_dateFilter->date();

    // Получаем список активных мастеров
    m_mastersMap.clear();
    m_masterRows.clear();
    m_timeColumns.clear();

    QSqlQuery mastersQuery("SELECT id, first_name, last_name FROM users WHERE role = 'Master' AND is_active = TRUE ORDER BY last_name, first_name");

    int rowCount = 0;
    while (mastersQuery.next()) {
        int masterId = mastersQuery.value(0).toInt();
        QString masterName = mastersQuery.value(1).toString() + " " + mastersQuery.value(2).toString();
        m_mastersMap[masterId] = masterName;
        m_masterRows[masterId] = rowCount++;
    }

    // Настраиваем таблицу - УБИРАЕМ ПУСТУЮ КОЛОНКУ
    int actualRowCount = m_mastersMap.size();
    int colCount = m_timeSlots.size(); // Теперь только временные слоты, без пустой колонки

    m_scheduleTable->setRowCount(actualRowCount);
    m_scheduleTable->setColumnCount(colCount);

    // Устанавливаем заголовки строк (мастера)
    QStringList verticalHeaders;
    for (const QString &masterName : m_mastersMap.values()) {
        verticalHeaders << masterName;
    }
    m_scheduleTable->setVerticalHeaderLabels(verticalHeaders);

    // Устанавливаем заголовки колонок (время) - ТОЛЬКО ВРЕМЯ
    QStringList horizontalHeaders;
    for (int i = 0; i < m_timeSlots.size(); ++i) {
        horizontalHeaders << m_timeSlots[i].toString("hh:mm");
        m_timeColumns[i] = i; // Теперь индексы начинаются с 0
    }
    m_scheduleTable->setHorizontalHeaderLabels(horizontalHeaders);

    // Получаем записи на выбранную дату
    QSqlQuery appointmentsQuery;
    appointmentsQuery.prepare(
        "SELECT a.id, a.master_id, a.client_name, a.service_id, a.appointment_time, a.duration, "
        "s.name as service_name, s.price, a.status "
        "FROM appointments a "
        "JOIN services s ON a.service_id = s.id "
        "WHERE a.appointment_date = :date "
        "ORDER BY a.appointment_time"
    );
    appointmentsQuery.bindValue(":date", selectedDate.toString("yyyy-MM-dd"));

    if (!appointmentsQuery.exec()) {
        qDebug() << "Error getting appointments:" << appointmentsQuery.lastError().text();
        return;
    }

    // Обрабатываем каждую запись
    while (appointmentsQuery.next()) {
        int appointmentId = appointmentsQuery.value(0).toInt();
        int masterId = appointmentsQuery.value(1).toInt();
        QString clientName = appointmentsQuery.value(2).toString();
        QTime appointmentTime = appointmentsQuery.value(4).toTime();
        int duration = appointmentsQuery.value(5).toInt();
        QString serviceName = appointmentsQuery.value(6).toString();
        double price = appointmentsQuery.value(7).toDouble();
        QString status = appointmentsQuery.value(8).toString();

        // Находим начальную колонку времени
        int startCol = -1;
        for (int i = 0; i < m_timeSlots.size(); ++i) {
            if (m_timeSlots[i] == appointmentTime) {
                startCol = m_timeColumns[i];
                break;
            }
        }

        if (startCol == -1) continue;

        // Находим строку мастера
        int row = m_masterRows.value(masterId, -1);
        if (row == -1) continue;

        // Вычисляем количество занимаемых колонок (30-минутные интервалы)
        int spanCols = qMax(1, duration / 30);

        // Создаем виджет для записи
        setupAppointmentWidget(row, startCol, clientName, serviceName, price, status);

        // Сохраняем связь ячейки с ID записи
        m_appointmentCells[QString("%1_%2").arg(row).arg(startCol)] = appointmentId;

        // Объединяем ячейки для длительных услуг
        if (spanCols > 1) {
            m_scheduleTable->setSpan(row, startCol, 1, spanCols);

            // Сохраняем связь для всех объединенных ячеек
            for (int i = startCol + 1; i < startCol + spanCols; ++i) {
                if (i < m_scheduleTable->columnCount()) {
                    m_appointmentCells[QString("%1_%2").arg(row).arg(i)] = appointmentId;
                }
            }
        }
    }

    // Настраиваем размеры колонок и строк
    for (int row = 0; row < actualRowCount; ++row) {
        m_scheduleTable->setRowHeight(row, 70);
    }

    for (int col = 0; col < colCount; ++col) {
        m_scheduleTable->horizontalHeader()->setSectionResizeMode(col, QHeaderView::Fixed);
        m_scheduleTable->setColumnWidth(col, 100);
    }
}

bool ScheduleWindow::checkTimeConflict(int masterId, const QDate &date, const QTime &startTime, int duration, int excludeAppointmentId) {
    QSqlQuery query;
    query.prepare(
        "SELECT id, appointment_time, duration FROM appointments "
        "WHERE master_id = ? AND appointment_date = ? AND id != ?"
    );
    query.addBindValue(masterId);
    query.addBindValue(date.toString("yyyy-MM-dd"));
    query.addBindValue(excludeAppointmentId);

    if (!query.exec()) {
        qDebug() << "Error checking time conflict:" << query.lastError().text();
        return false;
    }

    QTime newEndTime = startTime.addSecs(duration * 60);

    while (query.next()) {
        int existingId = query.value(0).toInt();
        QTime existingStartTime = query.value(1).toTime();
        int existingDuration = query.value(2).toInt();
        QTime existingEndTime = existingStartTime.addSecs(existingDuration * 60);

        // Проверяем пересечение временных интервалов
        if ((startTime >= existingStartTime && startTime < existingEndTime) ||
            (newEndTime > existingStartTime && newEndTime <= existingEndTime) ||
            (startTime <= existingStartTime && newEndTime >= existingEndTime)) {
            return true; // Найдено пересечение
        }
    }

    return false; // Конфликтов нет
}

void ScheduleWindow::showStyledMessageBox(const QString &title, const QString &message, bool isError) {
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
    cardLayout->setSpacing(20);

    // Title
    QLabel *titleLabel = new QLabel(title, dialogCard);
    titleLabel->setStyleSheet(QString("font-size: 18px; font-weight: bold; color: %1;").arg(isError ? "#e74c3c" : "#2d9cff"));
    titleLabel->setAlignment(Qt::AlignCenter);

    // Message
    QLabel *messageLabel = new QLabel(message, dialogCard);
    messageLabel->setStyleSheet("font-size: 14px; color: #333; text-align: center;");
    messageLabel->setWordWrap(true);
    messageLabel->setAlignment(Qt::AlignCenter);

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(messageLabel);
    cardLayout->addStretch();

    // Button
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    QPushButton *okButton = new QPushButton("OK", dialogCard);
    okButton->setFixedHeight(40);
    okButton->setCursor(Qt::PointingHandCursor);
    okButton->setStyleSheet(R"(
        QPushButton {
            background: #2d9cff;
            color: white;
            border-radius: 8px;
            font-weight: 600;
            border: none;
            font-size: 14px;
            min-width: 100px;
        }
        QPushButton:hover {
            background: #268ce6;
        }
    )");

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addStretch();
    cardLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    dialog.exec();
}

void ScheduleWindow::clearScheduleTable() {
    // Сначала удаляем все виджеты
    for (int row = 0; row < m_scheduleTable->rowCount(); ++row) {
        for (int col = 0; col < m_scheduleTable->columnCount(); ++col) {
            QWidget *widget = m_scheduleTable->cellWidget(row, col);
            if (widget) {
                widget->deleteLater();
            }
        }
    }

    // Затем очищаем таблицу
    m_scheduleTable->clear();
    m_scheduleTable->setRowCount(0);
    m_scheduleTable->setColumnCount(0);
}

void ScheduleWindow::onCellClicked(int row, int column) {
    // Находим masterId по строке
    int masterId = -1;
    for (auto it = m_masterRows.begin(); it != m_masterRows.end(); ++it) {
        if (it.value() == row) {
            masterId = it.key();
            break;
        }
    }

    // Находим время по колонке
    QTime selectedTime;
    for (auto it = m_timeColumns.begin(); it != m_timeColumns.end(); ++it) {
        if (it.value() == column) {
            selectedTime = m_timeSlots[it.key()];
            break;
        }
    }

    QString cellKey = QString("%1_%2").arg(row).arg(column);
    if (m_appointmentCells.contains(cellKey)) {
        // Ячейка содержит запись
        qDebug() << "Clicked on appointment:" << m_appointmentCells[cellKey];
    } else if (masterId != -1 && selectedTime.isValid()) {
        // Пустая ячейка - создаем новую запись
        showAddAppointmentDialog(masterId, selectedTime);
    }
}

void ScheduleWindow::onCellDoubleClicked(int row, int column) {
    QString cellKey = QString("%1_%2").arg(row).arg(column);
    if (m_appointmentCells.contains(cellKey)) {
        // Двойной клик по записи - редактируем
        showEditAppointmentDialog(m_appointmentCells[cellKey]);
    }
}

void ScheduleWindow::showEditAppointmentDialog(int appointmentId) {
    // Сначала получаем данные о записи
    QSqlQuery query;
    query.prepare(
        "SELECT a.master_id, a.client_name, a.client_phone, a.service_id, "
        "a.appointment_date, a.appointment_time, a.duration, a.status "
        "FROM appointments a WHERE a.id = ?"
    );
    query.addBindValue(appointmentId);

    if (!query.exec() || !query.next()) {
        showStyledMessageBox("Ошибка", "Не удалось загрузить данные записи!");
        return;
    }

    int masterId = query.value(0).toInt();
    QString clientName = query.value(1).toString();
    QString clientPhone = query.value(2).toString();
    int serviceId = query.value(3).toInt();
    QDate appointmentDate = query.value(4).toDate();
    QTime appointmentTime = query.value(5).toTime();
    int duration = query.value(6).toInt();
    QString status = query.value(7).toString();

    // Показываем диалог редактирования с передачей ВСЕХ данных
    showAddAppointmentDialog(masterId, appointmentTime, appointmentId, clientName, clientPhone, serviceId, appointmentDate, status);
}

void ScheduleWindow::showAddAppointmentDialog(int masterId, const QTime &startTime, int appointmentId,
                                            const QString &clientName, const QString &clientPhone,
                                            int serviceId, const QDate &appointmentDate, const QString &status) {
    bool isEditMode = (appointmentId != -1);

    QDialog dialog(this);
    dialog.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    dialog.setAttribute(Qt::WA_TranslucentBackground);
    dialog.setFixedSize(500, isEditMode ? 650 : 600);

    QFrame *dialogCard = new QFrame(&dialog);
    dialogCard->setObjectName("dialogCard");
    dialogCard->setStyleSheet(R"(
        QFrame#dialogCard {
            background: white;
            border-radius: 12px;
            border: 1px solid #e0e0e0;
        }
    )");
    dialogCard->setFixedSize(500, isEditMode ? 650 : 600);

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
    QLabel *titleLabel = new QLabel(isEditMode ? "Редактировать запись" : "Новая запись", dialogCard);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2d9cff;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // Client name
    QLabel *nameLabel = new QLabel("Имя клиента:", dialogCard);
    nameLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px;");

    QLineEdit *nameInput = new QLineEdit(dialogCard);
    nameInput->setPlaceholderText("Введите имя клиента");
    nameInput->setFixedHeight(40);
    nameInput->setStyleSheet(R"(
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

    // Заполняем данные если это редактирование
    if (isEditMode && !clientName.isEmpty()) {
        nameInput->setText(clientName);
    }

    // Client phone
    QLabel *phoneLabel = new QLabel("Телефон:", dialogCard);
    phoneLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px;");

    QLineEdit *phoneInput = new QLineEdit(dialogCard);
    phoneInput->setPlaceholderText("+7 XXX XXX XX XX");
    phoneInput->setFixedHeight(40);
    phoneInput->setStyleSheet(nameInput->styleSheet());

    // Заполняем данные если это редактирование
    if (isEditMode && !clientPhone.isEmpty()) {
        phoneInput->setText(clientPhone);
    }

    // Master selection
    QLabel *masterLabel = new QLabel("Мастер:", dialogCard);
    masterLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px;");

    QComboBox *masterComboBox = new QComboBox(dialogCard);
    masterComboBox->setFixedHeight(40);
    masterComboBox->setStyleSheet(R"(
        QComboBox {
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 0 12px;
            font-size: 14px;
        }
    )");

    // Fill masters combo
    QSqlQuery mastersQuery("SELECT id, first_name, last_name FROM users WHERE role = 'Master' AND is_active = TRUE");
    int selectedMasterIndex = -1;
    int index = 0;
    while (mastersQuery.next()) {
        int id = mastersQuery.value(0).toInt();
        QString name = mastersQuery.value(1).toString() + " " + mastersQuery.value(2).toString();
        masterComboBox->addItem(name, id);

        if (id == masterId) {
            selectedMasterIndex = index;
        }
        index++;
    }

    if (selectedMasterIndex != -1) {
        masterComboBox->setCurrentIndex(selectedMasterIndex);
    }

    // Service selection
    QLabel *serviceLabel = new QLabel("Услуга:", dialogCard);
    serviceLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px;");

    QComboBox *serviceComboBox = new QComboBox(dialogCard);
    serviceComboBox->setFixedHeight(40);
    serviceComboBox->setStyleSheet(masterComboBox->styleSheet());

    // Fill services combo
    QSqlQuery servicesQuery("SELECT id, name, price, duration FROM services WHERE is_active = TRUE");
    int selectedServiceIndex = -1;
    index = 0;
    while (servicesQuery.next()) {
        int id = servicesQuery.value(0).toInt();
        QString name = servicesQuery.value(1).toString();
        double price = servicesQuery.value(2).toDouble();
        int duration = servicesQuery.value(3).toInt();
        serviceComboBox->addItem(QString("%1 (%2 ₽, %3 мин)").arg(name).arg(price).arg(duration), id);

        if (isEditMode && id == serviceId) {
            selectedServiceIndex = index;
        }
        index++;
    }

    if (isEditMode && selectedServiceIndex != -1) {
        serviceComboBox->setCurrentIndex(selectedServiceIndex);
    }

    // Date and time
    QLabel *dateLabel = new QLabel("Дата и время:", dialogCard);
    dateLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px;");

    QHBoxLayout *datetimeLayout = new QHBoxLayout();
    datetimeLayout->setSpacing(10);

    QDateEdit *dateInput = new QDateEdit(dialogCard);
    dateInput->setCalendarPopup(true);
    dateInput->setFixedHeight(40);
    dateInput->setStyleSheet(nameInput->styleSheet());

    // Устанавливаем дату: при редактировании - из записи, иначе - из фильтра
    if (isEditMode && appointmentDate.isValid()) {
        dateInput->setDate(appointmentDate);
    } else {
        dateInput->setDate(m_dateFilter->date());
    }

    QTimeEdit *timeInput = new QTimeEdit(dialogCard);
    timeInput->setFixedHeight(40);
    timeInput->setStyleSheet(nameInput->styleSheet());

    // Устанавливаем время: при редактировании - из записи, иначе - переданное или по умолчанию
    if (isEditMode && startTime.isValid()) {
        timeInput->setTime(startTime);
    } else {
        timeInput->setTime(startTime.isValid() ? startTime : QTime(9, 0));
    }

    datetimeLayout->addWidget(dateInput);
    datetimeLayout->addWidget(timeInput);

    // Status selection (только для редактирования)
    QComboBox *statusComboBox = nullptr;
    if (isEditMode) {
        QLabel *statusLabel = new QLabel("Статус:", dialogCard);
        statusLabel->setStyleSheet("font-weight: bold; color: #333; font-size: 14px;");

        statusComboBox = new QComboBox(dialogCard);
        statusComboBox->setFixedHeight(40);
        statusComboBox->setStyleSheet(masterComboBox->styleSheet());
        statusComboBox->addItem("Запланировано", "scheduled");
        statusComboBox->addItem("Выполнено", "completed");
        statusComboBox->addItem("Отменено", "cancelled");

        // Устанавливаем текущий статус
        if (!status.isEmpty()) {
            int statusIndex = -1;
            if (status == "scheduled") statusIndex = 0;
            else if (status == "completed") statusIndex = 1;
            else if (status == "cancelled") statusIndex = 2;

            if (statusIndex != -1) {
                statusComboBox->setCurrentIndex(statusIndex);
            }
        }

        cardLayout->addWidget(statusLabel);
        cardLayout->addWidget(statusComboBox);
    }

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(nameLabel);
    cardLayout->addWidget(nameInput);
    cardLayout->addWidget(phoneLabel);
    cardLayout->addWidget(phoneInput);
    cardLayout->addWidget(masterLabel);
    cardLayout->addWidget(masterComboBox);
    cardLayout->addWidget(serviceLabel);
    cardLayout->addWidget(serviceComboBox);
    cardLayout->addWidget(dateLabel);
    cardLayout->addLayout(datetimeLayout);
    cardLayout->addStretch();

    // ... остальная часть метода без изменений (кнопки и их обработчики)

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    // Кнопка удаления (только для редактирования)
    if (isEditMode) {
        QPushButton *deleteBtn = new QPushButton("Удалить", dialogCard);
        deleteBtn->setFixedHeight(40);
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setStyleSheet(R"(
            QPushButton {
                background: #e74c3c;
                color: white;
                border-radius: 8px;
                font-weight: 600;
                border: none;
                font-size: 14px;
            }
            QPushButton:hover {
                background: #c0392b;
            }
        )");

        connect(deleteBtn, &QPushButton::clicked, [&]() {
            // Стилизованное подтверждение удаления
            QDialog confirmDialog(this);
            confirmDialog.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
            confirmDialog.setAttribute(Qt::WA_TranslucentBackground);
            confirmDialog.setFixedSize(400, 200);

            QFrame *confirmCard = new QFrame(&confirmDialog);
            confirmCard->setObjectName("confirmCard");
            confirmCard->setStyleSheet(R"(
                QFrame#confirmCard {
                    background: white;
                    border-radius: 12px;
                    border: 1px solid #e0e0e0;
                }
            )");
            confirmCard->setFixedSize(400, 200);

            QGraphicsDropShadowEffect *confirmShadow = new QGraphicsDropShadowEffect(confirmCard);
            confirmShadow->setBlurRadius(25);
            confirmShadow->setOffset(0, 5);
            confirmShadow->setColor(QColor(0,0,0,80));
            confirmCard->setGraphicsEffect(confirmShadow);

            QVBoxLayout *confirmLayout = new QVBoxLayout(&confirmDialog);
            confirmLayout->setContentsMargins(0,0,0,0);
            confirmLayout->addWidget(confirmCard);

            QVBoxLayout *confirmCardLayout = new QVBoxLayout(confirmCard);
            confirmCardLayout->setContentsMargins(25, 20, 25, 20);
            confirmCardLayout->setSpacing(20);

            QLabel *confirmTitle = new QLabel("Подтверждение удаления", confirmCard);
            confirmTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #e74c3c;");
            confirmTitle->setAlignment(Qt::AlignCenter);

            QLabel *confirmMessage = new QLabel("Вы уверены, что хотите удалить эту запись?", confirmCard);
            confirmMessage->setStyleSheet("font-size: 14px; color: #333; text-align: center;");
            confirmMessage->setWordWrap(true);
            confirmMessage->setAlignment(Qt::AlignCenter);

            confirmCardLayout->addWidget(confirmTitle);
            confirmCardLayout->addWidget(confirmMessage);
            confirmCardLayout->addStretch();

            QHBoxLayout *confirmButtonLayout = new QHBoxLayout();
            confirmButtonLayout->setSpacing(15);

            QPushButton *noBtn = new QPushButton("Отмена", confirmCard);
            noBtn->setFixedHeight(40);
            noBtn->setCursor(Qt::PointingHandCursor);
            noBtn->setStyleSheet(R"(
                QPushButton {
                    background: rgba(0,0,0,0.05);
                    color: #666;
                    border: 1px solid rgba(0,0,0,0.1);
                    border-radius: 8px;
                    font-weight: 600;
                    font-size: 14px;
                    min-width: 100px;
                }
                QPushButton:hover {
                    background: rgba(0,0,0,0.08);
                }
            )");

            QPushButton *yesBtn = new QPushButton("Удалить", confirmCard);
            yesBtn->setFixedHeight(40);
            yesBtn->setCursor(Qt::PointingHandCursor);
            yesBtn->setStyleSheet(R"(
                QPushButton {
                    background: #e74c3c;
                    color: white;
                    border-radius: 8px;
                    font-weight: 600;
                    border: none;
                    font-size: 14px;
                    min-width: 100px;
                }
                QPushButton:hover {
                    background: #c0392b;
                }
            )");

            confirmButtonLayout->addStretch();
            confirmButtonLayout->addWidget(noBtn);
            confirmButtonLayout->addWidget(yesBtn);
            confirmButtonLayout->addStretch();
            confirmCardLayout->addLayout(confirmButtonLayout);

            connect(noBtn, &QPushButton::clicked, &confirmDialog, &QDialog::reject);
            connect(yesBtn, &QPushButton::clicked, [&]() {
                QSqlQuery deleteQuery;
                deleteQuery.prepare("DELETE FROM appointments WHERE id = ?");
                deleteQuery.addBindValue(appointmentId);

                if (deleteQuery.exec()) {
                    refreshSchedule();
                    confirmDialog.accept();
                    dialog.accept();
                    // УБИРАЕМ ОКНО "УСПЕХ" после удаления
                } else {
                    showStyledMessageBox("Ошибка", "Не удалось удалить запись!");
                }
            });

            confirmDialog.exec();
        });

        buttonLayout->addWidget(deleteBtn);
    }

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

    QPushButton *confirmBtn = new QPushButton(isEditMode ? "Сохранить" : "Создать запись", dialogCard);
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
        if (nameInput->text().trimmed().isEmpty()) {
            showStyledMessageBox("Ошибка", "Введите имя клиента!");
            return;
        }

        if (masterComboBox->currentData().isNull()) {
            showStyledMessageBox("Ошибка", "Выберите мастера!");
            return;
        }

        if (serviceComboBox->currentData().isNull()) {
            showStyledMessageBox("Ошибка", "Выберите услугу!");
            return;
        }

        // Get service duration for the appointment
        QSqlQuery durationQuery;
        durationQuery.prepare("SELECT duration FROM services WHERE id = ?");
        durationQuery.addBindValue(serviceComboBox->currentData().toInt());
        if (!durationQuery.exec() || !durationQuery.next()) {
            showStyledMessageBox("Ошибка", "Не удалось получить данные об услуге!");
            return;
        }
        int duration = durationQuery.value(0).toInt();

        // Проверяем конфликт времени
        int selectedMasterId = masterComboBox->currentData().toInt();
        QDate selectedDate = dateInput->date();
        QTime selectedTime = timeInput->time();

        if (checkTimeConflict(selectedMasterId, selectedDate, selectedTime, duration, appointmentId)) {
            showStyledMessageBox("Ошибка", "Выбранное время пересекается с существующей записью!\nПожалуйста, выберите другое время.");
            return;
        }

        if (isEditMode) {
            // Редактирование существующей записи
            QSqlQuery updateQuery;
            updateQuery.prepare(
                "UPDATE appointments SET master_id = ?, service_id = ?, client_name = ?, client_phone = ?, "
                "appointment_date = ?, appointment_time = ?, duration = ?, status = ? "
                "WHERE id = ?"
            );

            updateQuery.addBindValue(selectedMasterId);
            updateQuery.addBindValue(serviceComboBox->currentData().toInt());
            updateQuery.addBindValue(nameInput->text().trimmed());
            updateQuery.addBindValue(phoneInput->text().trimmed());
            updateQuery.addBindValue(selectedDate.toString("yyyy-MM-dd"));
            updateQuery.addBindValue(selectedTime.toString("HH:mm:ss"));
            updateQuery.addBindValue(duration);
            updateQuery.addBindValue(statusComboBox->currentData().toString());
            updateQuery.addBindValue(appointmentId);

            if (updateQuery.exec()) {
                refreshSchedule();
                dialog.accept();
                // УБИРАЕМ ОКНО "УСПЕХ" после редактирования
            } else {
                showStyledMessageBox("Ошибка", "Не удалось обновить запись!");
            }
        } else {
            // Создание новой записи
            QSqlQuery insertQuery;
            insertQuery.prepare(
                "INSERT INTO appointments (master_id, service_id, client_name, client_phone, "
                "appointment_date, appointment_time, duration, created_by) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
            );

            insertQuery.addBindValue(selectedMasterId);
            insertQuery.addBindValue(serviceComboBox->currentData().toInt());
            insertQuery.addBindValue(nameInput->text().trimmed());
            insertQuery.addBindValue(phoneInput->text().trimmed());
            insertQuery.addBindValue(selectedDate.toString("yyyy-MM-dd"));
            insertQuery.addBindValue(selectedTime.toString("HH:mm:ss"));
            insertQuery.addBindValue(duration);
            insertQuery.addBindValue(2); // admin user id

            if (insertQuery.exec()) {
                refreshSchedule();
                dialog.accept();
                // УБИРАЕМ ОКНО "УСПЕХ" после создания
            } else {
                showStyledMessageBox("Ошибка", "Не удалось создать запись!");
            }
        }
    });

    dialog.exec();
}

void ScheduleWindow::onBackClicked() {
    emit backClicked();
}

void ScheduleWindow::onCloseClicked() {
    close();
}

void ScheduleWindow::onAddAppointmentClicked() {
    showAddAppointmentDialog(-1, QTime());
}

void ScheduleWindow::onDateChanged() {
    refreshSchedule();
}

// Обработка нажатия мыши
void ScheduleWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        // Проверяем, был ли клик внутри таблицы
        QPoint tablePos = m_scheduleTable->mapFromParent(event->pos());
        if (m_scheduleTable->rect().contains(tablePos)) {
            m_tableDragging = true;
            m_tableDragStartPosition = event->globalPos();
            m_tableDragStartScroll = QPoint(
                m_scheduleTable->horizontalScrollBar()->value(),
                m_scheduleTable->verticalScrollBar()->value()
            );
            setCursor(Qt::ClosedHandCursor);
        } else {
            // Перетаскивание окна
            m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        }
        event->accept();
    }
}

// Обработка перемещения мыши
void ScheduleWindow::mouseMoveEvent(QMouseEvent *event) {
    if (m_tableDragging && (event->buttons() & Qt::LeftButton)) {
        // Перетаскивание таблицы
        QPoint delta = event->globalPos() - m_tableDragStartPosition;
        m_scheduleTable->horizontalScrollBar()->setValue(
            m_tableDragStartScroll.x() - delta.x()
        );
        m_scheduleTable->verticalScrollBar()->setValue(
            m_tableDragStartScroll.y() - delta.y()
        );
        event->accept();
    } else if (event->buttons() & Qt::LeftButton) {
        // Перетаскивание окна
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

// Обработка отпускания мыши
void ScheduleWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_tableDragging) {
        m_tableDragging = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    }
}

// Обработка колесика мыши
void ScheduleWindow::wheelEvent(QWheelEvent *event) {
    if (event->angleDelta().y() != 0) {
        // Вертикальная прокрутка
        m_scheduleTable->verticalScrollBar()->setValue(
            m_scheduleTable->verticalScrollBar()->value() - event->angleDelta().y() / 2
        );
    } else if (event->angleDelta().x() != 0) {
        // Горизонтальная прокрутка
        m_scheduleTable->horizontalScrollBar()->setValue(
            m_scheduleTable->horizontalScrollBar()->value() - event->angleDelta().x() / 2
        );
    }
    event->accept();
}
