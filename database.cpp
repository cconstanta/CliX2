#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>
#include <QDate>

Database::Database(QObject *parent) : QObject(parent) {
    qDebug() << "Database constructor called";
}

Database::~Database() {
    qDebug() << "Database destructor called";
    if (m_database.isOpen()) {
        m_database.close();
    }
}

QList<QMap<QString, QVariant>> Database::getMasterSchedule(int masterId, const QDate &date) {
    QList<QMap<QString, QVariant>> schedule;

    QSqlQuery query;
    query.prepare(
        "SELECT a.id, a.client_name, s.name as service_name, s.price, s.duration, "
        "a.appointment_time, a.status, a.notes "
        "FROM appointments a "
        "JOIN services s ON a.service_id = s.id "
        "WHERE a.master_id = ? AND a.appointment_date = ? "
        "ORDER BY a.appointment_time"
    );
    query.addBindValue(masterId);
    query.addBindValue(date.toString("yyyy-MM-dd"));

    if (!query.exec()) {
        qDebug() << "Error getting master schedule:" << query.lastError().text();
        return schedule;
    }

    while (query.next()) {
        QMap<QString, QVariant> appointment;
        appointment["id"] = query.value(0).toInt();
        appointment["client_name"] = query.value(1).toString();
        appointment["service_name"] = query.value(2).toString();
        appointment["price"] = query.value(3).toDouble();
        appointment["duration"] = query.value(4).toInt();
        appointment["appointment_time"] = query.value(5).toTime();
        appointment["status"] = query.value(6).toString();
        appointment["notes"] = query.value(7).toString();

        schedule.append(appointment);
    }

    qDebug() << "Loaded" << schedule.size() << "appointments for master" << masterId;
    return schedule;
}

bool Database::updateAppointmentStatus(int appointmentId, const QString &status) {
    QSqlQuery query;
    query.prepare("UPDATE appointments SET status = ?, completion_time = CASE WHEN ? = 'completed' THEN CURRENT_TIMESTAMP ELSE NULL END WHERE id = ?");
    query.addBindValue(status);
    query.addBindValue(status);
    query.addBindValue(appointmentId);

    if (!query.exec()) {
        qDebug() << "Error updating appointment status:" << query.lastError().text();
        return false;
    }

    qDebug() << "Appointment status updated. ID:" << appointmentId << "Status:" << status;
    return true;
}

QMap<QString, QVariant> Database::getMasterStatistics(int masterId, const QDate &startDate, const QDate &endDate) {
    QMap<QString, QVariant> stats;

    QSqlQuery query;
    query.prepare(
        "SELECT "
        "COUNT(*) as total_appointments, "
        "SUM(CASE WHEN status = 'completed' THEN 1 ELSE 0 END) as completed, "
        "SUM(CASE WHEN status = 'cancelled' THEN 1 ELSE 0 END) as cancelled, "
        "SUM(CASE WHEN status = 'completed' THEN s.price ELSE 0 END) as total_earnings "
        "FROM appointments a "
        "JOIN services s ON a.service_id = s.id "
        "WHERE a.master_id = ? AND a.appointment_date BETWEEN ? AND ?"
    );
    query.addBindValue(masterId);
    query.addBindValue(startDate.toString("yyyy-MM-dd"));
    query.addBindValue(endDate.toString("yyyy-MM-dd"));

    if (query.exec() && query.next()) {
        stats["total_appointments"] = query.value(0).toInt();
        stats["completed"] = query.value(1).toInt();
        stats["cancelled"] = query.value(2).toInt();
        stats["total_earnings"] = query.value(3).toDouble();
    } else {
        stats["total_appointments"] = 0;
        stats["completed"] = 0;
        stats["cancelled"] = 0;
        stats["total_earnings"] = 0.0;
    }

    return stats;
}

