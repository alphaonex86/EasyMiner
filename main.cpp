#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    QCoreApplication::setOrganizationName("Ultracopier");
    QCoreApplication::setApplicationName("Easy Miner");
    MainWindow w;

    return a.exec();
}
