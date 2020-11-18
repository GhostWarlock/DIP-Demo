#include "qfuncselect.h"
#include "ui_qfuncselect.h"

QFuncSelect::QFuncSelect(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QFuncSelect)
{
    ui->setupUi(this);
    /************* 按钮样式表 ************/
    ui->ButtEdu->setStyleSheet(
                 //正常状态样式
                 "QToolButton{"
                 "background-color:transparent;"    //背景透明
                 "border-style:outset;"             //边框样式（inset/outset）
                 "border-width:1px;"
                 "border-color:transparent;"
                 "border-radius:5px;"               //边框圆角半径像素
                 "}"
                 //鼠标按下样式
                 "QToolButton:pressed{"
                 "padding-left:6px;"       //控制按压悬浮效果
                 "padding-top:4px;"
                 "}"
                 //鼠标悬停样式
                 "QToolButton:hover{"
                 "background-color:rgba(40,148,255,100);"
                 "border-color:rgba(40,148,255,255);"
                 "color:rgba(255,255,255,255);"
                 "}");
    ui->ButtAdvanceFunc->setStyleSheet(
                 //正常状态样式
                 "QToolButton{"
                 "background-color:transparent;"    //背景透明
                 "border-style:outset;"             //边框样式（inset/outset）
                 "border-width:1px;"
                 "border-color:transparent;"
                 "border-radius:5px;"               //边框圆角半径像素
                 "}"
                 //鼠标按下样式
                 "QToolButton:pressed{"
                 "padding-left:6px;"       //控制按压悬浮效果
                 "padding-top:4px;"
                 "}"
                 //鼠标悬停样式
                 "QToolButton:hover{"
                 "background-color:rgba(40,148,255,100);"
                 "border-color:rgba(40,148,255,255);"
                 "color:rgba(255,255,255,255);"
                 "}");
}

QFuncSelect::~QFuncSelect()
{
    delete ui;
}

/************************************************************
*函数：on_ButtEdu_clicked()
*参数：无
*功能：教学模块按钮的槽函数
*返回值：无
*备注：无
************************************************************/
void QFuncSelect::on_ButtEdu_clicked()
{
    this->hide();
    QEduFunc *w = new QEduFunc();    //功能窗口
    w->
            show();
}

/************************************************************
*函数：on_ButtAdvanceFunc_clicked()
*参数：无
*功能：高级功能按钮的槽函数
*返回值：无
*备注：无
************************************************************/
void QFuncSelect::on_ButtAdvancedFunc_clicked()
{
    ;
}




























