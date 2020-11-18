
#include "qedufunc.h"
#include "ui_qedufunc.h"

/************************************************************
*函数：QEduFunc()
*参数：无
*功能：初始化QEduFunc
*返回值：无
*备注：无
************************************************************/
QEduFunc::QEduFunc(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QEduFunc)
{
    ui->setupUi(this);
    ui->QtabEduFuncs->setCurrentWidget(ui->QTabImageBase);    //初始菜单栏显示主页图像基础
    ui->ShowBox->setCurrentWidget(ui->MainPage);              //初始图像显示区域为图片浏览模式  单图像显示
    ui->ImageBoxContents->resize(0,0);                        //初始化显示窗口大小
    ui->MainImageWidget ->resize(0,0);                        //初始化Label大小

    for(int i=0;i<256;i++)
        GrayTable.push_back(qRgb(i,i,i));                     //初始化颜色表
}
/************************************************************
*函数：~QEduFunc()
*参数：无
*功能：QEduFunc析构函数
*返回值：无
*备注：退出QEduFunc自动调用
************************************************************/
QEduFunc::~QEduFunc()
{
    delete ui;
}


/******************************************************槽函数区****************************************************************/
//Action槽函数
/************************************************************
*函数：on_actOpen_triggered()
*参数：无
*功能：Open Action 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_actNew_triggered()
{
    QImageAdjustSet *DialogAdjustSet = new QImageAdjustSet(this,0,500,500);
    int ret = DialogAdjustSet->exec();
    int NewImageWidth  = 0;
    int NewImageHeight = 0;
    if(ret == QImageAdjustSet::Accepted)   //正确设置了参数 （按下对话框确认按钮）
    {
        NewImageWidth  = DialogAdjustSet->ImageWidth();
        NewImageHeight = DialogAdjustSet->ImageHeight();
        DialogAdjustSet->close();          //关闭对话框
        OriginalImage = QImage(NewImageWidth,NewImageHeight,QImage::Format_ARGB32);
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    ui->ShowBox->setCurrentIndex(4);              //界面4 画笔等专用界面
    ui->PaintRedValue->setRange(0,255);           //设置SpinBox的范围  红色通道
    ui->PaintGreenValue->setRange(0,255);         //设置SpinBox的范围  绿色通道
    ui->PaintBlueValue->setRange(0,255);          //设置SpinBox的范围  蓝色通道
    ui->PaintRedValue->setSingleStep(5);          //步进值
    ui->PaintGreenValue->setSingleStep(5);        //步进值
    ui->PaintBlueValue->setSingleStep(5);         //步进值
    ui->LineWidth->setSingleStep(1);              //步进值
    ui->PaintRedValue->setValue(0);               //设置初值  红色通道
    ui->PaintGreenValue->setValue(255);             //设置初值  绿色通道
    ui->PaintBlueValue->setValue(0);              //设置初值  蓝色通道
    ui->LineWidth->setValue(2);                   //设置初值  画笔线宽
    ShowPaintColor(QColor(0,255,0));
    ui->PaintBrush->setChecked(true);             //初始化画笔模式
    ProcessedImage = OriginalImage;      //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    PaintHistory1 = ProcessedImage;      //历史备份
    ProcessedScale = 0;
    ShowImage(ui->PaintImageScroll,ui->PaintImageScrollContents,ui->PaintImageShow,
                            ProcessedImage,&ProcessedScale);  //显示图片
    ui->PaintImageShow->setCursor(Qt::CrossCursor);
    PaintStatus = 1;   //启动绘图功能->画笔功能
}

/************************************************************
*函数：on_actOpen_triggered()
*参数：无
*功能：Open Action 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_actOpen_triggered()
{
    static bool firstDialog = true;
    if((!ProcessedImage.isNull())&&(NewImageFlag)){  //当存在处理后图片时，询问后再释放
        if (QMessageBox::Yes == QMessageBox::question(this,
                                                      tr("保存图片"),
                                                      tr("有图片发生修改，是否另存图片后再打开新图片？"),
                                                      QMessageBox::Yes | QMessageBox::No,
                                                      QMessageBox::Yes)){
            while(true)
            {
                if(!SaveNewImage()){//保存图片
                    if (QMessageBox::Yes == QMessageBox::question(this,tr("保存图片"),
                                                                  tr("图片保存失败，再次尝试？"),
                                                                  QMessageBox::Yes | QMessageBox::No,
                                                                  QMessageBox::Yes));   //再次尝试保存
                    else break;  //放弃保存
                }
                else break;      //保存成功后离开
            }
        }
        QImage NullImage;            //定义一个空的QImage
        OriginalImage = NullImage;   //清空OriginalImage
        ui->MainImageWidget->clear();
        OriginalScale = 0.0;         //清除相关变量
        statusBar()->clearMessage(); //清空状态栏
        EduFuncsFree();              //界面切换前内存处理
        ui->ShowBox->setCurrentIndex(0);   //关闭图片后回到初始图片浏览界面
    }
    else if(!OriginalImage.isNull()){   //没有图片的时候才打开
            if (QMessageBox::Yes == QMessageBox::question(this,
                                                      tr(""),
                                                      tr("是否关闭当前图片并打开？"),
                                                      QMessageBox::Yes | QMessageBox::No,
                                                          QMessageBox::Yes)){
                QImage NullImage;            //定义一个空的QImage
                OriginalImage = NullImage;   //清空OriginalImage
                ui->MainImageWidget->clear();
                OriginalScale = 0.0;         //清除相关变量
                statusBar()->clearMessage(); //清空状态栏
                EduFuncsFree();              //界面切换前内存处理
                ui->ShowBox->setCurrentIndex(0);   //关闭图片后回到初始图片浏览界面
        }
        else return;    //拒绝关闭已打开图片不能打开新图片
    }
    QFileDialog dialog(this, tr("Open Image File"));
    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }
    dialog.setNameFilter(tr("All Files(*.*);;Windows BMP Images(*.bmp *.dib)"
                            ";;JPEG Image(*.jpeg *.jpg *.jpe);;PNG Image(*.png)"
                            ";;TIF Image(*.tif *.tiff);;Windows ICO Image(*.ico)"));
    while (dialog.exec() == QDialog::Accepted && !loadImage(dialog.selectedFiles().first())){}
}

/************************************************************
*函数：on_actSave_triggered()
*参数：无
*功能：Save Action 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_actSave_triggered()
{
    if(ProcessedImage.isNull())  //缓存为空的时候不响应
        return ;
    if(SaveNewImage()){          //保存文件
        NewImageFlag = 0;        //保存成功  清NewImageFlag 标志 无需要保存文件
    }
}

/************************************************************
*函数：on_actClose_triggered()
*参数：无
*功能：Close Action 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_actClose_triggered()
{
    if(OriginalImage.isNull())   //没有图片打开时  不响应
        return;
    if((!ProcessedImage.isNull())&&(NewImageFlag)){  //当存在处理后图片时，询问后再释放
        if (QMessageBox::Yes == QMessageBox::question(this,
                                                      tr("保存图片"),
                                                      tr("有图片发生修改，是否另存图片后再关闭？"),
                                                      QMessageBox::Yes | QMessageBox::No,
                                                      QMessageBox::Yes)){
            while(true)
            {
                if(!SaveNewImage()){//保存图片
                    if (QMessageBox::Yes == QMessageBox::question(this,tr("保存图片"),
                                                                  tr("图片保存失败，再次尝试？"),
                                                                  QMessageBox::Yes | QMessageBox::No,
                                                                  QMessageBox::Yes));   //再次尝试保存
                    else break;  //放弃保存
                }
                else break;      //保存成功后离开
            }
        }
        QImage NullImage;            //定义一个空的QImage
        OriginalImage = NullImage;   //清空OriginalImage
        ui->MainImageWidget->clear();
        OriginalScale = 0.0;         //清除相关变量
        statusBar()->clearMessage(); //清空状态栏
        EduFuncsFree();              //界面切换前内存处理
        ui->ShowBox->setCurrentWidget(ui->MainPage);   //关闭图片后回到初始图片浏览界面
        return ;
    }
    if (QMessageBox::Yes == QMessageBox::question(this,
                                                  tr(""),
                                                  tr("是否关闭当前文件？"),
                                                  QMessageBox::Yes | QMessageBox::No,
                                                  QMessageBox::Yes)){
        QImage NullImage;            //定义一个空的QImage
        OriginalImage = NullImage;   //清空OriginalImage
        ui->MainImageWidget->clear();
        OriginalScale = 0.0;         //清除相关变量
        statusBar()->clearMessage(); //清空状态栏
        EduFuncsFree();              //界面切换前内存处理
        ui->ShowBox->setCurrentWidget(ui->MainPage);   //关闭图片后回到初始图片浏览界面
    }
}

/************************************************************
*函数：on_actEnlarge_triggered()
*参数：无
*功能：Enlarge Action 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_actEnlarge_triggered()
{
    if(!OriginalImage.isNull())
    {
        if(OriginalScale <   5)   OriginalScale += 0.02;
        ShowImage(ui->ImageBox,ui->ImageBoxContents,
                  ui->MainImageWidget,OriginalImage,&OriginalScale);  //更新Labe组件  显示图片
    }
}

/************************************************************
*函数：on_actShrink_triggered()
*参数：无
*功能：Shrink Action 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_actShrink_triggered()
{
    if(!OriginalImage.isNull())
    {
        if(OriginalScale > 0.05)   OriginalScale -= 0.02;
        ShowImage(ui->ImageBox,ui->ImageBoxContents,
                  ui->MainImageWidget,OriginalImage,&OriginalScale);  //更新Labe组件  显示图片
    }
}

//图像基础Function 槽函数
/************************************************************
*函数：on_PixelData_clicked()
*参数：无
*功能：像素数据 ToolButton槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_PixelData_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    if((ui->MultFuncStack->currentIndex() == 0)&&(ui->ShowBox->currentIndex()==1)){   //当前已经处于像素数据显示界面了
        return ;   //不响应此按钮
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    ui->ShowBox->setCurrentWidget(ui->FuncImageToTab);       //主显示区 界面1 左右两块模式
    ui->MultFuncStack->setCurrentWidget(ui->PixelQWidget);   //显示像素数据显示界面
    ui->RGBtabWidget->setCurrentIndex(0);                    //Red通道显示界面
    PreImage = OriginalImage;                                //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->QPreImage,ui->QPreImageContents,ui->QPreImageShow,
                            PreImage,&PreImageScale);
    DataTableUpdate(PreImage);//刷新表格大小、显示像素数据
    if(PreImage.bitPlaneCount() == 8){//处理8位色图片写数据不兼容问题
        if(PreImage.allGray()) PreImage = OriginalImage.convertToFormat(QImage::Format_Indexed8);
        else PreImage = OriginalImage.convertToFormat(QImage::Format_RGB32);
    }
}

/************************************************************
*函数：on_Histogram_clicked()
*参数：无
*功能：图像直方图 ToolButton槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_Histogram_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    if((ui->ShowBox->currentIndex() == 1)&&(ui->MultFuncStack->currentIndex() == 1)){   //当前是已经处于直方图显示界面了
        return ;   //不响应此按钮
    }
    //上一界面功能处理 message

    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    ui->ShowBox->setCurrentWidget(ui->FuncImageToTab);       //主显示区 界面1 左右两块模式
    ui->MultFuncStack->setCurrentWidget(ui->HistWidget);   //显示像素数据显示界面
    PreImage = OriginalImage;     //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->QPreImage,ui->QPreImageContents,ui->QPreImageShow,
                            PreImage,&PreImageScale);  //显示备份图片

    QChart *RedChart   = new QChart();   //创建RedChar
    QChart *GreenChart = new QChart();   //创建GreenChar
    QChart *BlueChart  = new QChart();   //创建BuleChar
    QBarSeries *RedChartSeries   = new QBarSeries();   //
    QBarSeries *GreenChartSeries = new QBarSeries();   //
    QBarSeries *BlueChartSeries  = new QBarSeries();   //
    QBarSet *RedChartset   = new QBarSet("Red通道");
    QBarSet *GreenChartset = new QBarSet("Green通道");
    QBarSet *BlueChartset  = new QBarSet("Blue通道");
    QValueAxis *RedChartAxisX = new QValueAxis;     //创建X坐标轴
    QValueAxis *RedChartAxisY = new QValueAxis;     //创建Y坐标轴
    QValueAxis *GreenChartAxisX = new QValueAxis;     //创建X坐标轴
    QValueAxis *GreenChartAxisY = new QValueAxis;     //创建Y坐标轴
    QValueAxis *BlueChartAxisX = new QValueAxis;     //创建X坐标轴
    QValueAxis *BlueChartAxisY = new QValueAxis;     //创建Y坐标轴

    RedChart  ->setTitle("颜色直方图");       //设置四个图表的标题
    GreenChart->setTitle("颜色直方图");
    BlueChart ->setTitle("颜色直方图");
    RedChart  ->setAnimationOptions(QChart::AllAnimations);   //设置四个图表的显示动画
    GreenChart->setAnimationOptions(QChart::AllAnimations);
    BlueChart ->setAnimationOptions(QChart::AllAnimations);
    ui->RedHistChart  ->setChart(RedChart);  //将4个Chart添加到UI Designer 中设计的ChartView
    ui->GreenHistChart->setChart(GreenChart);
    ui->BlueHistChart ->setChart(BlueChart);
    ui->RedHistChart  ->setRenderHint(QPainter::Antialiasing);  //设置4个ChartView为防混叠显示
    ui->GreenHistChart->setRenderHint(QPainter::Antialiasing);
    ui->BlueHistChart ->setRenderHint(QPainter::Antialiasing);
   // ui->RGBHistChart  ->setRenderHint(QPainter::NonCosmeticDefaultPen);

    int Type;    //图片类型辅助变量
    if(PreImage.allGray()&&(PreImage.bitPlaneCount() == 8)) Type = 0;   //灰度图
    else Type = PreImage.bitPlaneCount();
    int PreImageHeight = PreImage.size().rheight();   //获取图片高度
    int PreImageWidth  = PreImage.size().rwidth();    //获取图片宽度
    int RedPixelCount  [256] = {0};
    int GreenPixelCount[256] = {0};
    int BluePixelCount [256] = {0};
    int RedPixelMax   = 0;     //记录各通道直方图最大值
    int GreenPixelMax = 0;
    int BluePixelMax  = 0;
    switch(Type)    //判断图片类型  根据图片类型统计直方图数据
    {
    case 0:
    case 1:{    //灰度图
        for(int i=0;i<PreImageHeight;i++)     //遍历 PreImageHeight*PreImageWidth 个像素
        {
            for(int j=0;j<PreImageWidth;j++){
                RedPixelCount[qRed(PreImage.pixel(j,i))] ++;//遍历每个像素，统计各像素值下的像素个数
                }
        }
        if(Type == 1){
            ui->RGBHistTab->setTabText(0,"单色通道");     //更改表头为->单色通道
            RedChartset->setColor(qRgb(0,0,0));          //设置数据颜色参数
        }
        else{
            ui->RGBHistTab->setTabText(0,"灰度通道");     //更改表头为->灰度通道
            RedChartset->setColor(qRgb(10,10,10));       //设置数据颜色参数
        }
        ui->RGBHistTab->setTabEnabled(0,true);       //开启需要显示的Tab
        ui->RGBHistTab->setTabEnabled(1,false);      //关闭多余的Tab
        ui->RGBHistTab->setTabEnabled(2,false);
        ui->RGBHistTab->setTabEnabled(3,false);
        RedChartSeries->append(RedChartset);         //将数据集加载入数据棒
        RedChart->addSeries(RedChartSeries);         //将数据棒添加到chart中
        RedChartSeries->setBarWidth(10);             //设置直方图数据棒宽度
        for(int i=0;i<256;i++)   //计算最大值
        {
             RedPixelMax   =qMax(RedPixelMax,RedPixelCount[i]);       //灰度 单色通道
             RedChartset->insert(i,RedPixelCount[i]);
        }
        RedChartAxisX->setRange(0,255);                       //X 像素值范围   固定值
        RedChart->setAxisX(RedChartAxisX,RedChartSeries);     //设置横坐标轴  0-255 像素值
        RedChartAxisY->setRange(0,int(RedPixelMax*1.2));      //Y 像素个数范围
        RedChart  ->setAxisY(RedChartAxisY,RedChartSeries);   //设置4个Chart的纵坐标轴 根据各直方图统计最大值做优化
        RedChart->legend()->setVisible(true);             //使能4个Chart显示
        RedChart->legend()->setAlignment(Qt::AlignBottom);
    }break;
     default:{    //RGB
        for(int i=0;i<PreImageHeight;i++)     //遍历 PreImageHeight*PreImageWidth 个像素
        {
            for(int j=0;j<PreImageWidth;j++){
                RedPixelCount[qRed(PreImage.pixel(j,i))] ++;//遍历每个像素，统计各像素值下的像素个数
                GreenPixelCount[qGreen(PreImage.pixel(j,i))] ++;
                BluePixelCount[qBlue(PreImage.pixel(j,i))] ++;
                }
        }
        ui->RGBHistTab->setTabText(0,"Red通道");      //恢复表头名称
        ui->RGBHistTab->setTabEnabled(0,true);       //开启需要显示的Tab
        ui->RGBHistTab->setTabEnabled(1,true);
        ui->RGBHistTab->setTabEnabled(2,true);
        ui->RGBHistTab->setTabEnabled(3,true);
        ui->RGBHistTab->setCurrentIndex(0);          //初始显示Red通道Chart
        RedChartset->setColor(qRgba(255,0,0,125));        //设置数据颜色参数
        GreenChartset->setColor(qRgba(0,255,0,125));
        BlueChartset->setColor(qRgba(0,0,255,125));
        RedChartSeries->setBarWidth(2);            //设置直方图数据棒宽度
        GreenChartSeries->setBarWidth(2);
        BlueChartSeries->setBarWidth(2);

        RedChartSeries->append(RedChartset);       //将数据集加载入数据棒
        GreenChartSeries->append(GreenChartset);
        BlueChartSeries->append(BlueChartset);
        RedChart->addSeries(RedChartSeries);         //将数据棒添加到4个chart中
        GreenChart->addSeries(GreenChartSeries);
        BlueChart->addSeries(BlueChartSeries);

        for(int i=0;i<256;i++)   //计算最大值
        {
             RedPixelMax   =qMax(RedPixelMax,RedPixelCount[i]);       //Red 通道
             RedChartset->insert(i,RedPixelCount[i]);
             GreenPixelMax =qMax(GreenPixelMax,GreenPixelCount[i]); //Green 通道
             GreenChartset->insert(i,GreenPixelCount[i]);
             BluePixelMax  =qMax(BluePixelMax,BluePixelCount[i]);      //Blue 通道
             BlueChartset->insert(i,BluePixelCount[i]);
        }
        RedChartAxisX->setRange(0,255);                       //X 像素值范围   固定值
        GreenChartAxisX->setRange(0,255);
        BlueChartAxisX->setRange(0,255);
        RedChart->setAxisX(RedChartAxisX,RedChartSeries);     //设置横坐标轴  0-255 像素值
        GreenChart->setAxisX(GreenChartAxisX,GreenChartSeries);
        BlueChart->setAxisX(BlueChartAxisX,BlueChartSeries);

        RedChartAxisY->setRange(0,int(RedPixelMax*1.2));      //Y 像素个数范围
        RedChart  ->setAxisY(RedChartAxisY,RedChartSeries);   //设置4个Chart的纵坐标轴 根据各直方图统计最大值做优化
        GreenChartAxisY->setRange(0,int(GreenPixelMax*1.2));
        GreenChart  ->setAxisY(GreenChartAxisY,GreenChartSeries);
        BlueChartAxisY->setRange(0,int(BluePixelMax*1.2));
        BlueChart  ->setAxisY(BlueChartAxisY,BlueChartSeries);

        RedChart->legend()->setVisible(true);             //使能4个Chart显示
        GreenChart->legend()->setVisible(true);
        BlueChart->legend()->setVisible(true);

        RedChart->legend()->setAlignment(Qt::AlignBottom);
        GreenChart->legend()->setAlignment(Qt::AlignBottom);
        BlueChart->legend()->setAlignment(Qt::AlignBottom);

       }break;
    }

}

//图像基础-->像素数据修改槽函数
/************************************************************
*函数：on_RedDataTable_itemChanged()
*参数：无
*功能：红色通道像素数据修改 Table槽函数
*返回值：无
*备注：红色通道、灰度、单色值共用一个Table
************************************************************/
void QEduFunc::on_RedDataTable_itemChanged(QTableWidgetItem *item)
{
    QString StrnewPixel = item->text();   //获取修改的数据
    int newPixel = StrnewPixel.toInt();   //转化为int 像素值
    if((newPixel>255)||(newPixel<0)) return;  //数据超出范围不予响应
    int PixelX = item->column();
    int PixelY = item->row();
    if((PreImage.bitPlaneCount() == 1)||(PreImage.bitPlaneCount() == 8))
    {
        if(newPixel)
            PreImage.setPixel(PixelX,PixelY,(PreImage.bitPlaneCount() == 8)?uint(newPixel):0);
        else
            PreImage.setPixel(PixelX,PixelY,(PreImage.bitPlaneCount() == 8)?uint(newPixel):1);
    }
    else{
        int GreenValue = qGreen(PreImage.pixel(PixelX,PixelY));
        int BlueValue  = qBlue (PreImage.pixel(PixelX,PixelY));
        PreImage.setPixel(PixelX,PixelY,qRgb(newPixel,GreenValue,BlueValue));
    }
    ShowImage(ui->QPreImage,ui->QPreImageContents,ui->QPreImageShow,
                            PreImage,&PreImageScale);      //更新图片
    ProcessedImage = PreImage;   //修改后图片复制到处理图片中
    NewImageFlag = 1;   //有新图片产生  离开需要保存
}