bool Database::initializeDatabase() {
    qDebug() << "Initializing database...";

    if (m_database.isOpen()) {
        qDebug() << "Database already open";
        return true;
    }

    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName("clix_database.db");

    if (!m_database.open()) {
        qDebug() << "Error: Failed to open database:" << m_database.lastError().text();
        return false;
    }

    qDebug() << "Database opened successfully";

    return updateDatabaseSchema();
}

bool Database::updateDatabaseSchema() {
    qDebug() << "Updating database schema...";

    QSqlQuery query;
    // Таблица услуг
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS services ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name VARCHAR(255) NOT NULL, "
        "price DECIMAL(10,2) NOT NULL, "
        "duration INTEGER NOT NULL, " // длительность в минутах
        "description TEXT, "
        "is_active BOOLEAN DEFAULT TRUE, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)"
    )) {
        qDebug() << "Error creating services table:" << query.lastError().text();
        return false;
    }

    // Таблица записей/графика
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS appointments ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "master_id INTEGER NOT NULL, "
        "service_id INTEGER NOT NULL, "
        "client_name VARCHAR(255) NOT NULL, "
        "client_phone VARCHAR(50), "
        "appointment_date DATE NOT NULL, "
        "appointment_time TIME NOT NULL, "
        "duration INTEGER NOT NULL, "
        "status VARCHAR(50) DEFAULT 'scheduled', " // scheduled, completed, cancelled
        "completed BOOLEAN DEFAULT FALSE, "
        "completion_time DATETIME, "
        "notes TEXT, "
        "created_by INTEGER NOT NULL, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (master_id) REFERENCES users(id), "
        "FOREIGN KEY (service_id) REFERENCES services(id), "
        "FOREIGN KEY (created_by) REFERENCES users(id))"
    )) {
        qDebug() << "Error creating appointments table:" << query.lastError().text();
        return false;
    }

    // Таблица пользователей
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "email VARCHAR(255) NOT NULL UNIQUE, "
        "password VARCHAR(255) NOT NULL, "
        "first_name VARCHAR(100) NOT NULL, "
        "last_name VARCHAR(100) NOT NULL, "
        "middle_name VARCHAR(100), "
        "role VARCHAR(50) NOT NULL, "
        "is_active BOOLEAN DEFAULT TRUE, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)"
    )) {
        qDebug() << "Error creating users table:" << query.lastError().text();
        return false;
    }

    // Таблица кодов приглашений
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS invite_codes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "code VARCHAR(50) NOT NULL UNIQUE, "
        "role VARCHAR(50) NOT NULL, "
        "used BOOLEAN DEFAULT FALSE, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)"
    )) {
        qDebug() << "Error creating invite_codes table:" << query.lastError().text();
        return false;
    }

    // Таблица финансовых операций
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS financial_operations ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "amount DECIMAL(10,2) NOT NULL, "
        "category VARCHAR(100) NOT NULL, "
        "description TEXT, "
        "operation_date DATE NOT NULL, "
        "created_by INTEGER NOT NULL, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (created_by) REFERENCES users(id))"
    )) {
        qDebug() << "Error creating financial_operations table:" << query.lastError().text();
        return false;
    }

    // Таблица настроек зарплаты
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS salary_settings ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "user_id INTEGER NOT NULL UNIQUE, "
        "base_salary DECIMAL(10,2) DEFAULT 0, "
        "percentage DECIMAL(5,2) DEFAULT 0, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "updated_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (user_id) REFERENCES users(id))"
    )) {
        qDebug() << "Error creating salary_settings table:" << query.lastError().text();
        return false;
    }

    // Таблица финансовых операций сотрудников
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS employee_financial_operations ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "user_id INTEGER NOT NULL, "
        "amount DECIMAL(10,2) NOT NULL, "
        "type VARCHAR(50) NOT NULL, " // bonus, advance, deduction, salary
        "description TEXT, "
        "operation_date DATE NOT NULL, "
        "created_by INTEGER NOT NULL, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (user_id) REFERENCES users(id), "
        "FOREIGN KEY (created_by) REFERENCES users(id))"
    )) {
        qDebug() << "Error creating employee_financial_operations table:" << query.lastError().text();
        return false;
    }

    qDebug() << "Database schema updated successfully";
    return true;
}

