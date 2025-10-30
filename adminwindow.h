#ifndef ADMINWINDOW_H
#define ADMINWINDOW_H

#include <QWidget>
#include <QPoint>

class AdminWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AdminWindow(QWidget *parent = nullptr);
    ~AdminWindow();

    void setUserInfo(const QString &userName, const QString &email) {
        // Реализация метода для установки информации о пользователе
        Q_UNUSED(userName);
        Q_UNUSED(email);
        // Можно добавить логику обновления интерфейса
    }

signals:
    void loginClicked();
    void openScheduleWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void onLogoutClicked();
    void onCloseClicked();
    void onScheduleButtonClicked();

private:
    QPoint m_dragPosition;
    void checkResources();
    QIcon loadIcon(const QString &path, const QString &fallbackText = "");
};

#endif // ADMINWINDOW_H
