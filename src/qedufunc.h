#ifndef QEDUFUNC_H
#define QEDUFUNC_H

#include <QtWidgets>
#include <QMainWindow>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QtCharts>
#include <QPoint>
#include <QMouseEvent>
#include <QCursor>
#include "qimageadjustset.h"
#include "capturescreen.h"
#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printdialog)
#include <QPrintDialog>
#endif
#endif
QT_CHARTS_USE_NAMESPACE

namespace Ui {
class QEduFunc;
}
class QEduFunc : public QMainWindow
{
    Q_OBJECT

public:
    QImage sharpen(QImage * origin);
    QImage blur(QImage * origin);
    QImage *greyScale(QImage * origin);
    QImage *LightnessAdjust(int delta, QImage * origin);
    QImage *WarmAdjust(int delta, QImage * origin);
    QImage *CoolAdujust(int delta, QImage * origin);
    QImage *SaturationAdjust(int delta, QImage * origin);
    void ImageDrawLine(QImage *origin,QPoint Begin,QPoint End,int LineWidth);
    void ImageDrawRect(QImage *origin,QPoint Begin,QPoint End,int LineWidth);
    void ImageDrawElipse(QImage *origin,QPoint Begin,QPoint End,int LineWidth);
    void ImageDrawArrow(QImage *origin,QPoint Begin,QPoint End,int LineWidth);
    void CalcVertexes(QPoint start,QPoint end,QPoint *ArrowLeft,QPoint *ArrowRight);
    void PushImage(QImage image);
    bool PopImage (QImage *image);
    void ShowPaintColor(QColor color);
    QImage LogTransfor(QImage *originImage, double C, double V);
    QImage GamaTransfor(QImage *originImage, double C, double R);
    QImage EqualizationTransfor(QImage *originImage);              //直方图均衡
    QImage ImageEnhancementFuzzy(QImage *originImage);
    void DataTableUpdate(QImage CurrentImage);
    void EduFuncsFree(void);        //根据当前界面信息清空相应数据释放缓存
    explicit QEduFunc(QWidget *parent = nullptr);
    ~QEduFunc();

private:
    bool loadImage(const QString &fileName);
    bool SaveNewImage(void);
    bool ShowImage(QScrollArea *ShowArea,QWidget *AreaContent,QLabel *ShowLabel,const QImage Image,double *Scale);
    void GrayTransforInit(unsigned char GrayFuncIndex);
    void ToneAdjustInit(unsigned char Index);
    Ui::QEduFunc *ui;

protected:
    void resizeEvent(QResizeEvent* size);
    void wheelEvent(QWheelEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);


private slots:
    void on_actNew_triggered ();    //打开Action槽函数
    void on_actOpen_triggered ();    //打开Action槽函数
    void on_actClose_triggered();    //关闭Action槽函数
    void on_actEnlarge_triggered();  //放大Action槽函数
    void on_actShrink_triggered ();  //缩小Action槽函数
    void on_PixelData_clicked();     //像素数据 ToolButton槽函数
    void on_Histogram_clicked();     //直方图   ToolButton槽函数
    void on_Log_clicked();           //对数变换 ToolButton槽函数
    void ParameterSelec1Changed(void);
    void ParameterSelec2Changed(void);
    void on_SetGrayParameter_clicked();//灰度变换参数设置按钮
    void on_GrayCodeButton_clicked();  //灰度变换源代码查看按钮
    void on_Gamma_clicked();
    void on_Equalization_clicked();    //灰度直方图均衡按钮  ToolButton槽函数
    void on_ImageEnhancement_clicked();
    void on_RedDataTable_itemChanged(QTableWidgetItem *item);
    void on_BlueDataTable_itemChanged(QTableWidgetItem *item);
    void on_GreenDataTable_itemChanged(QTableWidgetItem *item);
    void on_actSave_triggered();
    void on_Horizontal_clicked();
    void on_MirrorVertically_clicked();
    void on_Size_clicked();
    void on_Spin_clicked();
    void on_Sharpen_clicked();
    void on_Fuzzy_clicked();
    void on_RGBToGray_clicked();
    void on_ToneParameterSelec1_valueChanged(int value);
    void on_ToneParameterSelec2_valueChanged(int value);
    void on_LightIntensity_clicked();
    void on_Warm_clicked();
    void on_Cool_clicked();
    void on_Saturation_clicked();
    void on_ToneAdjustCode_clicked();
    void on_SetToneAdjustParameter_clicked();

    void on_Brush_clicked();

    void on_Rectangle_toggled(bool checked);

    void on_PaintBrush_toggled(bool checked);

    void on_Circular_toggled(bool checked);

    void on_Arrow_toggled(bool checked);

    void on_PaintColor_clicked();

    void on_PaintRedValue_valueChanged(int RedValue);

    void on_PaintGreenValue_valueChanged(int GrennValue);

    void on_PaintBlueValue_valueChanged(int BlueValue);

private:
    QImage OriginalImage;           //打开的原始图像数据
    QImage PreImage;                //备份的图像数据
    QImage ProcessedImage;          //处理后的图像数据
    QString CurrentImagePath;
    QVector<QRgb> GrayTable;        //灰度表
    double OriginalScale  = 0.0;
    double PreImageScale  = 0.0;
    double ProcessedScale = 0.0;
    unsigned char NewImageFlag     = 0; //记录是否有新修改的图片需要保存
    unsigned char GrayTransforFlag = 0; //记录当前灰度变换的功能号 0:对数变换 1:伽马变换
    unsigned char ToneAdujustFlag  = 0; //记录当前色调变换的功能号
    unsigned char PaintStatus = 0; //绘图功能状态 启用 未启用
    bool PaintStart  = false; //绘图状态  正在绘图 没有绘图
    QPoint PaintBeginPos;
    QPoint PaintEndPos;
    QPainter CutPainter;
    QWidget *CutWidget;
    QCursor *PaintCursor;
    QColor PenColor;
    int PenWidth;
    QImage PaintHistory1;
    QImage PaintHistory2;
    QImage PaintHistory3;
    QImage PaintHistory4;
    QImage PaintHistory5;
};

#endif // QEDUFUNC_H
