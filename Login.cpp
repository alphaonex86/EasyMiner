#include "Login.h"
#include "ui_Login.h"

Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
    m_validated=false;
}

Login::~Login()
{
    delete ui;
}

void Login::on_pushButton_clicked()
{
    m_validated=true;
    accept();
    close();
}

QString Login::getLogin()
{
    return ui->login->text();
}

QString Login::getWorkerName()
{
    return ui->workerName->text();
}

QString Login::getWorkerPass()
{
    return ui->workerPass->text();
}

void Login::setLogin(const QString &text)
{
    ui->login->setText(text);
}

void Login::setWorkerName(const QString &text)
{
    ui->workerName->setText(text);
}

void Login::setWorkerPass(const QString &text)
{
    ui->workerPass->setText(text);
}

bool Login::validated()
{
    return m_validated;
}

void Login::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Login::on_login_returnPressed()
{
    ui->workerName->setFocus();
}

void Login::on_workerName_returnPressed()
{
    ui->workerPass->setFocus();
}

void Login::on_workerPass_returnPressed()
{
    on_pushButton_clicked();
}
