#ifndef DIALOGLOGIN_H
#define DIALOGLOGIN_H

#include <QDialog>

namespace Ui {
class DialogLogin;
}

class DialogLogin : public QDialog
{
    Q_OBJECT

public:
    explicit DialogLogin(QWidget *parent = nullptr);
    ~DialogLogin();

private:
    Ui::DialogLogin *ui;
    bool    m_moving=false;//表示窗口是否在鼠标操作下移动
    QPoint  m_lastPos;     //上一次的鼠标位置
    int     m_tryCount=0;  //试错次数

    QString m_user="User"; //初始化用户名
    QString m_pswd="12345"; //初始化密码，未加密的

    void    readSettings(); //读取设置,从注册表
    void    writeSettings();//写入设置，从注册表

private slots:
    void    on_ButtLOGIN_clicked();   //
    void    on_ButtLoginRegister_clicked(); //

protected:
//用于鼠标拖动窗口的鼠标事件操作
     void mousePressEvent(QMouseEvent *event);
     void mouseMoveEvent(QMouseEvent *event);
     void mouseReleaseEvent(QMouseEvent *event);

};

#endif // DIALOGLOGIN_H