/************************************************************
*函数：on_GreenDataTable_itemChanged()
*参数：无
*功能：绿色通道像素数据修改 Table槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_GreenDataTable_itemChanged(QTableWidgetItem *item)
{
    QString StrnewPixel = item->text();   //获取修改的数据
    int newPixel = StrnewPixel.toInt();   //转化为int 像素值
    if((newPixel>255)||(newPixel<0)) return;  //数据超出范围不予响应
    int PixelX = item->column();
    int PixelY = item->row();    if(PreImage.bitPlaneCount() == 1)
    {
        if(newPixel == 1)
            PreImage.setPixel(PixelX,PixelY,0);
        else
            PreImage.setPixel(PixelX,PixelY,1);
    }
    else{
        int RedValue  = qRed (PreImage.pixel(PixelX,PixelY));
        int BlueValue = qBlue(PreImage.pixel(PixelX,PixelY));
        PreImage.setPixel(PixelX,PixelY,qRgb(RedValue,newPixel,BlueValue));
    }
    ShowImage(ui->QPreImage,ui->QPreImageContents,ui->QPreImageShow,
                            PreImage,&PreImageScale);      //更新图片
    ProcessedImage = PreImage;   //修改后图片复制到处理图片中
    NewImageFlag = 1;   //有新图片产生  离开需要保存
}

/************************************************************
*函数：on_BlueDataTable_itemChanged()
*参数：无
*功能：蓝色通道像素数据修改 Table槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_BlueDataTable_itemChanged(QTableWidgetItem *item)
{
    QString StrnewPixel = item->text();   //获取修改的数据
    int newPixel = StrnewPixel.toInt();   //转化为int 像素值
    if((newPixel>255)||(newPixel<0)) return;  //数据超出范围不予响应
    int PixelX = item->column();
    int PixelY = item->row();
    if(PreImage.bitPlaneCount() == 1)
    {
        if(newPixel)
            PreImage.setPixel(PixelX,PixelY,0);
        else
            PreImage.setPixel(PixelX,PixelY,1);
    }
    else{
        int RedValue   = qRed  (PreImage.pixel(PixelX,PixelY));
        int GreenValue = qGreen(PreImage.pixel(PixelX,PixelY));
        PreImage.setPixel(PixelX,PixelY,qRgb(RedValue,GreenValue,newPixel));
    }
    ShowImage(ui->QPreImage,ui->QPreImageContents,ui->QPreImageShow,
                            PreImage,&PreImageScale);      //更新图片
    ProcessedImage = PreImage;   //修改后图片复制到处理图片中
    NewImageFlag = 1;   //有新图片产生  离开需要保存
}

//几何变换Function 槽函数
/************************************************************
*函数：on_Size_clicked()
*参数：无
*功能：图像大小 ToolButton槽函数
*返回值：无
*备注：该功能根据选择的尺寸缩放原始图片
************************************************************/
void QEduFunc::on_Size_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    QImageAdjustSet *DialogAdjustSet = new QImageAdjustSet(this,0,OriginalImage.width(),OriginalImage.height());
    int ret = DialogAdjustSet->exec();
    int NewImageWidth  = 0;
    int NewImageHeight = 0;
    if(ret == QImageAdjustSet::Accepted)   //正确设置了参数 （按下对话框确认按钮）
    {
        NewImageWidth  = DialogAdjustSet->ImageWidth();
        NewImageHeight = DialogAdjustSet->ImageHeight();
        DialogAdjustSet->close();    //关闭对话框
    }
    else return;   //没有选择图像变化参数不予响应后续功能
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    ui->ShowBox->setCurrentIndex(1);  //主显示区 界面1 Func->Tab界面
    ui->MultFuncStack->setCurrentIndex(2);     //显示几何变换后图片显示界面(镜像功能)
    PreImage = OriginalImage;      //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->QPreImage,ui->QPreImageContents,ui->QPreImageShow,
                            PreImage,&PreImageScale);  //显示待处理原始图片（备份）
    ProcessedImage = PreImage.scaled(NewImageWidth,NewImageHeight,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    NewImageFlag   = 1;   //有新图片产生 关闭前询问是否保存
    ProcessedScale = 0.0;
    ShowImage(ui->GeomProcessedScroll,ui->GeomProcessedContents,ui->GeomProcessedShow,
                             ProcessedImage,&ProcessedScale); //处理完成后显示处理后的图片
}

/************************************************************
*函数：on_Spin_clicked()
*参数：无
*功能：图像旋转小 ToolButton槽函数
*返回值：无
*备注：该功能根据选择的角度旋转原始图片
************************************************************/
void QEduFunc::on_Spin_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    QImageAdjustSet *DialogAdjustSet = new QImageAdjustSet(this,1,0,0);  //旋转参数设置对话框
    int ret = DialogAdjustSet->exec();
    int SpinAngle  = 0;   //旋转角度  0-360°
    if(ret == QImageAdjustSet::Accepted)   //正确设置了参数 （按下对话框确认按钮）
    {
        SpinAngle  = DialogAdjustSet->ImageAngle();
        DialogAdjustSet->close();    //关闭对话框
    }
    else return;   //没有选择图像变化参数不予响应后续功能
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    ui->ShowBox->setCurrentIndex(1);  //主显示区 界面1 Func->Tab界面
    ui->MultFuncStack->setCurrentIndex(2);     //显示几何变换后图片显示界面(旋转功能)
    PreImage = OriginalImage;      //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->QPreImage,ui->QPreImageContents,ui->QPreImageShow,
                            PreImage,&PreImageScale);  //显示待处理原始图片（备份）
    QPixmap SpinImage = QPixmap::fromImage(PreImage);  //将图片转换为QPixmap形式
    QMatrix matrix;
    matrix.rotate(SpinAngle);
    ProcessedImage = SpinImage.transformed(matrix,Qt::SmoothTransformation).toImage(); //旋转图片
    NewImageFlag   = 1;   //有新图片产生 关闭前询问是否保存
    ProcessedScale = 0.0;
    ShowImage(ui->GeomProcessedScroll,ui->GeomProcessedContents,ui->GeomProcessedShow,
                             ProcessedImage,&ProcessedScale); //处理完成后显示处理后的图片
}

/************************************************************
*函数：on_Horizontal_clicked()
*参数：无
*功能：水平镜像 ToolButton槽函数
*返回值：无
*备注：该功能根据选择的角度旋转原始图片
************************************************************/
void QEduFunc::on_Horizontal_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    ui->ShowBox->setCurrentIndex(1);  //主显示区 界面1 Func->Tab界面
    ui->MultFuncStack->setCurrentIndex(2);     //显示几何变换后图片显示界面(镜像功能)
    PreImage = OriginalImage;      //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->QPreImage,ui->QPreImageContents,ui->QPreImageShow,
                            PreImage,&PreImageScale);  //显示待处理原始图片（备份）
    ProcessedImage = PreImage.mirrored(true,false); //水平镜像处理
    NewImageFlag   = 1;   //有新图片产生 关闭前询问是否保存
    ProcessedScale = 0.0;
    ShowImage(ui->GeomProcessedScroll,ui->GeomProcessedContents,ui->GeomProcessedShow,
                             ProcessedImage,&ProcessedScale); //处理完成后显示处理后的图片
}

/************************************************************
*函数：on_MirrorVertically_clicked()
*参数：无
*功能：垂直镜像 ToolButton槽函数
*返回值：无
*备注：该功能根据选择的角度旋转原始图片
************************************************************/
void QEduFunc::on_MirrorVertically_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    ui->ShowBox->setCurrentIndex(1);  //主显示区 界面1 Func->Tab界面
    ui->MultFuncStack->setCurrentIndex(2);     //显示几何变换后图片显示界面(镜像功能)
    PreImage = OriginalImage;      //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->QPreImage,ui->QPreImageContents,ui->QPreImageShow,
                            PreImage,&PreImageScale);  //显示待处理原始图片（备份）
    ProcessedImage = PreImage.mirrored(false,true); //垂直镜像处理
    NewImageFlag   = 1;   //有新图片产生 关闭前询问是否保存
    ProcessedScale = 0.0;
    ShowImage(ui->GeomProcessedScroll,ui->GeomProcessedContents,ui->GeomProcessedShow,
                             ProcessedImage,&ProcessedScale); //处理完成后显示处理后的图片
}

