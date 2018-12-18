#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <iostream>
#include <vector>
#include <string>

using namespace cv;
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 添加状态栏
    label_filename = new QLabel("");
    ui->statusBar->addWidget(label_filename);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 解析hdr得到波段名
QStringList MainWindow::getBandName(QString filename)
{
    QFile file(filename);
    QStringList band_name;
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        QString content = in.readAll();
        // 查找band names位置
        int pos = content.indexOf("band names");
        // 处理字符串
        QChar ch;
        while ((ch = content[pos]) != "{")
            pos++;
        int beg = pos+1;
        while ((ch = content[pos]) != "}")
            pos++;
        int end = pos-1;
        QString bnstr = content.mid(beg, end-beg+1);
        bnstr.replace("\r\n", "\n");    // 对于不同格式换行统一处理
        band_name = bnstr.split(QRegExp("[\\n,]"), QString::SkipEmptyParts);   // 利用正则表达式，有小问题（已解决）。
        // 删除列表中" "字符串
        auto it = band_name.begin();
        while (it != band_name.end())
        {
            *it = it->trimmed();
            if (it->isEmpty())
                it = band_name.erase(it);
            else
                it++;
        }
        file.close();
    }
    return band_name;
}

QList<float> MainWindow::getWavelength(QString filename)
{
    QFile file(filename);
    QList<float> wavelength;
    QStringList wavlist;
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        QString content = in.readAll();
        // 查找wavelength位置
        // int pos = content.indexOf("default bands");     // 直接搜索"wavelength"匹配到"wavelength units"了
        int pos = content.lastIndexOf("wavelength");    // 搜索到最后出现的"wavelength"，假设单位在前
        // 处理字符串
        QChar ch;
        while ((ch = content[pos]) != "{")
            pos++;
        int beg = pos+1;
        while ((ch = content[pos]) != "}")
            pos++;
        int end = pos-1;
        QString wavstr = content.mid(beg, end-beg+1);
        wavstr.replace("\r\n", "\n");    // 对于不同格式换行统一处理
        wavlist = wavstr.split(QRegExp("[\\n,]"), QString::SkipEmptyParts);   // 利用正则表达式，有小问题（已解决）。
        // 删除列表中" "字符串
        auto it = wavlist.begin();
        while (it != wavlist.end())
        {
            *it = it->trimmed();
            if (it->isEmpty())
                it = wavlist.erase(it);
            else
                it++;
        }
        // 字符转换成数值
        for (int i = 0; i < wavlist.size(); ++i)
            wavelength.append(wavlist[i].toFloat());
        file.close();
    }
    return wavelength;
}

void MainWindow::addFileItem(QString filename)
{
    // 设置文件节点图标
    QIcon file_icon(":/images/icon/layers.ico");
    // 添加文件节点至顶层节点
    QTreeWidgetItem *file_item = new QTreeWidgetItem(MainWindow::FileItem);
    file_item->setText(0, getFilename(filename));      // 设置显示为最后文件名
    file_item->setData(0, Qt::UserRole, QVariant(filename));    // 设置数据为完整文件名
    file_item->setIcon(0, file_icon);    // 添加文件节点图标
    // 设置文件节点属性
    file_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui->treeWidget->addTopLevelItem(file_item);
    // 得到头文件
    int dot_index = filename.lastIndexOf(".");
    QString hdr_file = filename.mid(0,dot_index+1) + "hdr";
    // 读取波段名称
    QStringList band_names = getBandName(hdr_file);
    QList<float> wavelength = getWavelength(hdr_file);
    // 添加波段节点至当前文件节点
    for (int i = 0; i < band_names.size(); ++i)
    {
        // 设置波段节点图标
        QIcon band_icon(":/images/icon/square.ico");
        // 添加波段节点至文件节点
        QTreeWidgetItem *band_item = new QTreeWidgetItem(MainWindow::BandItem);
        band_item->setText(0, QString("%1 (%2)").arg(band_names[i], QString::number(wavelength[i])));
        band_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        band_item->setIcon(0, band_icon);
        file_item->addChild(band_item);
    }
}

// 完整路径提取文件名
QString MainWindow::getFilename(QString full_filename)
{
    // 从完整路径中得到文件名
    int cnt = full_filename.length();
    int pos = full_filename.lastIndexOf("/");
    QString filename = full_filename.right(cnt-1-pos);
    return filename;
}

