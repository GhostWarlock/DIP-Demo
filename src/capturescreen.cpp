#include "capturescreen.h"
#include "ui_capturescreen.h"

CaptureScreen::CaptureScreen(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CaptureScreen)
{
    ui->setupUi(this);
    setWindowTitle("3435412.1");
    setGeometry(0,0,parent->size().rwidth(),parent->size().rheight());
    setStyleSheet("background-color:rgba(0,0,0,150);");
}

CaptureScreen::~CaptureScreen()
{
    delete ui;
}