//灰度变换  ToolButton槽函数
/************************************************************
*函数：on_Log_clicked()
*参数：无
*功能：对数变换 ToolButton槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_Log_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    if((!OriginalImage.allGray())||(!(OriginalImage.bitPlaneCount()==8))){
        QMessageBox::warning(this, tr(""), tr("请打开可用灰度图！"));
        return ;
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    GrayTransforFlag = 0;  //标注进入灰度处理功能
    ui->ShowBox->setCurrentWidget(ui->GrayTransforShow);              //主显示区 界面2 灰度转换专用界面
    ui->GrayProcessedStack->setCurrentWidget(ui->ProcessedPage1);     //显示灰度变换后图片显示界面
    PreImage = OriginalImage;     //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->GrayOriginalScroll,ui->GrayOriginalScrollContents,ui->GrayOriginalShow,
                            PreImage,&PreImageScale);  //显示待处理灰度图片
    GrayTransforInit(0);          //灰度变换之对数变换初始化
    ProcessedImage = LogTransfor(&PreImage,ui->ParameterSelec1->value()/100.0,
                                 ui->ParameterSelec2->value());    //按参数进行对数变换
    ProcessedScale = 0.0;
    ui->GrayProcessedStack->setCurrentIndex(0);//设置界面到处理后图形显示界面
    ShowImage(ui->GrayProcessedScroll,ui->GrayProcessedContents,ui->GrayProcessedShow,
                             ProcessedImage,&ProcessedScale);//处理完成后显示处理后的图片
}

/************************************************************
*函数：on_Gamma_clicked()
*参数：无
*功能：GammaTransfor ToolBotton 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_Gamma_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    if((!OriginalImage.allGray())||(!(OriginalImage.bitPlaneCount()==8))){
        QMessageBox::warning(this, tr(""), tr("请打开可用灰度图！"));
        return ;
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    GrayTransforFlag = 1;  //标注进入灰度处理功能1->伽马变换
    ui->ShowBox->setCurrentWidget(ui->GrayTransforShow);              //主显示区 界面2 灰度转换专用界面
    ui->GrayProcessedStack->setCurrentWidget(ui->ProcessedPage1);     //显示灰度变换后图片显示界面
    PreImage = OriginalImage;     //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->GrayOriginalScroll,ui->GrayOriginalScrollContents,ui->GrayOriginalShow,
                            PreImage,&PreImageScale);  //显示待处理灰度图片
    GrayTransforInit(GrayTransforFlag);          //灰度变换之对数变换初始化
    ProcessedImage = GamaTransfor(&PreImage,ui->ParameterSelec1->value()/100.0,
                                 ui->ParameterSelec2->value());    //按参数进行对数变换
    ProcessedScale = 0.0;
    ui->GrayProcessedStack->setCurrentIndex(0);//设置界面到处理后图形显示界面
    ShowImage(ui->GrayProcessedScroll,ui->GrayProcessedContents,ui->GrayProcessedShow,
                             ProcessedImage,&ProcessedScale);//处理完成后显示处理后的图片

}

/************************************************************
*函数：on_Equalization_clicked()
*参数：无
*功能：Equalization ToolBotton 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_Equalization_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    if((!OriginalImage.allGray())||(!(OriginalImage.bitPlaneCount()==8))){
        QMessageBox::warning(this, tr(""), tr("请打开可用灰度图！"));
        return ;
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    GrayTransforFlag = 2;  //标注进入灰度处理功能2->直方图均衡
    ui->ShowBox->setCurrentWidget(ui->GrayTransforShow);              //主显示区 界面2 灰度转换专用界面
    ui->GrayProcessedStack->setCurrentWidget(ui->ProcessedPage1);     //显示灰度变换后图片显示界面
    PreImage = OriginalImage;     //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->GrayOriginalScroll,ui->GrayOriginalScrollContents,ui->GrayOriginalShow,
                            PreImage,&PreImageScale);  //显示待处理灰度图片
    GrayTransforInit(GrayTransforFlag);                //灰度变换之对数变换初始化
    ProcessedImage = EqualizationTransfor(&PreImage);  //对图片做直方图均衡处理
    ProcessedScale = 0.0;
    ui->GrayProcessedStack->setCurrentIndex(0);//设置界面到处理后图形显示界面
    ShowImage(ui->GrayProcessedScroll,ui->GrayProcessedContents,ui->GrayProcessedShow,
                             ProcessedImage,&ProcessedScale);//处理完成后显示处理后的图片
}

/************************************************************
*函数：on_ImageEnhancement_clicked()
*参数：无
*功能：ImageEnhancement ToolBotton 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_ImageEnhancement_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    if((!OriginalImage.allGray())||(!(OriginalImage.bitPlaneCount()==8))){
        QMessageBox::warning(this, tr(""), tr("请打开可用灰度图！"));
        return ;
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    GrayTransforFlag = 3;  //标注进入灰度处理功能2->直方图均衡
    ui->ShowBox->setCurrentWidget(ui->GrayTransforShow);              //主显示区 界面2 灰度转换专用界面
    ui->GrayProcessedStack->setCurrentWidget(ui->ProcessedPage1);     //显示灰度变换后图片显示界面
    PreImage = OriginalImage;     //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->GrayOriginalScroll,ui->GrayOriginalScrollContents,ui->GrayOriginalShow,
                            PreImage,&PreImageScale);  //显示待处理灰度图片
    GrayTransforInit(GrayTransforFlag);                //灰度变换之对数变换初始化
    ProcessedImage = ImageEnhancementFuzzy(&PreImage);  //对图片做直方图均衡处理
    ProcessedScale = 0.0;
    ui->GrayProcessedStack->setCurrentIndex(0);//设置界面到处理后图形显示界面
    ShowImage(ui->GrayProcessedScroll,ui->GrayProcessedContents,ui->GrayProcessedShow,
                             ProcessedImage,&ProcessedScale);//处理完成后显示处理后的图片
}

//对数变换  相关组件槽函数
/************************************************************
*函数：ParameterSelec1Changed()
*参数：无
*功能：ParameterSelec1 QSlider 的槽函数
*返回值：无
*备注：Slider控件  输入灰度变换的相关参数  根据灰度变换标志进行处理
************************************************************/
void QEduFunc::ParameterSelec1Changed(void)
{
    switch(GrayTransforFlag){
    case 0:{
        ProcessedImage = LogTransfor(&PreImage,ui->ParameterSelec1->value()/100.0,
                                     ui->ParameterSelec2->value());    //按参数进行对数变换
    }break;
    case 1:{
        ProcessedImage = GamaTransfor(&PreImage,ui->ParameterSelec1->value()/100.0,
                                     ui->ParameterSelec2->value());    //按参数进行Gama变换
    }break;
    default:;break;
    }
    ProcessedScale = 0.0;
    ui->GrayProcessedStack->setCurrentIndex(0);//设置界面到处理后图形显示界面
    ShowImage(ui->GrayProcessedScroll,ui->GrayProcessedContents,ui->GrayProcessedShow,
                             ProcessedImage,&ProcessedScale);//处理完成后显示处理后的图片
}

/************************************************************
*函数：ParameterSelec2Changed()
*参数：无
*功能：ParameterSelec2 QSPinBox 的槽函数
*返回值：无
*备注：SpinBox控件  输入灰度变换的相关参数  根据灰度变换标志进行处理
************************************************************/
void QEduFunc::ParameterSelec2Changed(void)
{
    switch(GrayTransforFlag){
    case 0:{
        ProcessedImage = LogTransfor(&PreImage,ui->ParameterSelec1->value()/100.0,
                                     ui->ParameterSelec2->value());    //按参数进行对数变换
    }break;
    case 1:{
        ProcessedImage = GamaTransfor(&PreImage,ui->ParameterSelec1->value()/100.0,
                                     ui->ParameterSelec2->value());    //按参数进行Gama变换
    }break;
    default:;break;
    }
    ProcessedScale = 0.0;
    ui->GrayProcessedStack->setCurrentIndex(0);//设置界面到处理后图形显示界面
    ShowImage(ui->GrayProcessedScroll,ui->GrayProcessedContents,ui->GrayProcessedShow,
                             ProcessedImage,&ProcessedScale);//处理完成后显示处理后的图片
}

/************************************************************
*函数：on_SetParameter_clicked()
*参数：无
*功能：SetParameter ToolButton 的槽函数
*返回值：无
*备注：新参数导入  开始按参数进行灰度变换处理
************************************************************/
void QEduFunc::on_SetGrayParameter_clicked()
{
    switch(GrayTransforFlag){
    case 0:{
        ProcessedImage = LogTransfor(&PreImage,ui->ParameterSelec1->value()/100.0,
                                     ui->ParameterSelec2->value());    //按参数进行对数变换
    }break;
    case 1:{
        ProcessedImage = GamaTransfor(&PreImage,ui->ParameterSelec1->value()/100.0,
                                     ui->ParameterSelec2->value());    //按参数进行Gama变换
    }break;
    default:;break;
    }
    ProcessedScale = 0.0;
    ui->GrayProcessedStack->setCurrentIndex(0);//设置界面到处理后图形显示界面
    ShowImage(ui->GrayProcessedScroll,ui->GrayProcessedContents,ui->GrayProcessedShow,
                             ProcessedImage,&ProcessedScale);//处理完成后显示处理后的图片
}

/************************************************************
*函数：on_GrayCodeButton_clicked()
*参数：无
*功能：GrayCodeButton ToolButton 的槽函数
*返回值：无
*备注：切换界面 显示源代码
************************************************************/
void QEduFunc::on_GrayCodeButton_clicked()
{
    ui->GrayProcessedStack->setCurrentIndex(1);   //切换处理后数据显示界面至源代码查看
}

