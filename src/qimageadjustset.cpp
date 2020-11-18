#include "qimageadjustset.h"
#include "ui_qimageadjustset.h"

QImageAdjustSet::QImageAdjustSet(QWidget *parent,int CurrentStackIndex,int ImageW,int ImageH) :
    QDialog(parent),
    ui(new Ui::QImageAdjustSet)
{
    ui->setupUi(this);
    OriginalImageWidth  = ImageW;
    OriginalImageHeight = ImageH;
    MaxWidth  = OriginalImageWidth*10;
    MaxHeight = OriginalImageHeight*10;
    MinWidth  = OriginalImageWidth/10;
    MinHeight = OriginalImageHeight/10;
    ui->AspectRatio->setChecked(true);    //初始化保持纵横比
    if(CurrentStackIndex){
        setWindowTitle(tr("设置图像旋转角度")); //设置标题
        ui->FuncPages->setCurrentIndex(1);   //角度输入界面
        ui->AngleValue->setRange(0,360);     //设置旋转角度范围
        ui->AngleValue->setValue(0);         //初始化旋转0°
    }
    else{
        setWindowTitle(tr("修改图像大小"));
        ui->FuncPages->setCurrentIndex(0);   //图像大小输入界面
        ui->SetPixel->setChecked(true);      //默认像素输入
        ui->VerticalValue->setRange(MinWidth,MaxWidth);    //限制数据输入范围
        ui->HorizontalValue->setRange(MinHeight,MaxHeight);
        ui->VerticalValue->setValue(OriginalImageWidth);   //初始化原始宽度
        ui->HorizontalValue->setValue(OriginalImageHeight);
    }
}

QImageAdjustSet::~QImageAdjustSet()
{
    delete ui;
}


int QImageAdjustSet::ImageWidth()
{
    if(ui->SetPixel->isChecked()){    //像素调整
        return ui->VerticalValue->text().toInt();
    }
    else{
        return int(((ui->VerticalValue->text().toInt())/100.0)*OriginalImageWidth);
    }
}
int QImageAdjustSet::ImageHeight()
{
    if(ui->SetPixel->isChecked()){    //像素调整
        return ui->HorizontalValue->text().toInt();
    }
    else{
        return int(((ui->HorizontalValue->text().toInt())/100.0)*OriginalImageHeight);
    }
}
int QImageAdjustSet::ImageAngle()
{
    return ui->AngleValue->value();  //返回设置的旋转角度
}

void QImageAdjustSet::on_SetPixel_toggled(bool SetPixelchecked)
{
    if(SetPixelchecked){    //像素调整
        ui->VerticalValue->setRange(MinWidth,MaxWidth);
        ui->HorizontalValue->setRange(MinHeight,MaxHeight);
        ui->VerticalValue->setValue(OriginalImageWidth);
        ui->HorizontalValue->setValue(OriginalImageHeight);
    }
    else{                   //百分比调整
        ui->VerticalValue->setRange(10,1000);  //10% - 1000%  ±10倍
        ui->HorizontalValue->setRange(10,1000);
        ui->VerticalValue->setValue(100);
        ui->HorizontalValue->setValue(100);
    }
}

void QImageAdjustSet::on_AspectRatio_stateChanged(int AspectRatioChecked)
{
    if(AspectRatioChecked)
    {
        if(ui->SetPixel->isChecked()){  //像素调整
            double Scale = 0;
            Scale = (double(ui->VerticalValue->value()))/OriginalImageWidth;
            ui->HorizontalValue->setValue(int(Scale*OriginalImageHeight));  //比例缩放
        }
        else{
            ui->HorizontalValue->setValue(ui->VerticalValue->value());  //比例缩放
        }
    }
}

void QImageAdjustSet::on_VerticalValue_valueChanged(int NewWidth)
{
    ui->HorizontalValue->disconnect(SIGNAL(valueChanged(int)));
    if(ui->AspectRatio->isChecked())
    {
        if(ui->SetPixel->isChecked()){  //像素调整
            double Scale = 0;
            Scale = (double(NewWidth))/OriginalImageWidth;
            ui->HorizontalValue->setValue(int(Scale*OriginalImageHeight));  //比例缩放
        }
        else{
            ui->HorizontalValue->setValue(NewWidth);  //比例缩放
        }
    }
    connect(ui->HorizontalValue,SIGNAL(valueChanged(int)),this,SLOT(on_HorizontalValue_valueChanged(int)));
}

void QImageAdjustSet::on_HorizontalValue_valueChanged(int NewHeight)
{
    ui->VerticalValue->disconnect(SIGNAL(valueChanged(int)));
    if(ui->AspectRatio->isChecked())
    {
        if(ui->SetPixel->isChecked()){  //像素调整
            double Scale = 0;
            Scale = (double(NewHeight))/OriginalImageHeight;
            ui->VerticalValue->setValue(int(Scale*OriginalImageWidth));  //比例缩放
        }
        else{
            ui->VerticalValue->setValue(NewHeight);  //比例缩放
        }
    }
    connect(ui->VerticalValue,SIGNAL(valueChanged(int)),this,SLOT(on_VerticalValue_valueChanged(int)));
}
