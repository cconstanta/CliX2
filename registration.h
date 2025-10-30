#ifndef REGISTRATIONFORM_H
#define REGISTRATIONFORM_H

#include <QWidget>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QLabel;
QT_END_NAMESPACE

class RegistrationForm : public QWidget
{
    Q_OBJECT

public:
    explicit RegistrationForm(QWidget *parent = nullptr);
    ~RegistrationForm();

signals:
    void loginClicked();
    void registrationSuccess();

private slots:
    void onRegisterClicked();
    void onLoginClicked();

private:
    void setupUI();
    void showRoleSelectionDialog();
    void onRoleChanged(const QString &role); // ИЗМЕНИЛИ СИГНАТУРУ
    void showMessageBox(const QString &title, const QString &text, QMessageBox::Icon icon);
    QString getRoleDisplayName(const QString &role);

    QLineEdit *m_emailInput;
    QLineEdit *m_passwordInput;
    QLineEdit *m_confirmPasswordInput;
    QLineEdit *m_firstNameInput;
    QLineEdit *m_lastNameInput;
    QLineEdit *m_middleNameInput;
    QLineEdit *m_inviteCodeInput;
    QPushButton *m_roleButton;
    QPushButton *m_registerButton;
    QPushButton *m_loginButton;
    QLabel *m_inviteCodeLabel;
    QPoint m_dragPosition;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // REGISTRATIONFORM_H
