#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    a.setStyle("Fusion"); // Better looking theme for Windows
    MainWindow w;
    w.show();
    return a.exec();
}