void Database::createTestData() {
    qDebug() << "Creating test data...";

    // Тестовые услуги
    QSqlQuery checkServices("SELECT COUNT(*) FROM services");
    if (checkServices.next() && checkServices.value(0).toInt() == 0) {
        QStringList services = {
            "Стрижка мужская", "Стрижка женская", "Окрашивание", "Укладка", "Маникюр"
        };
        QList<double> prices = {800.0, 1200.0, 2500.0, 800.0, 1500.0};
        QList<int> durations = {60, 90, 120, 45, 60};

        for (int i = 0; i < services.size(); ++i) {
            QSqlQuery insertService;
            insertService.prepare("INSERT INTO services (name, price, duration) VALUES (?, ?, ?)");
            insertService.addBindValue(services[i]);
            insertService.addBindValue(prices[i]);
            insertService.addBindValue(durations[i]);
            insertService.exec();
        }
        qDebug() << "Test services created";
    }

    // Создаем тестового владельца, если его нет
    if (!userExists("owner@company.com")) {
        createUser("owner@company.com", "owner123", "Иван", "Петров", "Сергеевич", "Owner");
    }

    // Создаем тестового администратора
    if (!userExists("admin@company.com")) {
        createUser("admin@company.com", "admin123", "Мария", "Иванова", "Александровна", "Admin");
    }

    // Создаем тестовых мастеров (6 мастеров вместо 1)
    QVector<QString> masterEmails = {
        "master1@company.com", "master2@company.com", "master3@company.com",
        "master4@company.com", "master5@company.com", "master6@company.com"
    };

    QVector<QString> masterFirstNames = {
        "Алексей", "Екатерина", "Дмитрий", "Ольга", "Сергей", "Анна"
    };

    QVector<QString> masterLastNames = {
        "Сидоров", "Кузнецова", "Попов", "Васильева", "Петров", "Смирнова"
    };

    QVector<QString> masterMiddleNames = {
        "Дмитриевич", "Андреевна", "Сергеевич", "Ивановна", "Александрович", "Владимировна"
    };

    for (int i = 0; i < masterEmails.size(); ++i) {
        if (!userExists(masterEmails[i])) {
            createUser(masterEmails[i], "master123", masterFirstNames[i],
                      masterLastNames[i], masterMiddleNames[i], "Master");
        }
    }

    // Создаем тестовые коды приглашений
    createInviteCode("COOWNER123", "CoOwner");
    createInviteCode("ADMIN123", "Admin");
    createInviteCode("MASTER123", "Master");

    // Создаем тестовые настройки зарплаты для мастеров
    QSqlQuery salaryQuery("SELECT COUNT(*) FROM salary_settings");
    if (salaryQuery.next() && salaryQuery.value(0).toInt() == 0) {
        // Настройки для мастеров
        QVector<double> masterPercentages = {25.0, 30.0, 28.0, 26.0, 32.0, 27.0};
        QVector<double> masterSalaries = {15000.0, 12000.0, 18000.0, 14000.0, 16000.0, 13000.0};

        // Получаем ID мастеров
        QSqlQuery mastersQuery("SELECT id FROM users WHERE role = 'Master' ORDER BY id");
        int index = 0;
        while (mastersQuery.next() && index < masterPercentages.size()) {
            int masterId = mastersQuery.value(0).toInt();
            updateSalarySettings(masterId, masterSalaries[index], masterPercentages[index]);
            index++;
        }

        // Настройки для администратора
        QSqlQuery adminQuery("SELECT id FROM users WHERE role = 'Admin' AND email = 'admin@company.com'");
        if (adminQuery.next()) {
            int adminId = adminQuery.value(0).toInt();
            updateSalarySettings(adminId, 35000.0, 0.0); // Админ только оклад
        }

        qDebug() << "Test salary settings created";
    }

    // Создаем тестовые записи на текущую дату для мастеров
    QSqlQuery checkAppointments("SELECT COUNT(*) FROM appointments WHERE appointment_date = DATE('now')");
    if (checkAppointments.next() && checkAppointments.value(0).toInt() == 0) {
        createTestAppointments();
    }

    qDebug() << "Test data created successfully";
}