//色调变换  ToolButton槽函数
/************************************************************
*函数：on_LightIntensity_clicked()
*参数：无
*功能：亮度调节 ToolButton 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_LightIntensity_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    ui->ShowBox->setCurrentIndex(3);              //界面3 色调处理专用界面
    ui->ToneProcessedStack->setCurrentIndex(0);   //显示色调处理后图片界面
    ui->ToneParameterSelec1->setRange(-100,255);  //设置Slider的范围
    ui->ToneParameterSelec2->setRange(-100,255);  //设置SpinBox的范围
    ui->ToneParameterSelec2->disconnect(SIGNAL(valueChanged(int)));  //disconnect  避免反复调用
    ui->ToneParameterSelec1->disconnect(SIGNAL(valueChanged(int)));  //disconnect  避免反复调用
    ui->ToneParameterSelec1->setValue(0);         //设置初值
    ui->ToneParameterSelec2->setValue(0);         //设置初值
    connect(ui->ToneParameterSelec1,SIGNAL(valueChanged(int)),this,SLOT(on_ToneParameterSelec1_valueChanged(int)));
    connect(ui->ToneParameterSelec2,SIGNAL(valueChanged(int)),this,SLOT(on_ToneParameterSelec2_valueChanged(int)));
    ui->ToneAdjustInfo->setText("亮度调节");       //设置提示信息
    ui->ToneParameterSelec1->setSingleStep(5);    //步进值
    ui->ToneParameterSelec2->setSingleStep(5);    //步进值
    PreImage = OriginalImage;      //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->ToneOriginalScroll,ui->ToneOriginalScrollContents,ui->ToneOriginalShow,
                            PreImage,&PreImageScale);  //显示待处理原始图片（备份）
    QImage *LightImage;
    LightImage = LightnessAdjust(0,&PreImage);         //初始不调节亮度
    ProcessedImage = *LightImage;                      //保存处理后图片
    delete LightImage;   //释放内存
    ToneAdujustFlag = 0; //色调调节->0号功能->亮度调节
    NewImageFlag   = 1;   //有新图片产生 关闭前询问是否保存
    ProcessedScale = 0.0;
    ShowImage(ui->ToneProcessedScroll,ui->ToneProcessedContents,ui->ToneProcessedShow,
                             ProcessedImage,&ProcessedScale); //处理完成后显示处理后的图片
}

/************************************************************
*函数：on_Warm_clicked()
*参数：无
*功能：暖色调调节 ToolButton 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_Warm_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    ui->ShowBox->setCurrentIndex(3);              //界面3 色调处理专用界面
    ui->ToneProcessedStack->setCurrentIndex(0);   //显示色调处理后图片界面
    ui->ToneParameterSelec1->setRange(-100,255);  //设置Slider的范围
    ui->ToneParameterSelec2->setRange(-100,255);  //设置SpinBox的范围
    ui->ToneParameterSelec2->disconnect(SIGNAL(valueChanged(int)));  //disconnect  避免反复调用
    ui->ToneParameterSelec1->disconnect(SIGNAL(valueChanged(int)));  //disconnect  避免反复调用
    ui->ToneParameterSelec1->setValue(0);         //设置初值
    ui->ToneParameterSelec2->setValue(0);         //设置初值
    connect(ui->ToneParameterSelec1,SIGNAL(valueChanged(int)),this,SLOT(on_ToneParameterSelec1_valueChanged(int)));
    connect(ui->ToneParameterSelec2,SIGNAL(valueChanged(int)),this,SLOT(on_ToneParameterSelec2_valueChanged(int)));
    ui->ToneAdjustInfo->setText("暖色调调节");       //设置提示信息
    ui->ToneParameterSelec1->setSingleStep(5);    //步进值
    ui->ToneParameterSelec2->setSingleStep(5);    //步进值
    PreImage = OriginalImage;      //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->ToneOriginalScroll,ui->ToneOriginalScrollContents,ui->ToneOriginalShow,
                            PreImage,&PreImageScale);  //显示待处理原始图片（备份）
    QImage *WarmImage;
    WarmImage = WarmAdjust(0,&PreImage);         //初始不调节亮度
    ProcessedImage = *WarmImage;                      //保存处理后图片
    delete WarmImage;      //释放内存
    ToneAdujustFlag = 1;   //色调调节->1号功能->暖色调节
    NewImageFlag    = 1;   //有新图片产生 关闭前询问是否保存
    ProcessedScale  = 0.0;
    ShowImage(ui->ToneProcessedScroll,ui->ToneProcessedContents,ui->ToneProcessedShow,
                             ProcessedImage,&ProcessedScale); //处理完成后显示处理后的图片
}

/************************************************************
*函数：on_Cool_clicked()
*参数：无
*功能：冷色调调节 ToolButton 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_Cool_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    ui->ShowBox->setCurrentIndex(3);              //界面3 色调处理专用界面
    ui->ToneProcessedStack->setCurrentIndex(0);   //显示色调处理后图片界面
    ui->ToneParameterSelec1->setRange(-100,255);  //设置Slider的范围
    ui->ToneParameterSelec2->setRange(-100,255);  //设置SpinBox的范围
    ui->ToneParameterSelec2->disconnect(SIGNAL(valueChanged(int)));  //disconnect  避免反复调用
    ui->ToneParameterSelec1->disconnect(SIGNAL(valueChanged(int)));  //disconnect  避免反复调用
    ui->ToneParameterSelec1->setValue(0);         //设置初值
    ui->ToneParameterSelec2->setValue(0);         //设置初值
    connect(ui->ToneParameterSelec1,SIGNAL(valueChanged(int)),this,SLOT(on_ToneParameterSelec1_valueChanged(int)));
    connect(ui->ToneParameterSelec2,SIGNAL(valueChanged(int)),this,SLOT(on_ToneParameterSelec2_valueChanged(int)));
    ui->ToneAdjustInfo->setText("冷色调调节");       //设置提示信息
    ui->ToneParameterSelec1->setSingleStep(5);    //步进值
    ui->ToneParameterSelec2->setSingleStep(5);    //步进值
    PreImage = OriginalImage;      //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->ToneOriginalScroll,ui->ToneOriginalScrollContents,ui->ToneOriginalShow,
                            PreImage,&PreImageScale);  //显示待处理原始图片（备份）
    QImage *CoolImage;
    CoolImage = CoolAdujust(0,&PreImage);         //初始不调节亮度
    ProcessedImage = *CoolImage;                      //保存处理后图片
    delete CoolImage;   //释放内存
    ToneAdujustFlag = 2;   //色调调节->2号功能->冷色调节
    NewImageFlag    = 1;   //有新图片产生 关闭前询问是否保存
    ProcessedScale  = 0.0;
    ShowImage(ui->ToneProcessedScroll,ui->ToneProcessedContents,ui->ToneProcessedShow,
                             ProcessedImage,&ProcessedScale); //处理完成后显示处理后的图片
}

/************************************************************
*函数：on_Saturation_clicked()
*参数：无
*功能：饱和度调节 ToolButton 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_Saturation_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    ui->ShowBox->setCurrentIndex(3);              //界面3 色调处理专用界面
    ui->ToneProcessedStack->setCurrentIndex(0);   //显示色调处理后图片界面
    ui->ToneParameterSelec1->setRange(-100,255);  //设置Slider的范围
    ui->ToneParameterSelec2->setRange(-100,255);  //设置SpinBox的范围
    ui->ToneParameterSelec2->disconnect(SIGNAL(valueChanged(int)));  //disconnect  避免反复调用
    ui->ToneParameterSelec1->disconnect(SIGNAL(valueChanged(int)));  //disconnect  避免反复调用
    ui->ToneParameterSelec1->setValue(0);         //设置初值
    ui->ToneParameterSelec2->setValue(0);         //设置初值
    connect(ui->ToneParameterSelec1,SIGNAL(valueChanged(int)),this,SLOT(on_ToneParameterSelec1_valueChanged(int)));
    connect(ui->ToneParameterSelec2,SIGNAL(valueChanged(int)),this,SLOT(on_ToneParameterSelec2_valueChanged(int)));
    ui->ToneAdjustInfo->setText("饱和度调节");       //设置提示信息
    ui->ToneParameterSelec1->setSingleStep(5);    //步进值
    ui->ToneParameterSelec2->setSingleStep(5);    //步进值
    PreImage = OriginalImage;      //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->ToneOriginalScroll,ui->ToneOriginalScrollContents,ui->ToneOriginalShow,
                            PreImage,&PreImageScale);  //显示待处理原始图片（备份）
    QImage *SaturationImage;
    SaturationImage = SaturationAdjust(0,&PreImage);         //初始不调节亮度
    ProcessedImage = *SaturationImage;                      //保存处理后图片
    delete SaturationImage;   //释放内存
    ToneAdujustFlag = 3;      //色调调节->3号功能->饱和度调节
    NewImageFlag    = 1;      //有新图片产生 关闭前询问是否保存
    ProcessedScale  = 0.0;
    ShowImage(ui->ToneProcessedScroll,ui->ToneProcessedContents,ui->ToneProcessedShow,
                             ProcessedImage,&ProcessedScale); //处理完成后显示处理后的图片
}

/************************************************************
*函数：on_Sharpen_clicked()
*参数：无
*功能：图片锐化 ToolButton 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_Sharpen_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    ui->ShowBox->setCurrentIndex(1);  //主显示区 界面1 Func->Tab界面
    ui->MultFuncStack->setCurrentIndex(2);     //显示几何变换后图片显示界面(镜像功能)
    PreImage = OriginalImage;      //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->QPreImage,ui->QPreImageContents,ui->QPreImageShow,
                            PreImage,&PreImageScale);  //显示待处理原始图片（备份）
    ProcessedImage = sharpen(&PreImage);    //锐化处理
    NewImageFlag   = 1;   //有新图片产生 关闭前询问是否保存
    ProcessedScale = 0.0;
    ShowImage(ui->GeomProcessedScroll,ui->GeomProcessedContents,ui->GeomProcessedShow,
                             ProcessedImage,&ProcessedScale); //处理完成后显示处理后的图片
}

/************************************************************
*函数：on_Fuzzy_clicked()
*参数：无
*功能：图片模糊化 ToolButton 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_Fuzzy_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    ui->ShowBox->setCurrentIndex(1);  //主显示区 界面1 Func->Tab界面
    ui->MultFuncStack->setCurrentIndex(2);     //显示几何变换后图片显示界面(镜像功能)
    PreImage = OriginalImage;         //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->QPreImage,ui->QPreImageContents,ui->QPreImageShow,
                            PreImage,&PreImageScale);  //显示待处理原始图片（备份）
    ProcessedImage = blur(&PreImage);    //模糊处理
    NewImageFlag   = 1;   //有新图片产生 关闭前询问是否保存
    ProcessedScale = 0.0;
    ShowImage(ui->GeomProcessedScroll,ui->GeomProcessedContents,ui->GeomProcessedShow,
                             ProcessedImage,&ProcessedScale); //处理完成后显示处理后的图片
}

/************************************************************
*函数：on_RGBToGray_clicked()
*参数：无
*功能：彩图转RGB ToolButton 的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_RGBToGray_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    ui->ShowBox->setCurrentIndex(1);  //主显示区 界面1 Func->Tab界面
    ui->MultFuncStack->setCurrentIndex(2);     //显示几何变换后图片显示界面(镜像功能)
    PreImage = OriginalImage;      //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    ShowImage(ui->QPreImage,ui->QPreImageContents,ui->QPreImageShow,
                            PreImage,&PreImageScale);  //显示待处理原始图片（备份）
    QImage *GrayImage;
    GrayImage = greyScale(&PreImage);   //变换为灰度图
    ProcessedImage = GrayImage->convertToFormat(QImage::Format_Indexed8);
    delete GrayImage;   //释放内存
    ProcessedImage.setColorTable(GrayTable);
    NewImageFlag   = 1;   //有新图片产生 关闭前询问是否保存
    ProcessedScale = 0.0;
    ShowImage(ui->GeomProcessedScroll,ui->GeomProcessedContents,ui->GeomProcessedShow,
                             ProcessedImage,&ProcessedScale); //处理完成后显示处理后的图片
}

//色调变换相关控件槽函数
/************************************************************
*函数：on_ToneParameterSelec1_valueChanged()
*参数：无
*功能：色调变换参数 Slider的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_ToneParameterSelec1_valueChanged(int value)
{
    ui->ToneParameterSelec2->disconnect(SIGNAL(valueChanged(int)));  //disconnect  避免反复调用
    ui->ToneParameterSelec2->setValue(value);
    ui->ToneProcessedStack->setCurrentIndex(0);   //切换处理后数据显示界面至图片查看
    switch(ToneAdujustFlag){
    case 0:{   //亮度调节
        QImage *LightImage;
        LightImage = LightnessAdjust(value,&PreImage);         //初始不调节亮度
        ProcessedImage = *LightImage;                      //保存处理后图片
        delete LightImage;   //释放内存
    }break;
    case 1:{   //暖色调
        QImage *WarmImage;
        WarmImage = WarmAdjust(value,&PreImage);         //初始不调节亮度
        ProcessedImage = *WarmImage;                      //保存处理后图片
        delete WarmImage;      //释放内存
    }break;
    case 2:{   //冷色调
        QImage *CoolImage;
        CoolImage = CoolAdujust(value,&PreImage);         //初始不调节亮度
        ProcessedImage = *CoolImage;                      //保存处理后图片
        delete CoolImage;   //释放内存
    }break;
    case 3:{   //饱和度
        QImage *SaturationImage;
        SaturationImage = SaturationAdjust(value,&PreImage);         //初始不调节亮度
        ProcessedImage = *SaturationImage;                      //保存处理后图片
        delete SaturationImage;   //释放内存
    }break;
    default:;break;
    }
    NewImageFlag    = 1;      //有新图片产生 关闭前询问是否保存
    ProcessedScale  = 0.0;
    ShowImage(ui->ToneProcessedScroll,ui->ToneProcessedContents,ui->ToneProcessedShow,
                             ProcessedImage,&ProcessedScale); //处理完成后显示处理后的图片
    connect(ui->ToneParameterSelec2,SIGNAL(valueChanged(int)),this,SLOT(on_ToneParameterSelec2_valueChanged(int)));
}

/************************************************************
*函数：on_ToneParameterSelec2_valueChanged()
*参数：无
*功能：色调变换参数 SpinBox的槽函数
*返回值：无
*备注：无
************************************************************/
void QEduFunc::on_ToneParameterSelec2_valueChanged(int value)
{

    ui->ToneParameterSelec1->disconnect(SIGNAL(valueChanged(int)));  //disconnect  避免反复调用
    ui->ToneParameterSelec1->setValue(value);
    ui->ToneProcessedStack->setCurrentIndex(0);   //切换处理后数据显示界面至图片查看
    switch(ToneAdujustFlag){
    case 0:{   //亮度调节
        QImage *LightImage;
        LightImage = LightnessAdjust(value,&PreImage);         //初始不调节亮度
        ProcessedImage = *LightImage;                      //保存处理后图片
        delete LightImage;   //释放内存
    }break;
    case 1:{   //暖色调
        QImage *WarmImage;
        WarmImage = WarmAdjust(value,&PreImage);         //初始不调节亮度
        ProcessedImage = *WarmImage;                      //保存处理后图片
        delete WarmImage;      //释放内存
    }break;
    case 2:{   //冷色调
        QImage *CoolImage;
        CoolImage = CoolAdujust(value,&PreImage);         //初始不调节亮度
        ProcessedImage = *CoolImage;                      //保存处理后图片
        delete CoolImage;   //释放内存
    }break;
    case 3:{   //饱和度
        QImage *SaturationImage;
        SaturationImage = SaturationAdjust(value,&PreImage);         //初始不调节亮度
        ProcessedImage = *SaturationImage;                      //保存处理后图片
        delete SaturationImage;   //释放内存
    }break;
    default:;break;
    }
    NewImageFlag    = 1;      //有新图片产生 关闭前询问是否保存
    ProcessedScale  = 0.0;
    ShowImage(ui->ToneProcessedScroll,ui->ToneProcessedContents,ui->ToneProcessedShow,
                             ProcessedImage,&ProcessedScale); //处理完成后显示处理后的图片
    connect(ui->ToneParameterSelec1,SIGNAL(valueChanged(int)),this,SLOT(on_ToneParameterSelec1_valueChanged(int)));
}

/************************************************************
*函数：on_ToneAdjustCode_clicked()
*参数：无
*功能：源码查看 ToolButton的槽函数
*返回值：无
*备注：切换到源码查看界面
************************************************************/
void QEduFunc::on_ToneAdjustCode_clicked()
{
    ui->ToneProcessedStack->setCurrentIndex(1);   //切换处理后数据显示界面至源代码查看
}

/************************************************************
*函数：on_SetToneAdjustParameter_clicked()
*参数：无
*功能：参数设定 ToolButton的槽函数
*返回值：无
*备注：备用按钮  无实质功能
************************************************************/
void QEduFunc::on_SetToneAdjustParameter_clicked()
{
    int value = ui->ToneParameterSelec1->value();
    ui->ToneProcessedStack->setCurrentIndex(0);   //切换处理后数据显示界面至图片查看
    switch(ToneAdujustFlag){
    case 0:{   //亮度调节
        QImage *LightImage;
        LightImage = LightnessAdjust(value,&PreImage);         //初始不调节亮度
        ProcessedImage = *LightImage;                      //保存处理后图片
        delete LightImage;   //释放内存
    }break;
    case 1:{   //暖色调
        QImage *WarmImage;
        WarmImage = WarmAdjust(value,&PreImage);         //初始不调节亮度
        ProcessedImage = *WarmImage;                      //保存处理后图片
        delete WarmImage;      //释放内存
    }break;
    case 2:{   //冷色调
        QImage *CoolImage;
        CoolImage = CoolAdujust(value,&PreImage);         //初始不调节亮度
        ProcessedImage = *CoolImage;                      //保存处理后图片
        delete CoolImage;   //释放内存
    }break;
    case 3:{   //饱和度
        QImage *SaturationImage;
        SaturationImage = SaturationAdjust(value,&PreImage);         //初始不调节亮度
        ProcessedImage = *SaturationImage;                      //保存处理后图片
        delete SaturationImage;   //释放内存
    }break;
    default:;break;
    }
    NewImageFlag    = 1;      //有新图片产生 关闭前询问是否保存
    ProcessedScale  = 0.0;
    ShowImage(ui->ToneProcessedScroll,ui->ToneProcessedContents,ui->ToneProcessedShow,
                             ProcessedImage,&ProcessedScale); //处理完成后显示处理后的图片
}
/*******************************************************************************************************************************/


