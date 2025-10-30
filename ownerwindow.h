#ifndef OWNERWINDOW_H
#define OWNERWINDOW_H

#include <QWidget>
#include <QPoint>
#include <QMouseEvent>

class OwnerWindow : public QWidget {
    Q_OBJECT

public:
    explicit OwnerWindow(QWidget *parent = nullptr);
    ~OwnerWindow() override;

signals:
    void loginClicked();
    void openEmployeesWindow();
    void openPayrollWindow();

private slots:
    void onLogoutClicked();
    void onCloseClicked();
    void onEmployeesButtonClicked();
    void onPayrollButtonClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QPoint m_dragPosition;
};

#endif // OWNERWINDOW_H
