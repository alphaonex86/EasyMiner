#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    ~Login();
    QString getLogin();
    QString getWorkerName();
    QString getWorkerPass();
    void setLogin(const QString &text);
    void setWorkerName(const QString &text);
    void setWorkerPass(const QString &text);
    bool validated();
private slots:
    void on_pushButton_clicked();
    void on_login_returnPressed();
    void on_workerName_returnPressed();
    void on_workerPass_returnPressed();
private:
    Ui::Login *ui;
    bool m_validated;
protected:
    void changeEvent(QEvent *e);
};

#endif // LOGIN_H