void Database::createTestAppointments() {
    qDebug() << "Creating test appointments...";

    // Получаем ID мастеров и услуг
    QVector<int> masterIds;
    QSqlQuery mastersQuery("SELECT id FROM users WHERE role = 'Master' ORDER BY id");
    while (mastersQuery.next()) {
        masterIds.append(mastersQuery.value(0).toInt());
    }

    QVector<int> serviceIds;
    QSqlQuery servicesQuery("SELECT id FROM services ORDER BY id");
    while (servicesQuery.next()) {
        serviceIds.append(servicesQuery.value(0).toInt());
    }

    if (masterIds.isEmpty() || serviceIds.isEmpty()) {
        qDebug() << "No masters or services found for creating test appointments";
        return;
    }

    // Создаем записи на текущую дату
    QDate today = QDate::currentDate();
    QStringList clientNames = {
        "Иванов Алексей", "Петрова Мария", "Сидорова Анна",
        "Кузнецов Дмитрий", "Васильева Ольга", "Попов Сергей"
    };

    // Временные слоты с 9:00 до 18:00
    QVector<QTime> timeSlots;
    QTime time(9, 0);
    while (time <= QTime(18, 0)) {
        timeSlots.append(time);
        time = time.addSecs(60 * 60); // +1 час
    }

    int appointmentCount = 0;
    for (int i = 0; i < masterIds.size() && i < timeSlots.size(); ++i) {
        int masterId = masterIds[i];
        int serviceId = serviceIds[i % serviceIds.size()];
        QString clientName = clientNames[i % clientNames.size()];
        QTime appointmentTime = timeSlots[i];

        // Получаем длительность услуги
        int duration = 60; // значение по умолчанию
        QSqlQuery durationQuery;
        durationQuery.prepare("SELECT duration FROM services WHERE id = ?");
        durationQuery.addBindValue(serviceId);
        if (durationQuery.exec() && durationQuery.next()) {
            duration = durationQuery.value(0).toInt();
        }

        QSqlQuery insertAppointment;
        insertAppointment.prepare(
            "INSERT INTO appointments (master_id, service_id, client_name, client_phone, "
            "appointment_date, appointment_time, duration, status, created_by) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"
        );

        insertAppointment.addBindValue(masterId);
        insertAppointment.addBindValue(serviceId);
        insertAppointment.addBindValue(clientName);
        insertAppointment.addBindValue("+7 999 123 45 6" + QString::number(i));
        insertAppointment.addBindValue(today.toString("yyyy-MM-dd"));
        insertAppointment.addBindValue(appointmentTime.toString("HH:mm:ss"));
        insertAppointment.addBindValue(duration);
        insertAppointment.addBindValue("scheduled");
        insertAppointment.addBindValue(1); // создано владельцем

        if (insertAppointment.exec()) {
            appointmentCount++;
        } else {
            qDebug() << "Error creating appointment:" << insertAppointment.lastError().text();
        }
    }

    qDebug() << "Created" << appointmentCount << "test appointments for today";
}

