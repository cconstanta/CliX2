#ifndef PAYROLLWINDOW_H
#define PAYROLLWINDOW_H

#include <QWidget>
#include <QMap>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QTableWidget;
class QComboBox;
class QLineEdit;
class QDateEdit;
class QScrollArea;
QT_END_NAMESPACE

class PayrollWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PayrollWindow(QWidget *parent = nullptr);
    ~PayrollWindow();

signals:
    void backClicked();
    void dataUpdated(); // Добавляем сигнал об обновлении данных

private slots:
    void onBackClicked();
    void onCloseClicked();
    void onEmployeeButtonClicked();
    void onOperationTypeButtonClicked();
    void onSaveSalarySettings();
    void onAddFinancialOperation();
    void onRefreshData();
    void onDeleteOperation(int operationId);

private:
    void setupUI();
    void refreshEmployeesList();
    void refreshSalaryData();
    void refreshFinancialOperations();
    void calculateCurrentMonthEarnings();

    // Вспомогательные методы
    double calculateEmployeeEarnings(int userId, const QDate &startDate, const QDate &endDate);
    double getEmployeePercentage(int userId);
    double getEmployeeBaseSalary(int userId);
    QLabel* createStatLabel(const QString &title, const QString &value, const QString &color);

    // Данные
    int m_selectedEmployeeId;
    QString m_currentOperationType;
    QMap<QString, QVariant> m_currentEmployeeData;

    // UI элементы
    QPushButton *m_employeeButton;
    QLabel *m_employeeLabel;
    QPushButton *m_changeEmployeeButton;
    QLineEdit *m_baseSalaryInput;
    QLineEdit *m_percentageInput;
    QLineEdit *m_operationAmountInput;
    QPushButton *m_operationTypeButton;
    QLabel *m_operationTypeLabel;
    QPushButton *m_changeOperationTypeButton;
    QLineEdit *m_operationDescriptionInput;

    QLabel *m_totalEarningsLabel;
    QLabel *m_salaryEarningsLabel;
    QLabel *m_percentageEarningsLabel;
    QLabel *m_bonusEarningsLabel;
    QLabel *m_deductionEarningsLabel;
    QLabel *m_netEarningsLabel;
    QLabel *m_completedServicesLabel;

    QWidget *m_operationsContainer;
    QVBoxLayout *m_operationsLayout;

    QPoint m_dragPosition;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // PAYROLLWINDOW_H
