#ifndef FINANCESWINDOW_H
#define FINANCESWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QTableWidget;
class QLabel;
class QPushButton;
class QComboBox;
class QDateEdit;
class QLineEdit;
QT_END_NAMESPACE

class FinancesWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FinancesWindow(QWidget *parent = nullptr);
    ~FinancesWindow();

signals:
    void backClicked();

private slots:
    void onBackClicked();
    void onCloseClicked();
    void onAddOperationClicked();
    void onFilterChanged();
    void onDeleteOperationClicked();
    void refreshFinanceData();

private:
    void setupUI();
    void refreshOperationsList();
    void refreshStats();
    void showAddOperationDialog();
    QString getCategoryColor(const QString &category);
    QString formatCurrency(double amount);

    QWidget *m_operationsContainer;
    QVBoxLayout *m_operationsLayout;

    QLabel *m_totalIncomeLabel;
    QLabel *m_totalExpensesLabel;
    QLabel *m_balanceLabel;

    QComboBox *m_categoryFilter;
    QDateEdit *m_startDateFilter;
    QDateEdit *m_endDateFilter;

    QPoint m_dragPosition;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // FINANCESWINDOW_H

