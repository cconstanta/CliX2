#ifndef WORKWINDOW_H
#define WORKWINDOW_H

#include <QWidget>
#include <QMap>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QLabel;
class QPushButton;
class QProgressBar;
class QScrollArea;
QT_END_NAMESPACE

class WorkWindow : public QWidget
{
    Q_OBJECT

public:
    explicit WorkWindow(int userId, QWidget *parent = nullptr);
    ~WorkWindow();

signals:
    void backClicked();

private slots:
    void onBackClicked();
    void onCloseClicked();
    void onAppointmentStatusChanged(int appointmentId, bool completed);

private:
    void setupUI();
    void refreshWorkData();
    void refreshAppointmentsList();
    void refreshStats();
    void createAppointmentCard(QWidget *parent, int appointmentId,
                              const QString &clientName, const QString &serviceName,
                              double price, const QTime &time, const QString &status);

    // ДОБАВЬТЕ ЭТОТ МЕТОД:
    double getMasterPercentage();

    int m_userId;
    QString m_userName;

    QWidget *m_appointmentsContainer;
    QVBoxLayout *m_appointmentsLayout;

    QLabel *m_todayEarningsLabel;
    QLabel *m_completedCountLabel;
    QLabel *m_pendingCountLabel;
    QLabel *m_cancelledCountLabel;
    QProgressBar *m_progressBar;

    QPoint m_dragPosition;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // WORKWINDOW_H
