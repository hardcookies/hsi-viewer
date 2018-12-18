#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gdal_priv.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QTreeWidget>
#include <QListWidget>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    // 枚举类型 TreeItemType，创建节点type参数
    enum TreeItemType{FileItem=1001, BandItem};

    QLabel *label_filename;    // 状态栏显示文件名
    GDALDataset *cur_dataset;   // 当前数据集
    QStringList RGB_dataset;    // 三个波段对应数据集

    QStringList getBandName(QString filename);
    QList<float> getWavelength(QString filename);
    void addFileItem(QString filename);
    QString getFilename(QString full_filename);
    const char* qstringToChar(QString str);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_pushButton_clicked();

    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_radioButton_2_clicked();

    void on_radioButton_clicked();

    void on_pushButton_2_clicked();

    void on_actionCloseAll_triggered();

    void on_actionOptions_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
