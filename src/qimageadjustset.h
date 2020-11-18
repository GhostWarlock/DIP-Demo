#ifndef QIMAGEADJUSTSET_H
#define QIMAGEADJUSTSET_H

#include <QDialog>
#include <QMessageBox>
#include <QValidator>
#include <QDebug>
namespace Ui {
class QImageAdjustSet;
}

class QImageAdjustSet : public QDialog
{
    Q_OBJECT

public:
    int ImageWidth();
    int ImageHeight();
    int ImageAngle();

    explicit QImageAdjustSet(QWidget *parent = nullptr,int CurrentStackIndex = 0,int ImageW=0,int ImageH=0);
    ~QImageAdjustSet();

private slots:

    void on_SetPixel_toggled(bool SetPixelchecked);
//    void on_HorizontalValue_editingFinished();
//    void on_VerticalValue_editingFinished();

    void on_AspectRatio_stateChanged(int arg1);

//    void on_VerticalValue_textChanged(const QString &arg1);

//    void on_HorizontalValue_textChanged(const QString &arg1);

    void on_VerticalValue_valueChanged(int arg1);

    void on_HorizontalValue_valueChanged(int arg1);

private:
    Ui::QImageAdjustSet *ui;


private:
    int OriginalImageWidth = 0;
    int OriginalImageHeight = 0;
    int MaxWidth  = OriginalImageWidth*10;
    int MaxHeight = OriginalImageHeight*10;
    int MinWidth  = OriginalImageWidth/10;
    int MinHeight = OriginalImageHeight/10;
};








#endif // QIMAGEADJUSTSET_H
