#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Login.h"

#include <QFile>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDebug>

#ifdef Q_OS_LINUX
    #define ULTRACOPIER_CGMINER_PATH "/usr/bin/bfgminer"
#else
    #define ULTRACOPIER_CGMINER_PATH QCoreApplication::applicationDirPath()+"/bfgminer/bfgminer.exe"
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
    khs=0;
    khsMax=0;
    ui->setupUi(this);
    systray.setIcon(QIcon(":/bitcoin-16x16.png"));
    systray.setToolTip("Easy Miner");
    menu.addAction(ui->actionMinimize);
    menu.addAction(ui->actionQuit);
    systray.setContextMenu(&menu);
    connect(&systray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(activated(QSystemTrayIcon::ActivationReason)));
    initTimer.setSingleShot(true);
    initTimer.start(0);
    connect(&initTimer,SIGNAL(timeout()),this,SLOT(init()));
    updateTheLanguage();
    ui->statusBar->showMessage(tr("Welcome to %1").arg("Easy Miner"));
    extraArgs << "--no-adl" << "-T";
    #ifdef Q_OS_LINUX
    // << "-C" << "-G"
    #endif
    ui->labelBitcoin->setPixmap(QPixmap(":/bitcoin-disabled-256x256.png"));
    log.setFileName(QCoreApplication::applicationDirPath()+"/log.log");
    log.open(QIODevice::WriteOnly);
}

void MainWindow::activated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason==QSystemTrayIcon::Trigger || reason==QSystemTrayIcon::DoubleClick)
        on_actionMinimize_triggered();
}

void MainWindow::updateMinimizeTxt()
{
    if(systray.isVisible())
    {
        hide();
        ui->actionMinimize->setText(tr("Restore"));
    }
    else
    {
        show();
        ui->actionMinimize->setText(tr("Minimize"));
    }
}

MainWindow::~MainWindow()
{
    miner.terminate();
    miner.kill();
    delete ui;
    miner.waitForFinished();
    log.close();
}

void MainWindow::on_actionLogin_triggered()
{
    Login login(this);
    if(settings.contains("login"))
        login.setLogin(settings.value("login").toString());
    if(settings.contains("worker_name"))
        login.setWorkerName(settings.value("worker_name").toString());
    if(settings.contains("worker_pass"))
        login.setWorkerPass(settings.value("worker_pass").toString());
    login.exec();
    if(login.validated())
    {
        settings.setValue("login",login.getLogin());
        settings.setValue("worker_name",login.getWorkerName());
        settings.setValue("worker_pass",login.getWorkerPass());
        startMiner();
    }
}

void MainWindow::on_actionQuit_triggered()
{
    QCoreApplication::quit();
}

void MainWindow::on_actionMinimize_triggered()
{
    systray.setVisible(!systray.isVisible());
    updateMinimizeTxt();
}

void MainWindow::init()
{
    bool OpenCLDll=false;
    #ifdef Q_OS_WIN32
    char *arch=getenv("windir");
    if(arch!=NULL)
    {

        if(QFile(QString(arch)+"\\System32\\OpenCL.dll").exists()
            #if defined(_M_X64)
            || QFile(QString(arch)+"\\SysWOW64\\OpenCL.dll").exists()
            #endif
        )
            OpenCLDll=true;
    }
    #else
        OpenCLDll=true;
    #endif
    if(!QFile(ULTRACOPIER_CGMINER_PATH).exists() || !OpenCLDll)
    {
        if(!QFile(ULTRACOPIER_CGMINER_PATH).exists())
            QMessageBox::critical(this,tr("Allow the application"),tr("You need allow into your antivirus %1").arg(ULTRACOPIER_CGMINER_PATH));
        if(!OpenCLDll)
            QMessageBox::critical(this,tr("Enable the OpenCL"),tr("You need enable the OpenCL"));
        QCoreApplication::exit();
        return;
    }
    else
    {
        connect(&miner,SIGNAL(error(QProcess::ProcessError)),this,SLOT(error(QProcess::ProcessError)),Qt::QueuedConnection);
        connect(&miner,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(finished(int,QProcess::ExitStatus)),Qt::QueuedConnection);
        connect(&miner,SIGNAL(readyReadStandardError()),this,SLOT(readyReadStandardError()),Qt::QueuedConnection);
        connect(&miner,SIGNAL(readyReadStandardOutput()),this,SLOT(readyReadStandardOutput()),Qt::QueuedConnection);

        autorestartminer.setInterval(60*60*1000);
        autorestartminer.start();
        connect(&autorestartminer,SIGNAL(timeout()),this,SLOT(startMiner()),Qt::QueuedConnection);
        restartminer.setInterval(60*1000);
        restartminer.setSingleShot(true);
        connect(&restartminer,SIGNAL(timeout()),this,SLOT(startMiner()),Qt::QueuedConnection);

        connect(&updateTheInformationsTimer,SIGNAL(timeout()),this,SLOT(updateTheInformations()),Qt::QueuedConnection);
        updateTheInformationsTimer.start(5*1000);

        if(!settings.contains("login") || !settings.contains("worker_name") || !settings.contains("worker_pass"))
        {
            on_actionLogin_triggered();
            firstShow();
            return;
        }
        startMiner();
        firstShow();
    }
}

