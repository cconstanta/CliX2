#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QLabel;
QT_END_NAMESPACE

class LoginForm : public QWidget
{
    Q_OBJECT

public:
    explicit LoginForm(QWidget *parent = nullptr);
    ~LoginForm();

signals:
    void loginAttempt(const QString &email, const QString &password);
    void registrationClicked();

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    void setupUI();

    QLineEdit *m_emailInput;
    QLineEdit *m_passwordInput;
    QPushButton *m_loginButton;
    QPushButton *m_registerButton;
    QLabel *m_titleLabel;
    QPoint m_dragPosition;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // LOGINFORM_H