bool Database::createUser(const QString &email, const QString &password, const QString &firstName,
                         const QString &lastName, const QString &middleName, const QString &role,
                         const QString &inviteCode) {
    QSqlQuery query;
    query.prepare(
        "INSERT INTO users (email, password, first_name, last_name, middle_name, role) "
        "VALUES (:email, :password, :first_name, :last_name, :middle_name, :role)"
    );

    query.bindValue(":email", email);
    query.bindValue(":password", password);
    query.bindValue(":first_name", firstName);
    query.bindValue(":last_name", lastName);
    query.bindValue(":middle_name", middleName);
    query.bindValue(":role", role);

    if (!query.exec()) {
        qDebug() << "Error creating user:" << query.lastError().text();
        return false;
    }

    // Помечаем код приглашения как использованный, если он был передан
    if (!inviteCode.isEmpty()) {
        QSqlQuery updateCodeQuery;
        updateCodeQuery.prepare("UPDATE invite_codes SET used = TRUE WHERE code = :code");
        updateCodeQuery.bindValue(":code", inviteCode);
        updateCodeQuery.exec();
    }

    qDebug() << "User created:" << email << "Role:" << role;
    return true;
}

bool Database::validateUser(const QString &email, const QString &password) {
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE email = :email AND is_active = TRUE");
    query.bindValue(":email", email);

    if (!query.exec()) {
        qDebug() << "Error validating user:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        QString storedPassword = query.value(0).toString();
        return storedPassword == password;
    }

    return false;
}

QString Database::getUserRole(const QString &email) {
    QSqlQuery query;
    query.prepare("SELECT role FROM users WHERE email = :email");
    query.bindValue(":email", email);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }

    return "";
}

QString Database::getUserName(const QString &email) {
    QSqlQuery query;
    query.prepare("SELECT first_name, last_name FROM users WHERE email = :email");
    query.bindValue(":email", email);

    if (query.exec() && query.next()) {
        QString firstName = query.value(0).toString();
        QString lastName = query.value(1).toString();
        return QString("%1 %2").arg(firstName).arg(lastName);
    }

    return "";
}

bool Database::userExists(const QString &email) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM users WHERE email = :email");
    query.bindValue(":email", email);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

QList<QMap<QString, QVariant>> Database::getAllEmployees() {
    QList<QMap<QString, QVariant>> employees;

    QSqlQuery query("SELECT id, first_name, last_name, middle_name, role, is_active FROM users ORDER BY role, last_name, first_name");

    while (query.next()) {
        QMap<QString, QVariant> employee;
        employee["id"] = query.value(0).toInt();
        employee["first_name"] = query.value(1).toString();
        employee["last_name"] = query.value(2).toString();
        employee["middle_name"] = query.value(3).toString();
        employee["role"] = query.value(4).toString();
        employee["is_active"] = query.value(5).toBool();

        employees.append(employee);
    }

    return employees;
}

bool Database::updateEmployeeRole(int userId, const QString &newRole) {
    QSqlQuery query;
    query.prepare("UPDATE users SET role = :role WHERE id = :id");
    query.bindValue(":role", newRole);
    query.bindValue(":id", userId);

    if (!query.exec()) {
        qDebug() << "Error updating employee role:" << query.lastError().text();
        return false;
    }

    qDebug() << "Employee role updated. User ID:" << userId << "New role:" << newRole;
    return true;
}

bool Database::deleteEmployee(int userId) {
    QSqlQuery query;
    query.prepare("DELETE FROM users WHERE id = :id");
    query.bindValue(":id", userId);

    if (!query.exec()) {
        qDebug() << "Error deleting employee:" << query.lastError().text();
        return false;
    }

    qDebug() << "Employee deleted. User ID:" << userId;
    return true;
}

bool Database::toggleEmployeeActive(int userId, bool active) {
    QSqlQuery query;
    query.prepare("UPDATE users SET is_active = :active WHERE id = :id");
    query.bindValue(":active", active);
    query.bindValue(":id", userId);

    if (!query.exec()) {
        qDebug() << "Error toggling employee active status:" << query.lastError().text();
        return false;
    }

    qDebug() << "Employee active status updated. User ID:" << userId << "Active:" << active;
    return true;
}