/****************************************************图像数据功能实现*************************************************************/
/************************************************************
*函数：DataTableUpdate()
*参数：CurrentImage 传入图片
*功能：根据图片信息初始化表格
*返回值：无
*备注：能识别处理 1 8 16 32 位各种格式图片  处理显示图片像素数据
************************************************************/
void QEduFunc::DataTableUpdate(QImage CurrentImage)
{
    int i=0,j=0;
    int TableRow    = CurrentImage.size().rheight();    //通过图片尺寸得到表格行数
    int TableColumn = CurrentImage.size().rwidth();     //通过图片尺寸得到表格列数
    int Type;
    if(CurrentImage.allGray()&&(CurrentImage.bitPlaneCount() == 8)) Type = 0;   //灰度图
    else Type = CurrentImage.bitPlaneCount();
    QRgb Temp;   //临时变量  用于转换表格数据
    ui->RedDataTable->disconnect(SIGNAL(itemChanged(QTableWidgetItem *)));
    ui->GreenDataTable->disconnect(SIGNAL(itemChanged(QTableWidgetItem *)));
    ui->BlueDataTable->disconnect(SIGNAL(itemChanged(QTableWidgetItem *)));
    ui->AlphaDataTable->disconnect(SIGNAL(itemChanged(QTableWidgetItem *)));
    switch(Type)    //判断图片类型
    {
        case 0:{    //灰度图
            ui->RedDataTable->setRowCount(TableRow);       //设置红色通道数据表格的行数为图片高
            ui->RedDataTable->setColumnCount(TableColumn); //设置红色通道数据表格的列数为图片宽
            ui->RGBtabWidget->setTabText(0,"灰度值");       //修改红色通道Tab名字->灰度值
            for(i=0;i<TableRow;i++)
            {
                for(j=0;j<TableColumn;j++){
                    Temp = CurrentImage.pixel(j,i);
                    ui->RedDataTable->setItem(i,j,new QTableWidgetItem
                                              (QString::number(qRed(Temp))));
                    }
            }
            ui->RGBtabWidget->setTabEnabled(0,true);       //开启需要显示的Tab
            ui->RGBtabWidget->setTabEnabled(1,false);      //关闭多余的Tab
            ui->RGBtabWidget->setTabEnabled(2,false);
            ui->RGBtabWidget->setTabEnabled(3,false);
           }break;
        case 1:{    //单色位图
            ui->RedDataTable->setRowCount(TableRow);       //设置红色通道数据表格的行数为图片高
            ui->RedDataTable->setColumnCount(TableColumn); //设置红色通道数据表格的列数为图片宽
            ui->RGBtabWidget->setTabText(0,"逻辑值");       //修改红色通道Tab名字->逻辑值
            for(i=0;i<TableRow;i++)
            {
                for(j=0;j<TableColumn;j++){
                    if(qRed(CurrentImage.pixel(j,i))==255) //单色位图在QImage中会变成RGB相等的数值表
                        ui->RedDataTable->setItem(i,j,new QTableWidgetItem(QString{"1"}));   //恢复逻辑值
                    else
                        ui->RedDataTable->setItem(i,j,new QTableWidgetItem(QString{"0"}));
                    }
            }
            ui->RGBtabWidget->setTabEnabled(0,true);       //开启需要显示的Tab
            ui->RGBtabWidget->setTabEnabled(1,false);      //关闭多余的Tab
            ui->RGBtabWidget->setTabEnabled(2,false);
            ui->RGBtabWidget->setTabEnabled(3,false);
           }break;
        case 8:
        case 16:
        case 24:{   //RGB
            ui->RedDataTable->setRowCount(TableRow);         //设置红色通道数据表格的行数为图片高
            ui->RedDataTable->setColumnCount(TableColumn);   //设置红火色通道数据表格的列数为图片宽
            ui->GreenDataTable->setRowCount(TableRow);       //设置绿色通道数据表格的行数为图片高
            ui->GreenDataTable->setColumnCount(TableColumn); //设置绿色通道数据表格的列数为图片宽
            ui->BlueDataTable->setRowCount(TableRow);        //设置蓝色通道数据表格的行数为图片高
            ui->BlueDataTable->setColumnCount(TableColumn);  //设置蓝色通道数据表格的列数为图片宽
            ui->RGBtabWidget->setTabText(0,"Red通道");        //恢复红色通道Tab名字
            for(i=0;i<TableRow;i++)
            {
                for(j=0;j<TableColumn;j++){
                    Temp = CurrentImage.pixel(j,i);
                    ui->RedDataTable->setItem(i,j,new QTableWidgetItem
                                              (QString::number(qRed(Temp))));
                    ui->GreenDataTable->setItem(i,j,new QTableWidgetItem
                                              (QString::number(qGreen(Temp))));
                    ui->BlueDataTable->setItem(i,j,new QTableWidgetItem
                                              (QString::number(qBlue(Temp))));
                    }
            }
            ui->RGBtabWidget->setTabEnabled(0,true);         //开启需要显示的Tab
            ui->RGBtabWidget->setTabEnabled(1,true);
            ui->RGBtabWidget->setTabEnabled(2,true);
            ui->RGBtabWidget->setTabEnabled(3,false);        //关闭多余的Tab
           }break;
        default:{  //RGBA
            ui->RedDataTable->setRowCount(TableRow);          //设置红色通道数据表格的行数为图片高
            ui->RedDataTable->setColumnCount(TableColumn);    //设置红火色通道数据表格的列数为图片宽
            ui->GreenDataTable->setRowCount(TableRow);        //设置绿色通道数据表格的行数为图片高
            ui->GreenDataTable->setColumnCount(TableColumn);  //设置绿色通道数据表格的列数为图片宽
            ui->BlueDataTable->setRowCount(TableRow);         //设置蓝色通道数据表格的行数为图片高
            ui->BlueDataTable->setColumnCount(TableColumn);   //设置蓝色通道数据表格的列数为图片宽
            ui->AlphaDataTable->setRowCount(TableRow);        //设置透明通道数据表格的行数为图片高
            ui->AlphaDataTable->setColumnCount(TableColumn);  //设置透明通道数据表格的列数为图片宽
            ui->RGBtabWidget->setTabText(0,"Red通道");        //恢复红色通道Tab名字
            for(i=0;i<TableRow;i++)
            {
                for(j=0;j<TableColumn;j++){
                    Temp = CurrentImage.pixel(j,i);
                    ui->RedDataTable->setItem(i,j,new QTableWidgetItem
                                              (QString::number(qRed(Temp))));
                    ui->GreenDataTable->setItem(i,j,new QTableWidgetItem
                                              (QString::number(qGreen(Temp))));
                    ui->BlueDataTable->setItem(i,j,new QTableWidgetItem
                                              (QString::number(qBlue(Temp))));
                    ui->AlphaDataTable->setItem(i,j,new QTableWidgetItem
                                              (QString::number(qAlpha(Temp))));
                    }
            }
            ui->RGBtabWidget->setTabEnabled(0,true);          //开启需要显示的Tab
            ui->RGBtabWidget->setTabEnabled(1,true);
            ui->RGBtabWidget->setTabEnabled(2,true);
            ui->RGBtabWidget->setTabEnabled(3,true);
           }break;
    }
    connect(ui->RedDataTable,SIGNAL(itemChanged(QTableWidgetItem *)),this,SLOT(on_RedDataTable_itemChanged(QTableWidgetItem *)));
    connect(ui->GreenDataTable,SIGNAL(itemChanged(QTableWidgetItem *)),this,SLOT(on_GreenDataTable_itemChanged(QTableWidgetItem *)));
    connect(ui->BlueDataTable,SIGNAL(itemChanged(QTableWidgetItem *)),this,SLOT(on_BlueDataTable_itemChanged(QTableWidgetItem *)));
}
/*******************************************************************************************************************************/


/****************************************************灰度变换功能实现*************************************************************/
/************************************************************
*函数：GrayTransforInit()
*参数：GrayTransforFuncIndex 触发灰度变换功能的索引值 0->N
*功能：灰度变换相关功能初始化
*返回值：无
*备注：按功能初始化界面参数
************************************************************/
void QEduFunc::GrayTransforInit(unsigned char GrayFuncIndex)
{
    switch (GrayFuncIndex) {
    case 0:{     //对数变换 初始化数值选择条
        ui->ParameterSelec1->setEnabled(true);  //开启数值选择
        ui->ParameterSelec2->setEnabled(true);  //开启数值选择
        ui->SetGrayParameter->setEnabled(true); //开启参数修改
        ui->ParameterSelec1->setRange(0,500);   //设置对数变换的亮度范围
        ui->ParameterSelec2->setRange(0,50.0);  //对数值
        ui->ParameterSelec1->setSingleStep(20);//步进值
        ui->ParameterSelec2->setSingleStep(0.2);//步进值
        ui->ParameterSelec1->setValue(100);     //设定初始值
        ui->ParameterSelec2->setValue(3);
        connect(ui->ParameterSelec1,SIGNAL(valueChanged(int)),this,SLOT(ParameterSelec1Changed()));
        connect(ui->ParameterSelec2,SIGNAL(valueChanged(double)),this,SLOT(ParameterSelec2Changed()));
    }break;
    case 1:{     //伽马变换 初始化数值选择条
        ui->ParameterSelec1->setEnabled(true);  //开启数值选择
        ui->ParameterSelec2->setEnabled(true);  //开启数值选择
        ui->SetGrayParameter->setEnabled(true); //开启参数修改
        ui->ParameterSelec1->setRange(0,1000);   //设置伽马变换的范围
        ui->ParameterSelec2->setRange(0,20);    //指数值
        ui->ParameterSelec1->setSingleStep(20);//步进值
        ui->ParameterSelec2->setSingleStep(0.2);//步进值
        ui->ParameterSelec1->setValue(200);     //设定初始值
        ui->ParameterSelec2->setValue(1);
        connect(ui->ParameterSelec1,SIGNAL(valueChanged(int)),this,SLOT(ParameterSelec1Changed()));
        connect(ui->ParameterSelec2,SIGNAL(valueChanged(double)),this,SLOT(ParameterSelec2Changed()));
    }break;
    case 2:{     //直方图均衡
        ui->ParameterSelec1->setDisabled (true);  //关闭数值选择
        ui->ParameterSelec2->setDisabled (true);  //关闭数值选择
        ui->SetGrayParameter->setDisabled(true);  //关闭参数修改
    }break;
    case 3:{     //图像增强
        ui->ParameterSelec1->setDisabled (true);  //关闭数值选择
        ui->ParameterSelec2->setDisabled (true);  //关闭数值选择
        ui->SetGrayParameter->setDisabled(true);  //关闭参数修改
    }break;
    default:;break;
    }
}

/************************************************************
*函数：LogTransfor()
*参数：QImage image  待处理图片  double C 对数变换参数
*功能：对传入图片做对数变换处理
*返回值：变换完成后图片 QImage
*备注：
************************************************************/
QImage QEduFunc::LogTransfor(QImage *originImage, double C, double V)
{
    int width = originImage->width();     //获取图片尺寸
    int height = originImage->height();
    QImage newImage(width,height,QImage::Format_Indexed8); //构造一个参数与 originImage 相同的QImage newImage
    newImage.setColorTable(GrayTable);    //设置颜色表
    double GrayValue = 0;

    for(int i=0;i<height;i++){          //遍历所有像素矩阵
        for(int j=0;j<width;j++){
           GrayValue = qRed(originImage->pixel(j,i));  //获取像素数据
           GrayValue = GrayValue/255.0;                //归一化处理
           GrayValue = C*qLn(1+GrayValue*V)/qLn(V);    //进行对数变换
           GrayValue = 255*GrayValue;                  //恢复像素数据尺度
           if(GrayValue> 255) GrayValue = 255;
           else if(GrayValue < 0) GrayValue = 0;
           newImage.setPixel(j,i,uint(GrayValue));  //保存新的像素值  对于灰度图 RGB相等
        }
    }
    NewImageFlag = 1;      //有新图片产生，离开前需要保存
    return newImage;
}

/************************************************************
*函数：GamaTransfor()
*参数：image  待处理图片  C 亮度相关参数 R 对比度相关参数
*功能：对传入图片做伽马变换处理
*返回值：变换完成后图片 QImage
*备注：
************************************************************/
QImage QEduFunc::GamaTransfor(QImage *originImage, double C, double R)
{
    int width = originImage->width();     //获取图片尺寸
    int height = originImage->height();
    QImage newImage(width,height,QImage::Format_Indexed8); //构造一个参数与 originImage 相同的QImage newImage
    newImage.setColorTable(GrayTable);          //设置颜色表
    double GrayValue = 0;

    for(int i=0;i<height;i++){          //遍历所有像素矩阵
        for(int j=0;j<width;j++){
           GrayValue = qRed(originImage->pixel(j,i));  //获取像素数据
           GrayValue = GrayValue/255.0;                //归一化处理
           GrayValue = C*qPow(GrayValue,R);            //进行对数变换
           GrayValue = 255*GrayValue;                  //恢复像素数据尺度
           if(GrayValue> 255) GrayValue = 255;         //限制数据范围
           else if(GrayValue < 0) GrayValue = 0;
           newImage.setPixel(j,i,uint(GrayValue));  //保存新的像素值  对于灰度图 RGB相等
        }
    }
    NewImageFlag = 1;      //有新图片产生，离开前需要保存
    return newImage;
}

/************************************************************
*函数：EqualizationTransfor()
*参数：QImage image  待处理图片
*功能：对传入图片做直方图均衡处理
*返回值：变换完成后图片 QImage
*备注：
************************************************************/
QImage QEduFunc::EqualizationTransfor(QImage *originImage)
{
    int width  = originImage->width();     //获取图片尺寸
    int height = originImage->height();
    QImage newImage(width,height,QImage::Format_Indexed8); //构造一个参数与 originImage 相同的QImage newImage
    newImage.setColorTable(GrayTable);          //设置颜色表
    int GrayPixelCount          [256] = {0};    //存储图片各灰阶值个数
    double CumulativeProbability[256] = {0};    //存储累加概率

    for(int i=0;i<height;i++){          //遍历所有像素矩阵
        for(int j=0;j<width;j++){
            GrayPixelCount[qRed(originImage->pixel(j,i))] ++;  //统计图片各灰阶值个数
        }
    }
    int temp = 0;     //辅助变量
    double PixelSUM = width*height;           //总像素个数
    for(int i=0;i<256;i++){               //遍历所有像素矩阵
        for(int j=0;j<i;j++){
            temp += GrayPixelCount[j];    //统计灰阶累加个数
        }
        CumulativeProbability[i] = temp/PixelSUM;  //计算累加概率
        temp = 0;
    }
    for(int i=0;i<height;i++){             //遍历所有像素矩阵
        for(int j=0;j<width;j++){
            temp = qRed(originImage->pixel(j,i));     //像素值
            temp = int(CumulativeProbability[temp]*255);  //利用累加概率（灰阶系数）映射新像素值
            temp = qBound(0,temp,255);
            newImage.setPixel(j,i,uint(temp));  //保存新的像素值  对于灰度图 RGB相等
        }
    }
    NewImageFlag = 1;      //有新图片产生，离开前需要保存
    return newImage;
}

/************************************************************
*函数：ImageEnhancementFuzzy()
*参数：QImage image  待处理图片
*功能：对传入图片做基于规则的对比度增强
*返回值：变换完成后图片 QImage
*备注：实现效果并不明显？？？待优化
************************************************************/
QImage QEduFunc::ImageEnhancementFuzzy(QImage *originImage)
{
    int width = originImage->width();     //获取图片尺寸
    int height = originImage->height();
    QImage newImage(width,height,QImage::Format_Indexed8); //构造一个参数与 originImage 相同的QImage newImage
    newImage.setColorTable(GrayTable);          //设置颜色表
    int GrayPixelCount          [256] = {0};    //存储图片各灰阶值个数
    for(int i=0;i<height;i++){          //遍历所有像素矩阵
        for(int j=0;j<width;j++){
            GrayPixelCount[qRed(originImage->pixel(j,i))] ++;  //统计图片各灰阶值个数
        }
    }
    int PixelRangL = 0;
    int PixelRangH = 0;
    int PixelSUM = width*height;  //总像素个数
    int temp = 0;
    for(;temp<0.001*PixelSUM;PixelRangL++){                        //统计像素的分布区间低端像素
        temp += GrayPixelCount[PixelRangL];
    }
    for(PixelRangH = PixelRangL;temp<0.999*PixelSUM;PixelRangH++){ //统计像素的分布区间低端像素
        temp += GrayPixelCount[PixelRangH];
    }
    double FuuzyDark  [256] = {0};  //构建Dark Gray Bright隶属度函数
    double FuuzyGray  [256] = {0};
    double FuuzyBright[256] = {0};
    int PixelRangM = (PixelRangL+PixelRangH)/2;
    int PixelRang  = PixelRangH-PixelRangM;
    for(int i=0;i<256;i++){
        if(i<PixelRangL){
            FuuzyDark  [i] = 1;  //暗的更暗
            FuuzyGray  [i] = 0;
            FuuzyBright[i] = 0;
        }
        else if(i<PixelRangM){
            FuuzyDark  [i] = -1.0/PixelRang*i+PixelRangM;
            FuuzyGray  [i] =  1.0/PixelRang*i+PixelRangL;
            FuuzyBright[i] = 0;
        }
        else if(i<PixelRangH){
            FuuzyDark  [i] = 0;
            FuuzyGray  [i] = -1.0/PixelRang*i+PixelRangH;
            FuuzyBright[i] =  1.0/PixelRang*i+PixelRangM;
        }
        else{
            FuuzyDark  [i] = 0;
            FuuzyGray  [i] = 0;
            FuuzyBright[i] = 1;  //亮的更亮
        }
    }
    for(int i=0;i<height;i++){             //遍历所有像素矩阵
        for(int j=0;j<width;j++){
            temp = qRed(originImage->pixel(j,i));     //像素值
            temp = int(1.5*(FuuzyDark[temp]*PixelRangL+FuuzyGray[temp]*PixelRangM+FuuzyBright[temp]*PixelRangH)
                    /(FuuzyDark[temp]+FuuzyGray[temp]+FuuzyBright[temp]));
            temp = qBound(0,temp,255);
            newImage.setPixel(j,i,uint(temp));  //保存新的像素值  对于灰度图 RGB相等

        }
    }
    NewImageFlag = 1;      //有新图片产生，离开前需要保存
    return newImage;
}
/*******************************************************************************************************************************/


