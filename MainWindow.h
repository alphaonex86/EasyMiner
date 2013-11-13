#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QProcess>
#include <QTranslator>
#include <QCloseEvent>
#include <QFile>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QSettings settings;
private slots:
    void on_actionQuit_triggered();
    void updateMinimizeTxt();
    void on_actionMinimize_triggered();
    void activated(QSystemTrayIcon::ActivationReason reason);
    void init();
    void closeEvent(QCloseEvent *event);

    void startMiner();
    void error( QProcess::ProcessError error );
    void finished( int exitCode, QProcess::ExitStatus exitStatus );
    void readyReadStandardError();
    void readyReadStandardOutput();
    void firstShow();
    void updateTheInformations();

    void on_actionEnglish_triggered();
    void on_actionFran_ais_triggered();
    void on_actionEspanol_triggered();
    void updateTheLanguage();
    void on_actionStart_minimized_triggered();
    void on_bitcoinAddress_editingFinished();

private:
    Ui::MainWindow *ui;
    QMenu menu;
    QSystemTrayIcon systray;
    QTimer initTimer;
    QTimer autorestartminer,restartminer;
    QProcess miner;
    double khs,khsMax;
    QTranslator translator;
    QStringList extraArgs;
    QTimer updateTheInformationsTimer;
    QFile log;
protected:
    void changeEvent(QEvent *e);
};

#endif // MAINWINDOW_H