// QString转换为const char*
const char* MainWindow::qstringToChar(QString str)
{
    QByteArray ba = str.toLocal8Bit();
    const char *char_ptr = ba.data();    // 将QString转化为字符串指针
    return char_ptr;
}

// 打开高光谱数据
void MainWindow::on_actionOpen_triggered()
{
    // 选择文件
    QString file = QFileDialog::getOpenFileName(this,"选择一个高光谱文件");
    if (file.isEmpty()) //如果一个文件都没选
        return;
    // 打开数据集
    GDALDataset *poDataset;
    const char *filename = qstringToChar(file);
    poDataset = (GDALDataset *) GDALOpen(filename, GA_ReadOnly);
    if (poDataset == NULL)
    {
        QMessageBox::warning(this, "错误", "加载数据错误");
        return;
    }
    // 在QTreeWidget添加节点
    addFileItem(file);
}

// 显示当前数据集的波段影像
void MainWindow::on_pushButton_clicked()
{
    if (cur_dataset == NULL)
        return;

    // 获取当前数据集信息
    int width, height;
    width = cur_dataset->GetRasterXSize();
    height = cur_dataset->GetRasterYSize();

    // 加载Gray图像
    if (ui->radioButton->isChecked())
    {
        QTreeWidgetItem *item = ui->treeWidget->currentItem();
        // 选中波段项
        if (item->type() == BandItem)
        {
            QTreeWidgetItem *par_item = item->parent();
            int band_index = par_item->indexOfChild(item);   // 获得波段号，从0开始。
            // 读取波段
            GDALRasterBand  *poBand;
            float *data = new float[width*height];

            poBand = cur_dataset->GetRasterBand(band_index+1);
            poBand->RasterIO(GF_Read, 0, 0, width, height, data,
                width, height, GDT_Float32, 0, 0);
            // 存入Mat格式
            Mat band_img(height, width, CV_32F, data);
            // 转换并显示
            Mat img_gray;
            double min, max;
            minMaxLoc(band_img, &min, &max, NULL, NULL);
            band_img.convertTo(img_gray, CV_8U, 255/(max-min), -255*min/(max-min));
            string window_name = par_item->text(0).toStdString();     // 文件名std::string
            namedWindow(window_name, CV_WINDOW_AUTOSIZE);
            imshow(window_name, img_gray);
        }
    }
    // 加载RGB图像
    if (ui->radioButton_2->isChecked())
    {
        // 根据维基百科Color词条认为 R:580nm, G:540nm, B:430nm（暂时不用）
        // 有一点目前还没做，需要检验RGB三个波段同属一个数据集。
        // 确保已选三个波段
        if (ui->listWidget->count() != 3)
            return;
        // 判断三者同属一个数据集
        bool areSame = true;
        for (int i = 0; i < 2; ++i)
        {
            if (RGB_dataset[i+1] != RGB_dataset[i])
                areSame = false;
        }
        if (!areSame)
        {
            QMessageBox::warning(this, "错误", "波段不属于同一影像");
            return;
        }
        // 获取RGB波段号
        QList<int> band_rgb;
        for (int i = 0; i < 3; ++i)
        {
            QListWidgetItem *item = ui->listWidget->item(i);
            band_rgb.append(item->data(Qt::UserRole).toInt());  // 添加波段号至列表
        }
        // qDebug() << band_rgb;
        // 分别读取BGR然后合并为彩色
        vector<Mat> images;
        GDALRasterBand  *poBand;
        float *data = new float[width*height];
        for (int i = 2; i >= 0; --i)    // BGR
        {
            poBand = cur_dataset->GetRasterBand(band_rgb[i]);
            poBand->RasterIO(GF_Read, 0, 0, width, height, data,
                    width, height, GDT_Float32, 0, 0);
            // 转为Mat格式
            Mat band_image(height, width, CV_32F, data);
            // 线性拉伸
            Mat image;
            double min, max;
            minMaxLoc(band_image, &min, &max, NULL, NULL);
            band_image.convertTo(image, CV_8U, 255/(max-min), -255*min/(max-min));
            images.push_back(image);
        }
        Mat color_image;
        merge(images, color_image);
        // 打开窗口并显示
        const char *filename = cur_dataset->GetDescription();   // 由当前数据集获得文件名
        string str(filename);   // 转换成string
        size_t found = str.find_last_of("/\\");
        string window_name = str.substr(found+1);
        namedWindow(window_name, CV_WINDOW_AUTOSIZE);
        imshow(window_name, color_image);
    }
}