/********************************************************** 色调处理功能实现 *****************************************************/
/************************************************************
*函数：LightnessAdjust()
*参数：QImage image  待处理图片 delta 调节参数
*功能：对传入图片调节亮度
*返回值：变换完成后图片 QImage
*备注：无
************************************************************/
QImage * QEduFunc::LightnessAdjust(int delta, QImage * origin){
    int Type = 0;
    if(origin->allGray()&&(origin->bitPlaneCount() == 8)) Type = 8; //灰度图
    else if(origin->bitPlaneCount() == 1) Type = 1;                 //单值位图
    else Type = 0;
    *origin = origin->convertToFormat(QImage::Format_RGB32);//统一处理格式
    QImage * newImage = new QImage(origin->width(), origin->height(), QImage::Format_ARGB32);
    QColor oldColor;
    QColor newColor;
    int h,s,l;
    for(int x=0; x<newImage->width(); x++){
        for(int y=0; y<newImage->height(); y++){
            oldColor = QColor(origin->pixel(x,y));
            newColor = oldColor.toHsl();      //转到HSL空间中处理
            h = newColor.hue();
            s = newColor.saturation();
            l = newColor.lightness()+delta;   //在HSL空间中对亮度进行调节
            l = qBound(0, l, 255);            //调整至有效范围
            newColor.setHsl(h, s, l);
            newImage->setPixel(x, y, qRgb(newColor.red(), newColor.green(), newColor.blue()));
        }
    }
    if(Type == 8){   //灰度图
        QImage *GrayImage = greyScale(newImage);
        *newImage = *GrayImage;
        delete GrayImage;
    }
    else if (Type == 1){
        *newImage = newImage->convertToFormat(QImage::Format_Mono);      //转为单色图
    } //其他 不再做处理
    return newImage;
}

/************************************************************
*函数：WarmAdjust()
*参数：QImage image  待处理图片 delta 调节参数
*功能：对传入图片调节暖色调
*返回值：变换完成后图片 QImage
*备注：无
************************************************************/
QImage * QEduFunc::WarmAdjust(int delta, QImage * origin){
    *origin = origin->convertToFormat(QImage::Format_RGB32);//统一处理格式
    QImage *newImage = new QImage(origin->width(), origin->height(), QImage::Format_ARGB32);
    QColor oldColor;
    int r,g,b;
    for(int x=0; x<newImage->width(); x++){
        for(int y=0; y<newImage->height(); y++){
            oldColor = QColor(origin->pixel(x,y));
            r = oldColor.red() ;
            g = oldColor.green() ;
            b = oldColor.blue()-delta;
//            r = qBound(0, r, 255);
//            g = qBound(0, g, 255);
            b = qBound(0, b, 255);
            newImage->setPixel(x,y, qRgb(r,g,b));
        }
    }
    //暖色调不做处理  RGB才有暖色调
    return newImage;
}

/************************************************************
*函数：CoolAdujust()
*参数：QImage image  待处理图片 delta 调节参数
*功能：对传入图片调节冷色调
*返回值：变换完成后图片 QImage
*备注：无
************************************************************/
QImage * QEduFunc::CoolAdujust(int delta, QImage * origin){
    *origin = origin->convertToFormat(QImage::Format_RGB32);//统一处理格式
    QImage *newImage = new QImage(origin->width(), origin->height(), QImage::Format_ARGB32);
    QColor oldColor;
    int r,g,b;
    for(int x=0; x<newImage->width(); x++){
        for(int y=0; y<newImage->height(); y++){
            oldColor = QColor(origin->pixel(x,y));
            r = oldColor.red();
            g = oldColor.green();
            b = oldColor.blue()+delta;
            b = qBound(0, b, 255);
            newImage->setPixel(x,y, qRgb(r,g,b));
        }
    }
    //冷色调不做处理 RGB才有冷色调
    return newImage;
}

/************************************************************
*函数：SaturationAdjust()
*参数：QImage image  待处理图片 delta 调节参数
*功能：对传入图片调节饱和度
*返回值：变换完成后图片 QImage
*备注：无
************************************************************/
QImage * QEduFunc::SaturationAdjust(int delta, QImage * origin){
    *origin = origin->convertToFormat(QImage::Format_RGB32);//统一处理格式
    QImage * newImage = new QImage(origin->width(), origin->height(), QImage::Format_ARGB32);
    QColor oldColor;
    QColor newColor;
    int h,s,l;

    for(int x=0; x<newImage->width(); x++){
        for(int y=0; y<newImage->height(); y++){
            oldColor = QColor(origin->pixel(x,y));

            newColor = oldColor.toHsl();
            h = newColor.hue();
            s = newColor.saturation()+delta;
            l = newColor.lightness();
            s = qBound(0, s, 255);
            newColor.setHsl(h, s, l);
            newImage->setPixel(x, y, qRgb(newColor.red(), newColor.green(), newColor.blue()));
        }
    }
    //饱和度不做处理  RGB才有饱和度
    return newImage;
}

/************************************************************
*函数：greyScale()
*参数：QImage image  待处理图片
*功能：将传入图片转化为灰度图
*返回值：变换完成后图片 QImage
*备注：
************************************************************/
QImage * QEduFunc::greyScale(QImage * origin){
    *origin = origin->convertToFormat(QImage::Format_RGB32);//统一处理格式
    QImage * newImage = new QImage(origin->width(), origin->height(), QImage::Format_Indexed8);
    newImage->setColorTable(GrayTable);  //配置颜色表
    QColor oldColor;
    for(int x = 0; x<newImage->width(); x++){
        for(int y = 0; y<newImage->height(); y++){
            oldColor = QColor(origin->pixel(x,y));
            int average = int(0.2999*oldColor.red()+0.587*oldColor.green()+0.114*oldColor.blue());
            newImage->setPixel(x,y,uint(average));
        }
    }
    return newImage;
}

/************************************************************
*函数：blur()
*参数：QImage image  待处理图片
*功能：将传入图片做模糊化处理
*返回值：变换完成后图片 QImage
*备注：
************************************************************/
QImage QEduFunc::blur(QImage * origin){
    int Type = 0;
    if(origin->allGray()&&(origin->bitPlaneCount() == 8)) Type = 8; //灰度图
    else if(origin->bitPlaneCount() == 1) Type = 1;                 //单值位图
    else Type = 0;
    *origin = origin->convertToFormat(QImage::Format_RGB32);//统一处理格式
    QImage * newImage = new QImage(*origin);
    int kernel [5][5]= {{0,0,1,0,0},
                        {0,1,3,1,0},
                        {1,3,7,3,1},
                        {0,1,3,1,0},
                        {0,0,1,0,0}};
    int kernelSize = 5;
    int sumKernel = 27;
    int r,g,b;
    QColor color;
    for(int x=kernelSize/2; x<newImage->width()-(kernelSize/2); x++){
        for(int y=kernelSize/2; y<newImage->height()-(kernelSize/2); y++){
            r = 0;
            g = 0;
            b = 0;
            for(int i = -kernelSize/2; i<= kernelSize/2; i++){
                for(int j = -kernelSize/2; j<= kernelSize/2; j++){
                    color = QColor(origin->pixel(x+i, y+j));
                    r += color.red()*kernel[kernelSize/2+i][kernelSize/2+j];
                    g += color.green()*kernel[kernelSize/2+i][kernelSize/2+j];
                    b += color.blue()*kernel[kernelSize/2+i][kernelSize/2+j];
                }
            }
            r = qBound(0, r/sumKernel, 255);
            g = qBound(0, g/sumKernel, 255);
            b = qBound(0, b/sumKernel, 255);
            newImage->setPixel(x,y, qRgb(r,g,b));
        }
    }
    if(Type == 8){   //灰度图
        QImage *GrayImage = greyScale(newImage);   //转为灰度图
        *newImage = *GrayImage;
        delete GrayImage;
    }
    else if (Type == 1){
        *newImage = newImage->convertToFormat(QImage::Format_Mono);      //转为单色图
    } //其他 不再做处理
    return *newImage;
}

/************************************************************
*函数：sharpen()
*参数：QImage image  待处理图片
*功能：对传入图片做锐化化处理
*返回值：变换完成后图片 QImage
*备注：突出图片的轮廓感
************************************************************/
QImage QEduFunc::sharpen(QImage * origin){
    int Type = 0;
    if(origin->allGray()&&(origin->bitPlaneCount() == 8)) Type = 8; //灰度图
    else if(origin->bitPlaneCount() == 1) Type = 1;                 //单值位图
    else Type = 0;
    *origin = origin->convertToFormat(QImage::Format_RGB32);//统一处理格式
    QImage * newImage = new QImage(* origin);
    int kernel [3][3]= {{ 0,-1, 0},
                        {-1, 5,-1},
                        { 0,-1, 0}};
    int kernelSize = 3;
    int sumKernel = 1;
    int r,g,b;
    QColor color;
    for(int x=kernelSize/2; x<newImage->width()-(kernelSize/2); x++){
        for(int y=kernelSize/2; y<newImage->height()-(kernelSize/2); y++){

            r = 0;
            g = 0;
            b = 0;

            for(int i = -kernelSize/2; i<= kernelSize/2; i++){
                for(int j = -kernelSize/2; j<= kernelSize/2; j++){
                    color = QColor(origin->pixel(x+i, y+j));
                    r += color.red()*kernel[kernelSize/2+i][kernelSize/2+j];
                    g += color.green()*kernel[kernelSize/2+i][kernelSize/2+j];
                    b += color.blue()*kernel[kernelSize/2+i][kernelSize/2+j];
                }
            }
            r = qBound(0, r/sumKernel, 255);
            g = qBound(0, g/sumKernel, 255);
            b = qBound(0, b/sumKernel, 255);
            newImage->setPixel(x,y, qRgb(r,g,b));
        }
    }
    if(Type == 8){   //灰度图
        QImage *GrayImage = greyScale(newImage);
        *newImage = *GrayImage;
        delete GrayImage;
    }
    else if (Type == 1){
        *newImage = newImage->convertToFormat(QImage::Format_Mono);      //转为单色图
    } //其他 不再做处理
    return *newImage;
}
/*******************************************************************************************************************************/


/******************************************************辅助函数******************************************************************/
//OpenAction 辅助函数
/************************************************************
*函数：loadImage()
*参数：无
*功能：加载图片  显示到界面1的Labe中
*返回值：无
*备注：打开一张图片 显示到主显示区域 图片基本浏览实现
************************************************************/
bool QEduFunc::loadImage(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);   //启动自动转换
    const QImage newImage = reader.read();
    if (newImage.isNull()){          //如果图片数据为空，错误弹窗
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }
    OriginalImage = newImage;             //更新系统当前图像
    ShowImage(ui->ImageBox,ui->ImageBoxContents,ui->MainImageWidget,OriginalImage,&OriginalScale);    //更新Labe组件  显示图片
    setWindowFilePath(fileName);          //保存当前图片路径，方便下次打开相同路径文件
    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
        .arg(QDir::toNativeSeparators(fileName)).arg(newImage.width()).arg(newImage.height()).arg(newImage.bitPlaneCount());
    statusBar()->showMessage(message);    //在状态栏显示当前打开图片信息
    CurrentImagePath = QDir::toNativeSeparators(fileName);  //保存当前路径信息  保存图片用
    return true;
}

/*****************************************************************
*函数：ShowImage()
*参数：*ShowArea     要显示的QScrollArea位置
*     *AreaContent  QScrollArea位置下的QWidget类Content成员
*     *ShowLabe     QLabel控件显示图片
*     Image         QImage类数据
*     *Scale        当前类数据对应的缩放比例
*功能：加载图片  显示到界面1的Label中
*返回值： true or false
*备注：QLabel 必须配置为ScaledContents：true允许内容自动缩放Scale才能生效
*****************************************************************/
bool QEduFunc::ShowImage(QScrollArea *ShowArea,QWidget *AreaContent,QLabel *ShowLabel,
                        const QImage Image,double *Scale)
{
    //根据图片信息计算缩放比例以及显示区域大小
    if(*Scale == 0.0){   //当第一次显示这个区域时  计算一个适当的值
        QSize ShowAreaSize = ShowArea->frameGeometry().size();     //获取显示区域大小
        *Scale = qMin((ShowAreaSize.rheight()*0.618)/Image.size().rheight(),
                     (ShowAreaSize.rwidth() *0.618)/Image.size().rwidth());   //根据显示区域大小计算一个合适的缩放比例
    }
    AreaContent->resize((*Scale)*Image.size());      //重新设定显示窗口大小
    ShowLabel  ->resize  ((*Scale)*Image.size());      //重新设定显示Label大小
    ShowLabel->setPixmap(QPixmap::fromImage(Image)); //显示图片
    return true;
}

//Save Function 辅助函数
/************************************************************
*函数：SaveNewImage()
*参数：无
*功能：调用文件对话框 获取保存文件的格式与文件名
*返回值：无
*备注：支持bmp jpg png tif等格式文件
************************************************************/
bool QEduFunc::SaveNewImage(void){
    QString DlgTitle = "Save as new Image";  //对话框标题
    QString FileFilter = "Windows BMP Images(*.bmp *.dib)"
                         ";;JPEG Image(*.jpeg *.jpg *.jpe);;PNG Image(*.png)"
                         ";;TIF Image(*.tif *.tiff)";
    QString SaveFileName = QFileDialog::getSaveFileName(this,DlgTitle,CurrentImagePath,FileFilter);  //打开资源选择对话框  等待文件名
    if(SaveFileName.isEmpty()){   //文件名为空  结束
        return false;             //保存失败
    }
    if(ProcessedImage.save(SaveFileName,"",-1)){  //保存文件并返回保存结果
        QMessageBox::information(this, tr("Save"), tr("保存成功！"));
        NewImageFlag = 0;   //清空标志 没有需要保存的文件
        return true;
    }
    else{
        QMessageBox::warning(this, tr("Save"), tr("保存失败！"));
        return false;
    }
}