void MainWindow::firstShow()
{
    if(settings.contains("start_minimized"))
        if(settings.value("start_minimized").toBool())
        {
            systray.show();
            updateMinimizeTxt();
            ui->actionStart_minimized->setChecked(true);
            return;
        }
    show();
}

void MainWindow::startMiner()
{
    QStringList args;
    if(!settings.contains("login") || !settings.contains("worker_name") || !settings.contains("worker_pass"))
    {
        ui->statusBar->showMessage(tr("Put the login and worker informations"));
        return;
    }
    if(settings.value("login").toString().isEmpty())
    {
        ui->statusBar->showMessage(tr("The login can't be empty"));
        return;
    }
    if(settings.value("worker_name").toString().isEmpty())
    {
        ui->statusBar->showMessage(tr("The worker name can't be empty"));
        return;
    }
    if(settings.value("worker_pass").toString().isEmpty())
    {
        ui->statusBar->showMessage(tr("The pass can't be empty"));
        return;
    }
    ui->statusBar->showMessage(tr("Mining started"));
    miner.terminate();
    miner.kill();
    miner.waitForFinished();
    args << extraArgs;
    args << "-o" << "stratum+tcp://mint.bitminter.com:3333" << "-u" << QString("%1_%2").arg(settings.value("login").toString()).arg(settings.value("worker_name").toString()) << "-p" << settings.value("worker_pass").toString();
    args << "-o" << "http://mint.bitminter.com:8332" << "-u" << QString("%1_%2").arg(settings.value("login").toString()).arg(settings.value("worker_name").toString()) << "-p" << settings.value("worker_pass").toString();
    qDebug() << "debug code: 9uyFDQGLyTN8bFcq";
    if(log.isOpen())
        log.write(QString("%1\n").arg("debug code: 9uyFDQGLyTN8bFcq").toUtf8());
    miner.start(ULTRACOPIER_CGMINER_PATH,args);
    qDebug() << ULTRACOPIER_CGMINER_PATH << args;
    if(log.isOpen())
        log.write(QString("%1\n").arg(QString(ULTRACOPIER_CGMINER_PATH)+" "+args.join(" ")).toUtf8());
    ui->labelBitcoin->setPixmap(QPixmap(":/bitcoin-256x256.png"));
}

void MainWindow::error( QProcess::ProcessError error )
{
    Q_UNUSED(error);
    //ui->statusBar->showMessage(tr("Mining is in error: %1").arg(error));
    //if(error==QProcess::Crashed)
    qDebug() << "error()" << error;
    if(log.isOpen())
        log.write(QString("error: %1\n").arg(error).toUtf8());
}

void MainWindow::finished( int exitCode, QProcess::ExitStatus exitStatus )
{
    qDebug() << "finished()";
    if(log.isOpen())
        log.write(QString("exitCode: %1, exitStatus: %2\n").arg(exitCode).arg(exitStatus).toUtf8());
    restartminer.start();
    khsMax=0;
    khs=0;
    /*if(exitCode!=0)
        ui->statusBar->showMessage(tr("Mining is in finished with exit code: %1 in status: %2").arg((quint32)exitCode).arg(exitStatus));*/
}