// 当前节点变化后响应，点击波段节点生效。
void MainWindow::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    // 节点变化时相应处理，注意previous可能为NULL!
    Q_UNUSED(previous);
    if (current == NULL)
        return;
    // 若选择文件节点则无效
    if (current->type() != BandItem)
        return;
    // 选择波段节点，设置为当前数据集
    QTreeWidgetItem *par_item = current->parent();
    int nDScount = 0;
    GDALDataset **openedDSList = GDALDataset::GetOpenDatasets(&nDScount);   // 当前打开数据集列表
    int file_index = ui->treeWidget->indexOfTopLevelItem(par_item);      // 所选文件节点序号
    cur_dataset = openedDSList[file_index];
    // 状态栏显示当前数据集
    label_filename->setText(par_item->text(0));
    // qDebug() << "DS" << file_index;
    // ListWidget显示选中波段
    if (ui->radioButton->isChecked())
    {
        // 显示所选灰度波段
        ui->listWidget->clear();
        QIcon icon(":/images/icon/gray.png");
        QListWidgetItem *header, *band;
        header = new QListWidgetItem;
        band = new QListWidgetItem;
        header->setText("Selected band:");
        band->setIcon(icon);
        band->setText(current->text(0));
        ui->listWidget->addItem(header);
        ui->listWidget->addItem(band);
    }
    if (ui->radioButton_2->isChecked())
    {
        // 显示所选RGB波段
        int n = ui->listWidget->count();
        // 判断已选波段数
        if (n == 3)
        {
            // 重新选择
            ui->listWidget->clear();
            // 清除波段数据集
            RGB_dataset.clear();
            n = 0;
        }
        // 获得所选波段号
        int band_index = par_item->indexOfChild(current)+1;   // GDAL从1起始
        // 保存并添加至波段列表
        QStringList color = {"R: ", "G: ", "B: "};
        QIcon icon_R(":/images/icon/red.png");
        QIcon icon_G(":/images/icon/green.png");
        QIcon icon_B(":/images/icon/blue.png");
        QList<QIcon> icon = {icon_R, icon_G, icon_B};
        QListWidgetItem *band = new QListWidgetItem;
        // 显示颜色和对应波段
        band->setText(color[n]+current->text(0));
        // 设置对应颜色图标
        band->setIcon(icon[n]);
        // 保存波段号以供显示使用
        band->setData(Qt::UserRole, QVariant(band_index));
        ui->listWidget->addItem(band);

        // 应该设置波段所属文件，以确保三个波段节点同属一个文件节点，避免超限。
        // 不能简单判断范围，应该用队列判断三者一致。
        RGB_dataset.append(par_item->text(0));
    }
}

// 每次选择RGB按钮初始化列表
void MainWindow::on_radioButton_2_clicked()
{
    // 初始化列表
    ui->listWidget->clear();
    // 初始化已选波段数据集
    RGB_dataset.clear();
}

// 每次选择Gray按钮初始化列表
void MainWindow::on_radioButton_clicked()
{
    ui->listWidget->clear();
}

// 关闭显示窗口
void MainWindow::on_pushButton_2_clicked()
{
    destroyAllWindows();
}

// 关闭打开数据集
void MainWindow::on_actionCloseAll_triggered()
{
    // 调用GDALClose()关闭所有数据集
    int nDScount = 0;
    GDALDataset **openedDSList = GDALDataset::GetOpenDatasets(&nDScount);   // 当前打开数据集列表
    for (int i = 0; i < nDScount; ++i)
    {
        GDALClose(openedDSList[i]);
    }
    // 当前文件指针置为空
    cur_dataset = NULL;
    // 清空文件节点
    ui->treeWidget->clear();
    // 清空选择波段
    ui->listWidget->clear();
    // 关闭打开图像
    destroyAllWindows();
    // 当前文件状态栏置为空
    label_filename->setText("");
}

// 其他选项，用于测试。
void MainWindow::on_actionOptions_triggered()
{
    // 输出当前RGB波段对应数据集
    qDebug() << RGB_dataset;
}
