#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSpacerItem>

class RegistrationForm : public QWidget
{
    Q_OBJECT

public:
    RegistrationForm(QWidget *parent = nullptr) : QWidget(parent)
    {
        setupUI();
    }

private:
    void setupUI()
    {
        // Основной layout
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(10);
        mainLayout->setContentsMargins(30, 20, 30, 20);

        // Заголовок "С I I X"
        QLabel *titleLabel = new QLabel("С I I X");
        titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin-bottom: 20px;");
        titleLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(titleLabel);

        // Поля ввода
        addField(mainLayout, "Email");
        addField(mainLayout, "Имя");
        addField(mainLayout, "Пароль");
        addField(mainLayout, "Фамилия");
        addField(mainLayout, "Вы:");
        addField(mainLayout, "Отчество");

        // Радио-кнопки "Владелец" и "Работник"
        QHBoxLayout *radioLayout = new QHBoxLayout();
        QRadioButton *ownerRadio = new QRadioButton("Владелец");
        QRadioButton *workerRadio = new QRadioButton("Работник");

        ownerRadio->setStyleSheet("QRadioButton { font-size: 14px; }");
        workerRadio->setStyleSheet("QRadioButton { font-size: 14px; }");

        radioLayout->addWidget(ownerRadio);
        radioLayout->addWidget(workerRadio);
        radioLayout->addStretch();

        mainLayout->addLayout(radioLayout);

        // Поле "Код приглашения от владельца"
        addField(mainLayout, "Код приглашения от владельца");

        // Кнопка "Зарегистрироваться"
        QPushButton *registerButton = new QPushButton("Зарегистрироваться");
        registerButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #2E7D32;"
            "    color: white;"
            "    border: none;"
            "    padding: 10px;"
            "    font-size: 14px;"
            "    border-radius: 4px;"
            "    font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "    background-color: #1B5E20;"
            "}"
        );
        mainLayout->addWidget(registerButton);

        // Ссылка "У меня уже есть эккаунт"
        QLabel *loginLabel = new QLabel("У меня уже есть эккаунт");
        loginLabel->setStyleSheet(
            "QLabel {"
            "    color: #1976D2;"
            "    font-size: 14px;"
            "    text-decoration: underline;"
            "}"
            "QLabel:hover {"
            "    color: #1565C0;"
            "}"
        );
        loginLabel->setCursor(Qt::PointingHandCursor);
        loginLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(loginLabel);

        // Добавляем растягивающийся элемент в конце
        mainLayout->addStretch();
    }

    void addField(QVBoxLayout *layout, const QString &labelText)
    {
        QLabel *label = new QLabel(labelText);
        label->setStyleSheet("font-size: 14px; margin-bottom: 5px;");

        QLineEdit *lineEdit = new QLineEdit();
        lineEdit->setStyleSheet(
            "QLineEdit {"
            "    border: 1px solid #ccc;"
            "    border-radius: 4px;"
            "    padding: 8px;"
            "    font-size: 14px;"
            "    margin-bottom: 10px;"
            "}"
        );

        layout->addWidget(label);
        layout->addWidget(lineEdit);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    RegistrationForm form;
    form.setWindowTitle("Регистрация");
    form.resize(400, 600);
    form.show();

    return app.exec();
}