bool Database::createInviteCode(const QString &code, const QString &role) {
    QSqlQuery query;
    query.prepare("INSERT INTO invite_codes (code, role) VALUES (:code, :role)");
    query.bindValue(":code", code);
    query.bindValue(":role", role);

    if (!query.exec()) {
        qDebug() << "Error creating invite code:" << query.lastError().text();
        return false;
    }

    qDebug() << "Invite code created:" << code << "Role:" << role;
    return true;
}

bool Database::validateInviteCode(const QString &code, QString &role) {
    QSqlQuery query;
    query.prepare("SELECT role FROM invite_codes WHERE code = :code AND used = FALSE");
    query.bindValue(":code", code);

    if (!query.exec()) {
        qDebug() << "Error validating invite code:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        role = query.value(0).toString();
        return true;
    }

    return false;
}

QList<QMap<QString, QVariant>> Database::getFinancialOperations(const QDate &startDate, const QDate &endDate) {
    QList<QMap<QString, QVariant>> operations;

    QSqlQuery query;
    query.prepare(
        "SELECT fo.id, fo.amount, fo.category, fo.description, fo.operation_date, "
        "u.first_name || ' ' || u.last_name as created_by_name "
        "FROM financial_operations fo "
        "JOIN users u ON fo.created_by = u.id "
        "WHERE fo.operation_date BETWEEN :start_date AND :end_date "
        "ORDER BY fo.operation_date DESC, fo.id DESC"
    );
    query.bindValue(":start_date", startDate.toString("yyyy-MM-dd"));
    query.bindValue(":end_date", endDate.toString("yyyy-MM-dd"));

    if (!query.exec()) {
        qDebug() << "Error getting financial operations:" << query.lastError().text();
        return operations;
    }

    while (query.next()) {
        QMap<QString, QVariant> operation;
        operation["id"] = query.value(0).toInt();
        operation["amount"] = query.value(1).toDouble();
        operation["category"] = query.value(2).toString();
        operation["description"] = query.value(3).toString();
        operation["operation_date"] = query.value(4).toDate();
        operation["created_by_name"] = query.value(5).toString();

        operations.append(operation);
    }

    return operations;
}

bool Database::addFinancialOperation(double amount, const QString &category, const QString &description,
                                   const QDate &date, int createdBy) {
    QSqlQuery query;
    query.prepare(
        "INSERT INTO financial_operations (amount, category, description, operation_date, created_by) "
        "VALUES (:amount, :category, :description, :operation_date, :created_by)"
    );

    query.bindValue(":amount", amount);
    query.bindValue(":category", category);
    query.bindValue(":description", description);
    query.bindValue(":operation_date", date.toString("yyyy-MM-dd"));
    query.bindValue(":created_by", createdBy);

    if (!query.exec()) {
        qDebug() << "Error adding financial operation:" << query.lastError().text();
        return false;
    }

    qDebug() << "Financial operation added:" << description << "Amount:" << amount;
    return true;
}

bool Database::deleteFinancialOperation(int operationId) {
    QSqlQuery query;
    query.prepare("DELETE FROM financial_operations WHERE id = :id");
    query.bindValue(":id", operationId);

    if (!query.exec()) {
        qDebug() << "Error deleting financial operation:" << query.lastError().text();
        return false;
    }

    qDebug() << "Financial operation deleted. ID:" << operationId;
    return true;
}

