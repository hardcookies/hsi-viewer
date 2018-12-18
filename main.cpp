#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    // 注册GDAL驱动
    GDALAllRegister();

    return a.exec();
}
