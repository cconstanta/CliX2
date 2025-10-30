#ifndef EMPLOYEESWINDOW_H
#define EMPLOYEESWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QLineEdit;
class QComboBox;
QT_END_NAMESPACE

class EmployeesWindow : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeesWindow(QWidget *parent = nullptr);
    ~EmployeesWindow();

signals:
    void backClicked();

private slots:
    void onBackClicked();
    void onCloseClicked();
    void onAddEmployeeClicked();
    void onCopyCodeClicked();

private:
    void setupUI();
    void refreshEmployeesList();
    void createEmployeeCard(QWidget *parent, int id, const QString &name,
                           const QString &role, bool isActive);
    void showAddEmployeeDialog();
    void showDeleteConfirmationDialog(int userId, const QString &userName);
    QString generateInviteCode();
    QString getRoleDisplayName(const QString &role);
    QString getRoleColor(const QString &role);

    QWidget *m_employeesContainer;
    QVBoxLayout *m_employeesLayout;
    QLineEdit *m_inviteCodeInput;
    QPoint m_dragPosition;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // EMPLOYEESWINDOW_H
