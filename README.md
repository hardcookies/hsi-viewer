# hsi-viewer
## 功能介绍
基于Qt GUI利用GDAL和OpenCV库实现类似ENVI的高光谱图像显示，功能如下：
- 支持打开多个高光谱影像
- 影像列表显示打开的影像及其波段
- 选择波段并显示灰度或假彩色影像
- 波段选择列表实时显示选中波段

## 界面布局
软件主窗口包括工具栏、用于显示打开影像及其波段的QTreeWidget、选择显示模式的两个QRadioButton、一个显示已选波段的QListWidget和两个加载/关闭显示窗口的QPushButton。

界面及显示效果如下图所示：
![界面显示](http://imgs3.oss-cn-hongkong.aliyuncs.com/pigment/hsi-viewer.png)

## 使用方法
1. 点击工具栏"Open"选项，选择要打开的影像。
2. 选择图像显示模式，灰度图或假彩色。
3. 根据显示模式选择对应波段。
4. 点击"Load image"显示影像。
5. 可重复2-4。
6. 点击"Close"关闭影像窗口。

## 其他
Qt Creater中打开`GDAL_OpenCV_demo.pro`，查看项目。具体实现方法可以参考代码注释部分，希望大家批评指正！
