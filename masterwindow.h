#ifndef MASTERWINDOW_H
#define MASTERWINDOW_H

#include <QWidget>
#include <QMouseEvent>
#include <QPoint>

class MasterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MasterWindow(QWidget *parent = nullptr);
    ~MasterWindow() override;

    void setUserInfo(const QString &userName, const QString &email) {
        Q_UNUSED(userName);
        Q_UNUSED(email);
        // Реализация метода
    }

    // Добавляем метод setUserId
    void setUserId(int userId) { m_userId = userId; }

signals:
    void loginClicked();
    void openWorkWindow();

private slots:
    void onLogoutClicked();
    void onCloseClicked();
    void onWorkButtonClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QPoint m_dragPosition;
    int m_userId = -1; // Добавляем ID пользователя с значением по умолчанию
};

#endif // MASTERWINDOW_H
