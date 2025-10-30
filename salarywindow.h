#ifndef SALARYWINDOW_H
#define SALARYWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QScrollArea;
QT_END_NAMESPACE

class SalaryWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SalaryWindow(int userId, QWidget *parent = nullptr);
    ~SalaryWindow();

signals:
    void backClicked();

private slots:
    void onBackClicked();
    void onCloseClicked();

private:
    void setupUI();
    void refreshSalaryData();
    void refreshSalaryHistory();
    void calculateCurrentMonthSalary();

    // Вспомогательные методы
    double calculateServiceEarnings(const QDate &startDate, const QDate &endDate);
    double getTotalBonuses(const QDate &startDate, const QDate &endDate);
    double getTotalDeductions(const QDate &startDate, const QDate &endDate);
    double getTotalAdvances(const QDate &startDate, const QDate &endDate);
    double getTotalSalaryPayments(const QDate &startDate, const QDate &endDate);

    int m_userId;
    QString m_userName;
    QString m_userRole;

    // UI элементы
    QLabel *m_baseSalaryLabel;
    QLabel *m_percentageLabel;
    QLabel *m_totalEarningsLabel;
    QLabel *m_serviceEarningsLabel;
    QLabel *m_bonusEarningsLabel;
    QLabel *m_deductionsLabel;
    QLabel *m_advancesLabel;
    QLabel *m_netSalaryLabel;
    QLabel *m_payableLabel;

    QWidget *m_historyContainer;
    QVBoxLayout *m_historyLayout;

    QPoint m_dragPosition;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // SALARYWINDOW_H