double Database::getTotalIncome(const QDate &startDate, const QDate &endDate) {
    QSqlQuery query;
    query.prepare(
        "SELECT COALESCE(SUM(amount), 0) FROM financial_operations "
        "WHERE amount > 0 AND operation_date BETWEEN :start_date AND :end_date"
    );
    query.bindValue(":start_date", startDate.toString("yyyy-MM-dd"));
    query.bindValue(":end_date", endDate.toString("yyyy-MM-dd"));

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

double Database::getTotalExpenses(const QDate &startDate, const QDate &endDate) {
    QSqlQuery query;
    query.prepare(
        "SELECT COALESCE(SUM(amount), 0) FROM financial_operations "
        "WHERE amount < 0 AND operation_date BETWEEN :start_date AND :end_date"
    );
    query.bindValue(":start_date", startDate.toString("yyyy-MM-dd"));
    query.bindValue(":end_date", endDate.toString("yyyy-MM-dd"));

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

bool Database::updateSalarySettings(int userId, double baseSalary, double percentage) {
    QSqlQuery query;
    query.prepare(
        "INSERT OR REPLACE INTO salary_settings (user_id, base_salary, percentage, updated_at) "
        "VALUES (?, ?, ?, CURRENT_TIMESTAMP)"
    );

    query.addBindValue(userId);
    query.addBindValue(baseSalary);
    query.addBindValue(percentage);

    if (!query.exec()) {
        qDebug() << "Error updating salary settings:" << query.lastError().text();
        return false;
    }

    qDebug() << "Salary settings updated for user:" << userId;
    return true;
}

QMap<QString, QVariant> Database::getSalarySettings(int userId) {
    QMap<QString, QVariant> settings;

    QSqlQuery query;
    query.prepare("SELECT base_salary, percentage FROM salary_settings WHERE user_id = ?");
    query.addBindValue(userId);

    if (query.exec() && query.next()) {
        settings["base_salary"] = query.value(0).toDouble();
        settings["percentage"] = query.value(1).toDouble();
    } else {
        settings["base_salary"] = 0.0;
        settings["percentage"] = 0.0;
    }

    return settings;
}

QList<QMap<QString, QVariant>> Database::getEmployeeFinancialOperations(int userId, const QDate &startDate, const QDate &endDate) {
    QList<QMap<QString, QVariant>> operations;

    QSqlQuery query;
    query.prepare(
        "SELECT id, amount, type, description, operation_date "
        "FROM employee_financial_operations "
        "WHERE user_id = ? AND operation_date BETWEEN ? AND ? "
        "ORDER BY operation_date DESC, id DESC"
    );
    query.addBindValue(userId);
    query.addBindValue(startDate.toString("yyyy-MM-dd"));
    query.addBindValue(endDate.toString("yyyy-MM-dd"));

    if (!query.exec()) {
        qDebug() << "Error getting employee financial operations:" << query.lastError().text();
        return operations;
    }

    while (query.next()) {
        QMap<QString, QVariant> operation;
        operation["id"] = query.value(0).toInt();
        operation["amount"] = query.value(1).toDouble();
        operation["type"] = query.value(2).toString();
        operation["description"] = query.value(3).toString();
        operation["operation_date"] = query.value(4).toDate();

        operations.append(operation);
    }

    return operations;
}

bool Database::addEmployeeFinancialOperation(int userId, double amount, const QString &type, const QString &description, const QDate &date, int createdBy) {
    QSqlQuery query;
    query.prepare(
        "INSERT INTO employee_financial_operations (user_id, amount, type, description, operation_date, created_by) "
        "VALUES (?, ?, ?, ?, ?, ?)"
    );

    query.addBindValue(userId);
    query.addBindValue(amount);
    query.addBindValue(type);
    query.addBindValue(description);
    query.addBindValue(date.toString("yyyy-MM-dd"));
    query.addBindValue(createdBy);

    if (!query.exec()) {
        qDebug() << "Error adding employee financial operation:" << query.lastError().text();
        return false;
    }

    qDebug() << "Employee financial operation added. User:" << userId << "Type:" << type << "Amount:" << amount;
    return true;
}

double Database::getEmployeeServiceEarnings(int userId, const QDate &startDate, const QDate &endDate) {
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
        return query.value(0).toDouble();
    }

    return 0.0;
}
