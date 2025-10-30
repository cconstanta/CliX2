#ifndef SCHEDULEWINDOW_H
#define SCHEDULEWINDOW_H

#include <QWidget>
#include <QMap>
#include <QTime>
#include <QVector>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QTableWidget;
class QLabel;
class QPushButton;
class QComboBox;
class QDateEdit;
class QLineEdit;
class QTimeEdit;
class QTableWidget;
class QSqlQuery;
QT_END_NAMESPACE

class ScheduleWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ScheduleWindow(QWidget *parent = nullptr);
    ~ScheduleWindow();

signals:
    void backClicked();

private slots:
    void onBackClicked();
    void onCloseClicked();
    void onAddAppointmentClicked();
    void onDateChanged();
    void refreshSchedule();
    void onCellClicked(int row, int column);
    void onCellDoubleClicked(int row, int column);

private:
    void setupUI();
    void refreshScheduleTable();
    void showAddAppointmentDialog(int masterId = -1, const QTime &startTime = QTime(),
                                int appointmentId = -1, const QString &clientName = "",
                                const QString &clientPhone = "", int serviceId = -1,
                                const QDate &appointmentDate = QDate(), const QString &status = "");
    void showEditAppointmentDialog(int appointmentId);
    void setupTimeSlots();
    void clearScheduleTable();
    void setupAppointmentWidget(int row, int column, const QString &clientName,
                               const QString &serviceName, double price, const QString &status);
    bool checkTimeConflict(int masterId, const QDate &date, const QTime &startTime, int duration, int excludeAppointmentId = -1);
    void showStyledMessageBox(const QString &title, const QString &message, bool isError = true);

    QTableWidget *m_scheduleTable;
    QDateEdit *m_dateFilter;

    QVector<QTime> m_timeSlots;
    QMap<int, QString> m_mastersMap; // master_id -> master_name
    QMap<int, int> m_masterRows; // master_id -> row_index
    QMap<int, int> m_timeColumns; // time_index -> column_index
    QMap<QString, int> m_appointmentCells; // "row_col" -> appointment_id

    // Для перетаскивания таблицы
    bool m_tableDragging;
    QPoint m_tableDragStartPosition;
    QPoint m_tableDragStartScroll;

    QPoint m_dragPosition;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
};

#endif // SCHEDULEWINDOW_H