/************************************************************
*函数：EduFuncsFree()
*参数：无
*功能：释放当前界面的相关内存占用
*返回值：无
*备注：每次界面切换时均需要调用
************************************************************/
void QEduFunc::EduFuncsFree(void)
{
    switch(ui->ShowBox->currentIndex()){  //获取顶层界面索引值 根据界面坐标处理
    case 0:{          //由主界面转入功能界面
        OriginalScale = 0.0;           //清空缩放比例
        ui->MainImageWidget->clear();  //清空图片显示Lable
    }break;
    case 1:{          //ShowBox的界面1为左半显示图片 右半显示处理结果
        switch(ui->MultFuncStack->currentIndex())
        {
        case 0:{      //退出像素数据功能
            QImage NullImage;
            ProcessedImage = NullImage;  //利用内部QImage构造函数构造空QImage释放其他Image占用内存
            PreImage       = NullImage;  //内存释放后保留原变量
            NewImageFlag = 0;
            PreImageScale = 0.0;
            OriginalScale = 0.0;
            ui->QPreImageShow->clear();   //清空图片显示区域
            ui->RedDataTable->clear();    //清空Red通道Table
            ui->BlueDataTable->clear();   //清空Blue通道Table
            ui->GreenDataTable->clear();  //清空Green通道Table
            ui->AlphaDataTable->clear();  //清空Alpha通道Table
        }break;
        case 1:{      //退出直方图查看功能
            PreImageScale = 0.0;
            OriginalScale = 0.0;
            ui->QPreImageShow->clear();   //清空图片显示区域.QImage NullImage;
            QImage NullImage;             //利用内部QImage构造函数构造空QImage释放其他Image占用内存
            PreImage       = NullImage;   //内存释放后保留原变量
        }break;
        case 2:{      //退出几何处理结果显示功能
            NewImageFlag = 0;
            PreImageScale = 0.0;
            OriginalScale = 0.0;
            ProcessedScale= 0.0;
            ui->GeomProcessedShow->clear();   //清空图片显示区域.QImage NullImage;
            QImage NullImage;             //利用内部QImage构造函数构造空QImage释放其他Image占用内存
            PreImage       = NullImage;   //内存释放后保留原变量
            ProcessedImage = NullImage;
        }break;
        default:;break;
        }
    }break;
    case 2:{     //--->GrayTransfor功能界面
        ui->GrayOriginalShow->clear();    //清空显示区域
        ui->GrayProcessedShow->clear();
        QImage NullImage;
        ProcessedImage = NullImage;  //利用内部QImage构造函数构造空QImage释放其他Image占用内存
        PreImage       = NullImage;  //内存释放后保留原变量
        NewImageFlag   =   0;
        PreImageScale  = 0.0;
        OriginalScale  = 0.0;
        ProcessedScale = 0.0;
    }break;
    case 3:{     //--->ToneTransfor功能界面
        ui->ToneOriginalShow->clear();    //清空显示区域
        ui->ToneProcessedShow->clear();
        QImage NullImage;
        ProcessedImage = NullImage;  //利用内部QImage构造函数构造空QImage释放其他Image占用内存
        PreImage       = NullImage;  //内存释放后保留原变量
        ToneAdujustFlag =   0;
        NewImageFlag    =   0;
        PreImageScale   = 0.0;
        OriginalScale   = 0.0;
        ProcessedScale  = 0.0;
    }break;
    case 4:{     //--->Painter功能界面
        ui->PaintImageShow->clear();    //清空显示区域
        QImage NullImage;
        ProcessedImage = NullImage;  //利用内部QImage构造函数构造空QImage释放其他Image占用内存
        PreImage       = NullImage;  //内存释放后保留原变量
        PaintStatus = 0; //绘图功能状态 启用 未启用
        PaintStart  = false;           //绘图状态
        PaintHistory1 = NullImage;
        PaintHistory2 = NullImage;
        PaintHistory3 = NullImage;
        PaintHistory4 = NullImage;
        PaintHistory5 = NullImage;
        NewImageFlag    =   0;
        PreImageScale   = 0.0;
        OriginalScale   = 0.0;
        ProcessedScale  = 0.0;
    }break;
    default:;break;
    }
}
/*******************************************************************************************************************************/


/******************************************************重构系统内部事件函数*******************************************************/
/************************************************************
*函数：resizeEvent()
*参数：*size 窗口尺寸发生改变事件的参数类
*功能：重构尺寸修改事件 实现在窗口尺寸发生变化后图片获得最优显示效果
*返回值：无
*备注：根据页面索引刷新图片显示比例  获得较好视野区域
************************************************************/
void QEduFunc::resizeEvent(QResizeEvent *size)
{
    size->ignore();
    int StackedIndex = ui->ShowBox->currentIndex();   //获取当前的堆栈页面索引值

    switch (StackedIndex) {    //不同的页面对滚轮的操作有区分
    case 0:{                   //index = 0 -->MainPage
        QSize ShowAreaSize = ui->MainPage->frameGeometry().size();     //获取显示区域大小
        if(!OriginalImage.isNull()){                                   //OriginalImage  图片数据存在时才进行此操作
            OriginalScale = qMin((ShowAreaSize.rheight()*0.618)/OriginalImage.size().rheight(),
                         (ShowAreaSize.rwidth() *0.618)/OriginalImage.size().rwidth());   //根据显示区域大小计算一个合适的缩放比例
            ShowImage(ui->ImageBox,ui->ImageBoxContents,ui->MainImageWidget,
                      OriginalImage,&OriginalScale);    //更新Labe组件  显示图片
        }

    }break;
    case 1:{                   //index = 1 -->FuncImageToTab
        QSize ShowAreaSize = ui->QPreImage->frameGeometry().size();     //获取显示区域大小
        if(!PreImage.isNull()){                                    //OriginalImage  图片数据存在时才进行此操作
            PreImageScale = qMin((ShowAreaSize.rheight()*0.618)/OriginalImage.size().rheight(),
                         (ShowAreaSize.rwidth() *0.618)/OriginalImage.size().rwidth());   //根据显示区域大小计算一个合适的缩放比例
            ShowImage(ui->QPreImage,ui->QPreImageContents,ui->QPreImageShow,
                                     PreImage,&PreImageScale);     //更新Labe组件  显示图片
        }
        if(ui->MultFuncStack->currentIndex() == 2){    //有几何变换后图片正在显示
            ProcessedScale = 0.0;                      //初始化显示比例  显示函数会根据窗口尺寸重新计算显示比例
            ShowImage(ui->GeomProcessedScroll,ui->GeomProcessedContents,ui->GeomProcessedShow,
                                     ProcessedImage,&ProcessedScale);     //更新Labe组件  显示图片
        }
    }break;
    case 2:{                  //Index = 2 -->GrayTransforFunc
        if(!PreImage.isNull()){  //图像数据存在时才进行此操作
            QSize ShowAreaSize = ui->GrayOriginalScroll->frameGeometry().size();        //获取显示区域大小
            PreImageScale = qMin((ShowAreaSize.rheight()*0.618)/PreImage.size().rheight(),
                         (ShowAreaSize.rwidth() *0.618)/PreImage.size().rwidth()); //根据显示区域大小计算一个合适的缩放比例
            ShowImage(ui->GrayOriginalScroll,ui->GrayOriginalScrollContents,ui->GrayOriginalShow,
                                     PreImage,&PreImageScale);     //更新Labe组件  显示图片
        }
        if((!ProcessedImage.isNull())                        //有图像数据
           &&(ui->GrayProcessedStack->currentIndex() == 0)){ //显示区域处于图像显示界面
            QSize ShowAreaSize = ui->GrayProcessedScroll->frameGeometry().size();        //获取显示区域大小
            ProcessedScale = qMin((ShowAreaSize.rheight()*0.618)/ProcessedImage.size().rheight(),
                         (ShowAreaSize.rwidth() *0.618)/ProcessedImage.size().rwidth()); //根据显示区域大小计算一个合适的缩放比例
            ShowImage(ui->GrayProcessedScroll,ui->GrayProcessedContents,ui->GrayProcessedShow,
                                     ProcessedImage,&ProcessedScale);//按新比例显示图片
        }
    }break;
    case 3:{                  //Index = 3 -->ToneAdjust
        if(!PreImage.isNull()){  //图像数据存在时才进行此操作
            QSize ShowAreaSize = ui->ToneOriginalScroll->frameGeometry().size();        //获取显示区域大小
            PreImageScale = qMin((ShowAreaSize.rheight()*0.618)/PreImage.size().rheight(),
                         (ShowAreaSize.rwidth() *0.618)/PreImage.size().rwidth()); //根据显示区域大小计算一个合适的缩放比例
            ShowImage(ui->ToneOriginalScroll,ui->ToneOriginalScrollContents,ui->ToneOriginalShow,
                                     PreImage,&PreImageScale);     //更新Labe组件  显示图片
        }
        if((!ProcessedImage.isNull())                        //有图像数据
           &&(ui->ToneProcessedStack->currentIndex() == 0)){ //显示区域处于图像显示界面
            QSize ShowAreaSize = ui->ToneProcessedScroll->frameGeometry().size();        //获取显示区域大小
            ProcessedScale = qMin((ShowAreaSize.rheight()*0.618)/ProcessedImage.size().rheight(),
                         (ShowAreaSize.rwidth() *0.618)/ProcessedImage.size().rwidth()); //根据显示区域大小计算一个合适的缩放比例
            ShowImage(ui->ToneProcessedScroll,ui->ToneProcessedContents,ui->ToneProcessedShow,
                                     ProcessedImage,&ProcessedScale);//按新比例显示图片
        }
    }break;
    case 4:{                  //Index = 4 -->Paint
        if(!ProcessedImage.isNull()){  //图像数据存在时才进行此操作
            QSize ShowAreaSize = ui->PaintImageScroll->frameGeometry().size();        //获取显示区域大小
            ProcessedScale = qMin((ShowAreaSize.rheight()*0.618)/ProcessedImage.size().rheight(),
                         (ShowAreaSize.rwidth() *0.618)/ProcessedImage.size().rwidth()); //根据显示区域大小计算一个合适的缩放比例
            ShowImage(ui->PaintImageScroll,ui->PaintImageScrollContents,ui->PaintImageShow,
                                     ProcessedImage,&ProcessedScale);     //更新Labe组件  显示图片
        }
    }break;
    default : ;break;
    }
}
/************************************************************
*函数：wheelEvent()
*参数：*event 滚轮事件传入参数类
*功能：重构滚轮事件 实现图片的滚轮缩放
*返回值：无
*备注：根据页面索引实现不同区域的图片独立滚轮调节缩放比例
************************************************************/
void QEduFunc::wheelEvent(QWheelEvent *event)
{
    int Angle = event->delta();      //获取鼠标转动滚轮角度
    int StackedIndex = ui->ShowBox->currentIndex();   //获取当前的堆栈页面索引值
    switch (StackedIndex) {    //不同的页面对滚轮的操作有区分
    case 0:{                   //index = 0 -->MainPage
        QRect ShowRect = QRect(ui->ShowBox->pos()+ui->centralwidget->pos(),
                               ui->ShowBox->size());  //计算当前界面的显示区域
        if(ShowRect.contains(this->mapFromGlobal(QCursor::pos()))
           &&(!OriginalImage.isNull())){  //当滚轮事件是在当前页面显示窗口内发生 且有图片正在显示
            if(Angle > 0){                //根据旋转方向调整缩放比例
                if(OriginalScale <   5)   OriginalScale += 0.02;
            }
            else{
                if(OriginalScale > 0.1)   OriginalScale -= 0.02;
            }
            ShowImage(ui->ImageBox,ui->ImageBoxContents,ui->MainImageWidget,
                      OriginalImage,&OriginalScale);  //按新比例显示图片
        }
    }break;
    case 1:{                   //index = 1 -->FuncImageToTab
        QRect ShowRect = QRect(ui->QPreImage->pos()+ui->ShowBox->pos()+ui->centralwidget->pos(),
                               ui->QPreImage->size());  //计算当前界面的显示区域
        QRect ShowRectProcessed  = QRect(ui->MultFuncStack->pos()+ui->ShowBox->pos()+ui->centralwidget->pos(),
                                  ui->MultFuncStack->size());
        if(ShowRect.contains(this->mapFromGlobal(QCursor::pos()))
           &&(!PreImage.isNull())){  //当滚轮事件是在当前页面显示窗口内发生 且有图片正在显示
            if(Angle > 0){                //根据旋转方向调整缩放比例
                if(PreImageScale <   5)   PreImageScale += 0.02;
            }
            else{
                if(PreImageScale > 0.1)   PreImageScale -= 0.02;
            }
            ShowImage(ui->QPreImage,ui->QPreImageContents,ui->QPreImageShow,
                                     PreImage,&PreImageScale);//按新比例显示图片
        }
        if((ShowRectProcessed.contains(this->mapFromGlobal(QCursor::pos())))    //当滚轮事件是在当前页面显示窗口内发生
           &&(!ProcessedImage.isNull())                        //图片数据是否存在
           &&(ui->MultFuncStack->currentIndex() == 2)){       //是否在处理后图片显示界面
                if(Angle > 0){                //根据旋转方向调整缩放比例
                    if(ProcessedScale <   5)   ProcessedScale += 0.02;
                }
                else{
                    if(ProcessedScale > 0.1)   ProcessedScale -= 0.02;
                }
            ShowImage(ui->GeomProcessedScroll,ui->GeomProcessedContents,ui->GeomProcessedShow,
                                     ProcessedImage,&ProcessedScale);  //按新比例显示图片
        }
    }break;
    case 2:{                  //Index = 2 -->GrayTransforFunc
        QRect ShowRectOriginal  = QRect(ui->GrayOriginalScroll->pos()+ui->ShowBox->pos()+ui->centralwidget->pos(),
                                  ui->GrayOriginalScroll->size());    //计算灰度变换界面原始图片的显示区域
        QRect ShowRectProcessed  = QRect(ui->GrayProcessedStack->pos()+ui->ShowBox->pos()+ui->centralwidget->pos(),
                                  ui->GrayProcessedStack->size());
        if(ShowRectOriginal.contains(this->mapFromGlobal(QCursor::pos()))
           &&(!PreImage.isNull())){       //当滚轮事件是在当前页面显示窗口内发生 且有图片正在显示
            if(Angle > 0){                //根据旋转方向调整缩放比例
                if(PreImageScale <   5)   PreImageScale += 0.02;
            }
            else{
                if(PreImageScale > 0.1)   PreImageScale -= 0.02;
            }
            ShowImage(ui->GrayOriginalScroll,ui->GrayOriginalScrollContents,ui->GrayOriginalShow,
                                    PreImage,&PreImageScale);//按新比例显示图片
        }

        if((ShowRectProcessed.contains(this->mapFromGlobal(QCursor::pos())))    //当滚轮事件是在当前页面显示窗口内发生
           &&(!ProcessedImage.isNull())                        //图片数据是否存在
           &&(ui->GrayProcessedStack->currentIndex() == 0)){   //是否在处理后图片显示界面
                if(Angle > 0){                //根据旋转方向调整缩放比例
                    if(ProcessedScale <   5)   ProcessedScale += 0.02;
                }
                else{
                    if(ProcessedScale > 0.1)   ProcessedScale -= 0.02;
                }
            ShowImage(ui->GrayProcessedScroll,ui->GrayProcessedContents,ui->GrayProcessedShow,
                                     ProcessedImage,&ProcessedScale);  //按新比例显示图片
        }
    }break;
    case 3:{       // Index = 3 -->ToneAdjust
        QRect ShowRectOriginal  = QRect(ui->ToneOriginalScroll->pos()+ui->ShowBox->pos()+ui->centralwidget->pos(),
                                  ui->ToneOriginalScroll->size());    //计算灰度变换界面原始图片的显示区域
        QRect ShowRectProcessed  = QRect(ui->ToneProcessedStack->pos()+ui->ShowBox->pos()+ui->centralwidget->pos(),
                                  ui->ToneProcessedStack->size());
        if(ShowRectOriginal.contains(this->mapFromGlobal(QCursor::pos()))
           &&(!PreImage.isNull())){       //当滚轮事件是在当前页面显示窗口内发生 且有图片正在显示
            if(Angle > 0){                //根据旋转方向调整缩放比例
                if(PreImageScale <   5)   PreImageScale += 0.02;
            }
            else{
                if(PreImageScale > 0.1)   PreImageScale -= 0.02;
            }
            ShowImage(ui->ToneOriginalScroll,ui->ToneOriginalScrollContents,ui->ToneOriginalShow,
                                    PreImage,&PreImageScale);//按新比例显示图片
        }

        if((ShowRectProcessed.contains(this->mapFromGlobal(QCursor::pos())))    //当滚轮事件是在当前页面显示窗口内发生
           &&(!ProcessedImage.isNull())                        //图片数据是否存在
           &&(ui->ToneProcessedStack->currentIndex() == 0)){   //是否在处理后图片显示界面
                if(Angle > 0){                //根据旋转方向调整缩放比例
                    if(ProcessedScale <   5)   ProcessedScale += 0.02;
                }
                else{
                    if(ProcessedScale > 0.1)   ProcessedScale -= 0.02;
                }
            ShowImage(ui->ToneProcessedScroll,ui->ToneProcessedContents,ui->ToneProcessedShow,
                                     ProcessedImage,&ProcessedScale);  //按新比例显示图片
        }
    }break;
    case 4:{
        QRect ShowRect  = QRect(ui->PaintImageScroll->pos()+ui->ShowBox->pos()+ui->centralwidget->pos(),
                                  ui->PaintImageScroll->size());    //计算画图界面图片的显示区域
        if(ShowRect.contains(this->mapFromGlobal(QCursor::pos()))){       //当滚轮事件是在当前页面显示窗口内发生 且有图片正在显示
                if(!ProcessedImage.isNull()){  //图像数据存在时才进行此操作
                    double MaxScale = qMin((double(ui->PaintImageScroll->width()))/ProcessedImage.width(),
                                           (double(ui->PaintImageScroll->height()))/ProcessedImage.height())-0.02;
                    if(Angle > 0){                //根据旋转方向调整缩放比例
                        if(ProcessedScale < MaxScale)   ProcessedScale += 0.02;
                    }
                    else{
                        if(ProcessedScale > 0.1)   ProcessedScale -= 0.02;
                    }
                    ShowImage(ui->PaintImageScroll,ui->PaintImageScrollContents,ui->PaintImageShow,
                                             ProcessedImage,&ProcessedScale);     //更新Labe组件  显示图片
                }
        }
    }break;
    default : ;break;
    }
}




