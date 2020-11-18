#include <QApplication>
#include "mainwindow.h"
#include "dialoglogin.h"
#include "qfuncselect.h"
#include "qedufunc.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DialogLogin   *dlgLogin=new DialogLogin;    //创建登录对话框

    if (dlgLogin->exec()==QDialog::Accepted)
    {
        QFuncSelect Functions;     //功能选择窗口
        Functions.show();
        return a.exec();
    }
    else
        return  0;
}