void MainWindow::readyReadStandardError()
{
    QString error=miner.readAllStandardError();
    qDebug() << error;
    if(log.isOpen())
        log.write(QString("error: %1\n").arg(error).toUtf8());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    if(!systray.isVisible())
        QCoreApplication::quit();
}

void MainWindow::readyReadStandardOutput()
{
    QString output=miner.readAllStandardOutput();
    if(output.contains(QRegExp("([0-9]+(\\.[0-9]+)?) kh")))
    {
        output.replace(QRegExp("^.*avg:[\t ]*([0-9]+(\\.[0-9]+)?) $"),"\\1");
        khs=output.toFloat();
    }
    if(output.contains(QRegExp("([0-9]+(\\.[0-9]+)?) Mh")))
    {
        output.replace(QRegExp("^.*avg:[\t ]*([0-9]+(\\.[0-9]+)?) .*$"),"\\1");
        khs=output.toFloat()*1000;
    }
    if(output.contains(QRegExp("([0-9]+(\\.[0-9]+)?) Gh")))
    {
        output.replace(QRegExp("^.*avg:[\t ]*([0-9]+(\\.[0-9]+)?) .*$"),"\\1");
        khs=output.toFloat()*1000*1000;
    }
    if(output.contains(QRegExp("([0-9]+(\\.[0-9]+)?) Th")))
    {
        output.replace(QRegExp("^.*avg:[\t ]*([0-9]+(\\.[0-9]+)?) .*$"),"\\1");
        khs=output.toInt()*1000*1000;
    }
    if(khs>khsMax)
        khsMax=khs;
    if(output.contains("have not set up workers"))
        ui->statusBar->showMessage(tr("Error to login on the pool (wrong login or worker informations?)"));
    qDebug() << output;
}

void MainWindow::updateTheInformations()
{
    double total_khs=khs;
    double total_khsMax=khsMax;
    ui->progressBar->setMaximum(total_khsMax);
    ui->progressBar->setValue(total_khs);
    if(total_khs<1000)
        ui->progressBar->setFormat(tr("%1 kh/s").arg(total_khs));
    else if(total_khs<1000*1000)
        ui->progressBar->setFormat(tr("%1 Mh/s").arg(total_khs/(1000)));
    else if(total_khs<1000*1000*1000)
        ui->progressBar->setFormat(tr("%1 Gh/s").arg(total_khs/(1000*1000)));
    else
        ui->progressBar->setFormat(tr("%1 Th/s").arg(total_khs/(1000*1000*1000)));
    if(miner.state()==QProcess::NotRunning)
        ui->labelBitcoin->setPixmap(QPixmap(":/bitcoin-disabled-256x256.png"));
    else
        ui->labelBitcoin->setPixmap(QPixmap(":/bitcoin-256x256.png"));
}

void MainWindow::on_actionEnglish_triggered()
{
    settings.setValue("language",0);
    updateTheLanguage();
}

void MainWindow::on_actionFran_ais_triggered()
{
    settings.setValue("language",1);
    updateTheLanguage();
}

void MainWindow::on_actionEspanol_triggered()
{
    settings.setValue("language",2);
    updateTheLanguage();
}

void MainWindow::updateTheLanguage()
{
    QCoreApplication::removeTranslator(&translator);
    if(settings.contains("language"))
    {
        switch(settings.value("language").toInt())
        {
            case 1:
                translator.load(":/ts/fr.qm");
            break;
            case 2:
                translator.load(":/ts/es.qm");
            break;
            case 0:
            default:
                translator.load("");
            break;
        }
        switch(settings.value("language").toInt())
        {
            case 1:
            case 2:
                QCoreApplication::installTranslator(&translator);
            break;
            case 0:
            default:
            break;
        }
        ui->retranslateUi(this);
        updateTheInformations();
    }
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        updateTheInformations();
        break;
    default:
        break;
    }
}

void MainWindow::on_actionStart_minimized_triggered()
{
    settings.setValue("start_minimized",ui->actionStart_minimized->isChecked());
}