void QEduFunc::on_Brush_clicked()
{
    if(OriginalImage.isNull())  //检测是否有图片打开
    {
       QMessageBox::warning(this, tr(""), tr("请先打开可用图片文件！"));
       return ;
    }
    EduFuncsFree();        //界面切换前内存处理
    PreImageScale = 0.0;
    ui->ShowBox->setCurrentIndex(4);              //界面4 画笔等专用界面
    ui->PaintRedValue->setRange(0,255);           //设置SpinBox的范围  红色通道
    ui->PaintGreenValue->setRange(0,255);         //设置SpinBox的范围  绿色通道
    ui->PaintBlueValue->setRange(0,255);          //设置SpinBox的范围  蓝色通道
    ui->PaintRedValue->setSingleStep(5);          //步进值
    ui->PaintGreenValue->setSingleStep(5);        //步进值
    ui->PaintBlueValue->setSingleStep(5);         //步进值
    ui->LineWidth->setSingleStep(1);              //步进值
    ui->PaintRedValue->setValue(0);               //设置初值  红色通道
    ui->PaintGreenValue->setValue(255);             //设置初值  绿色通道
    ui->PaintBlueValue->setValue(0);              //设置初值  蓝色通道
    ui->LineWidth->setValue(2);                   //设置初值  画笔线宽
    ShowPaintColor(QColor(0,255,0));
    ui->PaintBrush->setChecked(true);             //初始化画笔模式
    ProcessedImage = OriginalImage;      //备份原始图片 不修改原始值 所有操作从备份数据中获取处理
    PaintHistory1 = ProcessedImage;      //历史备份
    ProcessedScale = 0;
    ShowImage(ui->PaintImageScroll,ui->PaintImageScrollContents,ui->PaintImageShow,
                            ProcessedImage,&ProcessedScale);  //显示图片
    ui->PaintImageShow->setCursor(Qt::CrossCursor);
    PaintStatus = 1;   //启动绘图功能->画笔功能
}

void QEduFunc::mousePressEvent(QMouseEvent* event){
    if(PaintStatus){   //当启用绘图功能时响应
        QRect ShowRect  = QRect(ui->PaintImageScrollContents->pos()+ui->PaintImageScroll->pos()+ui->ShowBox->pos()+ui->centralwidget->pos(),
                                  ui->PaintImageScrollContents->size());       //计算画图界面图片的显示区域
        if(ShowRect.contains(this->mapFromGlobal(QCursor::pos()))    //在图片区域内
                &&(event->button() == Qt::LeftButton)){              //按下了鼠标左键
            PenColor = QColor(ui->PaintRedValue->value(),ui->PaintGreenValue->value(),ui->PaintBlueValue->value());   //获取当前设定画笔颜色
            PenWidth = ui->LineWidth->value();
            PaintBeginPos = event->pos();   //记录起始点坐标
            PaintBeginPos.setX(int((PaintBeginPos.x()-ShowRect.x())/ProcessedScale));
            PaintBeginPos.setY(int((PaintBeginPos.y()-ShowRect.y())/ProcessedScale));   //转化为图片的相对坐标
            PaintStart = true;   //开始绘图
        }
    }
}
void QEduFunc::mouseMoveEvent(QMouseEvent* event){
    if(PaintStatus && PaintStart){   //当启用绘图功能 并已经开始绘图时响应
        QRect ShowRect  = QRect(ui->PaintImageScrollContents->pos()+ui->PaintImageScroll->pos()+ui->ShowBox->pos()+ui->centralwidget->pos(),
                                  ui->PaintImageScrollContents->size());       //计算画图界面图片的显示区域
        if(ShowRect.contains(this->mapFromGlobal(QCursor::pos()))){    //在图片区域内
            PaintEndPos = event->pos();   //记录起终点坐标
            PaintEndPos.setX(int((PaintEndPos.x()-ShowRect.x())/ProcessedScale));
            PaintEndPos.setY(int((PaintEndPos.y()-ShowRect.y())/ProcessedScale));   //转化为图片的相对坐标
            switch(PaintStatus){
            case 1:{    //画笔功能
                ImageDrawLine(&ProcessedImage,PaintBeginPos,PaintEndPos,PenWidth);//根据起点与终点画线
                PaintBeginPos = PaintEndPos;  //画笔功能需要更新起点
            }break;
            case 2:{    //箭头功能
                ProcessedImage = PaintHistory1;   //每次取新图
                ImageDrawArrow(&ProcessedImage,PaintBeginPos,PaintEndPos,PenWidth); //根据起点与终点画线
            }break;
            case 3:{    //椭圆功能
                ProcessedImage = PaintHistory1;   //每次取新图
                ImageDrawElipse(&ProcessedImage,PaintBeginPos,PaintEndPos,PenWidth); //根据起点与终点画线
            }break;
            case 4:{    //矩形框功能
                ProcessedImage = PaintHistory1;   //每次取新图
                ImageDrawRect(&ProcessedImage,PaintBeginPos,PaintEndPos,PenWidth); //根据起点与终点画线
            }break;
            default :;break;
            }
            ShowImage(ui->PaintImageScroll,ui->PaintImageScrollContents,ui->PaintImageShow,
                                    ProcessedImage,&ProcessedScale);  //刷新显示
        }
        else{  //超出区域  停止画图  取消已画内容
            PaintStart  = false;   //终止当前画图
            ProcessedImage = PaintHistory1;   //恢复画图前
            ShowImage(ui->PaintImageScroll,ui->PaintImageScrollContents,ui->PaintImageShow,
                                    ProcessedImage,&ProcessedScale);  //刷新显示
        }
    }
}
void QEduFunc::mouseReleaseEvent(QMouseEvent* event){
    event->button();
    if(PaintStatus && PaintStart){   //当启用绘图功能时响应
        QRect ShowRect  = QRect(ui->PaintImageScrollContents->pos()+ui->PaintImageScroll->pos()+ui->ShowBox->pos()+ui->centralwidget->pos(),
                                  ui->PaintImageScrollContents->size());       //计算画图界面图片的显示区域
        if(ShowRect.contains(this->mapFromGlobal(QCursor::pos()))){    //在图片区域内
            PushImage(ProcessedImage);  //当前操作保存
            NewImageFlag = 1;           //新图片参生
        }
        else{  //超出区域  停止画图  取消已画内容
            PaintStart = false;    //终止画图
            ProcessedImage = PaintHistory1;   //恢复画图前
            ShowImage(ui->PaintImageScroll,ui->PaintImageScrollContents,ui->PaintImageShow,
                                    ProcessedImage,&ProcessedScale);  //刷新显示
        }
    }
    PaintStart = false;    //结束画图
}

/************************************************************
*函数：ImageDrawLine()
*参数：origin  待处理图片  起点  终点  线宽
*功能：在传入的图片上画一条线
*返回值：无
*备注：直线
************************************************************/
void QEduFunc::ImageDrawLine(QImage *origin,QPoint Begin,QPoint End,int LineWidth){
    *origin = origin->convertToFormat(QImage::Format_RGB32);//统一处理格式
    QPainter *Painter = new QPainter(origin);
    Painter->setPen(QPen(PenColor,LineWidth));  //设置画笔颜色和大小
    Painter->setRenderHint(QPainter::Antialiasing,true);  //设置渲染,启动反锯齿
    QLine Line(Begin,End);
    Painter->drawLine(Line);
    Painter->end();
    delete Painter;
}

void QEduFunc::PushImage(QImage image){
    PaintHistory5 = PaintHistory4;
    PaintHistory4 = PaintHistory3;
    PaintHistory3 = PaintHistory2;
    PaintHistory2 = PaintHistory1;
    PaintHistory1 = image;
}

bool QEduFunc::PopImage(QImage *image){
    if(!PaintHistory1.isNull()){
        *image = PaintHistory1;
        PaintHistory1 = PaintHistory2;
        PaintHistory2 = PaintHistory3;
        PaintHistory3 = PaintHistory4;
        PaintHistory4 = PaintHistory5;
        PaintHistory5 = QImage();
        return true;
    }
    return false;
}

/************************************************************
*函数：ImageDrawLine()
*参数：origin  待处理图片  起点  终点  线宽
*功能：在传入的图片上画一条线
*返回值：无
*备注：直线
************************************************************/
void QEduFunc::ImageDrawRect(QImage *origin,QPoint Begin,QPoint End,int LineWidth){
    *origin = origin->convertToFormat(QImage::Format_RGB32);//统一处理格式
    QPainter *Painter = new QPainter(origin);
    Painter->setPen(QPen(PenColor,LineWidth));  //设置画笔颜色和大小
    Painter->setRenderHint(QPainter::Antialiasing,true);  //设置渲染,启动反锯齿
    QRect Rect(Begin,End);
    Painter->drawRect(Rect);
    Painter->end();
    delete Painter;
}

/************************************************************
*函数：ImageDrawLine()
*参数：origin  待处理图片  起点  终点  线宽
*功能：在传入的图片上画一条线
*返回值：无
*备注：直线
************************************************************/
void QEduFunc::ImageDrawElipse(QImage *origin,QPoint Begin,QPoint End,int LineWidth){
    *origin = origin->convertToFormat(QImage::Format_RGB32);//统一处理格式
    QPainter *Painter = new QPainter(origin);
    Painter->setPen(QPen(PenColor,LineWidth));  //设置画笔颜色和大小
    Painter->setRenderHint(QPainter::Antialiasing,true);  //设置渲染,启动反锯齿
    QRect Rect(Begin,End);
    Painter->drawEllipse(Rect);         //绘制椭圆 当Rect为正方形时为正圆
    Painter->end();
    delete Painter;
}



/************************************************************
*函数：ImageDrawLine()
*参数：origin  待处理图片  起点  终点  线宽
*功能：在传入的图片上画一条线
*返回值：无
*备注：直线
************************************************************/
void QEduFunc::ImageDrawArrow(QImage *origin,QPoint Begin,QPoint End,int LineWidth){
    *origin = origin->convertToFormat(QImage::Format_RGB32);//统一处理格式
    QPainter *Painter = new QPainter(origin);
    Painter->setPen(QPen(PenColor,LineWidth));  //设置画笔颜色和大小
    Painter->setRenderHint(QPainter::Antialiasing,true);  //设置渲染,启动反锯齿
    QPoint ArrowLeft;
    QPoint ArrowRight;
    CalcVertexes(Begin,End,&ArrowLeft,&ArrowRight);
    QLine Line(Begin,End);
    QLine Arrow1(End,ArrowLeft);
    QLine Arrow2(End,ArrowRight);
    QPainterPath path;
    QPolygon pts;
    pts.setPoints(4,End.x(),End.y(),
                  ArrowLeft.x(),ArrowLeft.y(),
                  ArrowRight.x(),ArrowRight.y(),
                  End.x(),End.y()); //第一个参数表示3个点，后面是三个点坐标
    path.addPolygon(pts);
    Painter->drawLine(Line);
    Painter->drawPath(path);
    Painter->fillPath(path,QBrush(PenColor));
    Painter->end();
    delete Painter;
}


void QEduFunc::CalcVertexes(QPoint start,QPoint end,QPoint *ArrowLeft,QPoint *ArrowRight){
    double Arrowlenght  = 20;   //箭头长度
    double Arrowdegrees = 0.5;  //箭头角度
    double angle = qAtan2(end.y() - start.y(), end.x() - start.x());// + 3.1415926;//
    ArrowLeft->setX(int(end.x() - Arrowlenght * qCos(angle - Arrowdegrees)));   //求得箭头点1坐标
    ArrowLeft->setY(int(end.y() - Arrowlenght * qSin(angle - Arrowdegrees)));
    ArrowRight->setX(int(end.x() - Arrowlenght * qCos(angle + Arrowdegrees)));  //求得箭头点1坐标
    ArrowRight->setY(int(end.y() - Arrowlenght * qSin(angle + Arrowdegrees)));
}

void QEduFunc::on_Rectangle_toggled(bool checked)
{
    if(checked){
        PaintStatus = 4;  //选中4号功能  画矩形框
    }
}


void QEduFunc::on_PaintBrush_toggled(bool checked)
{
    if(checked){
        PaintStatus = 1;  //选中1号功能  画笔模式
    }
}

void QEduFunc::on_Circular_toggled(bool checked)
{
    if(checked){
        PaintStatus = 3;  //选中3号功能  画椭圆模式
    }
}

void QEduFunc::on_Arrow_toggled(bool checked)
{
    if(checked){
        PaintStatus = 2;  //选中3号功能  画椭圆模式
    }
}

void QEduFunc::ShowPaintColor(QColor color){
    QImage *ColorImage = new QImage(80,20,QImage::Format_RGB32);
    QPainter *Painter = new QPainter(ColorImage);
    Painter->fillRect(QRect(0,0,80,20),QBrush(color));
    Painter->end();
    delete Painter;
    ui->ColorShow->setPixmap(QPixmap::fromImage(*ColorImage));
}

void QEduFunc::on_PaintColor_clicked()
{
    QColor c = QColorDialog::getColor(Qt::black);
    if (c.isValid())
    {
    ui->PaintRedValue->setValue(c.red());
    ui->PaintGreenValue->setValue(c.green());
    ui->PaintBlueValue->setValue(c.blue());
    ShowPaintColor(c);
    PenColor = c;
    }
}

void QEduFunc::on_PaintRedValue_valueChanged(int RedValue)
{
    PenColor = QColor(RedValue,ui->PaintGreenValue->value(),ui->PaintBlueValue->value());
    ShowPaintColor(PenColor);
}
void QEduFunc::on_PaintGreenValue_valueChanged(int GrennValue)
{
    PenColor = QColor(ui->PaintRedValue->value(),GrennValue,ui->PaintBlueValue->value());
    ShowPaintColor(PenColor);
}

void QEduFunc::on_PaintBlueValue_valueChanged(int BlueValue)
{
    PenColor = QColor(ui->PaintRedValue->value(),ui->PaintGreenValue->value(),BlueValue);
    ShowPaintColor(PenColor);
}
