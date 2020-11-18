
#include    <QMouseEvent>
#include    <QSettings>
#include    <QMessageBox>
#include    <QByteArray>
#include    <QCryptographicHash>

#include <QBitmap>
#include <QPainter>

#include "dialoglogin.h"
#include "ui_dialoglogin.h"




DialogLogin::DialogLogin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogLogin)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);       //设置为关闭时删除
    this->setWindowFlags(Qt::SplashScreen|Qt::WindowStaysOnTopHint);  //设置为SplashScreen, 窗口无边框 窗口置顶显示
    QBitmap bmp(this->size());
    bmp.fill();
    QPainter p(&bmp);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.drawRoundedRect(bmp.rect(),10,10);
    setMask(bmp);
    //关于各控件的配置在UI设计界面样式表设计中，为程序简洁，此处不使用代码式
    ui->LoginPSWD->setEchoMode(QLineEdit::Password); //密码输入编辑框设置为密码输入模式
    /********** 最小化 关闭按钮样式设置 **********/
    ui->ButtLoginClose->setStyleSheet(
        "QToolButton{background-color:transparent}" //正常状态下
        "QToolButton:pressed{background-color:red}" //鼠标按下样式
        "QToolButton:hover{background-color:red}"); //鼠标悬停样式

    readSettings();   //读注册表  登陆参数
}

DialogLogin::~DialogLogin()
{
    delete ui;
}


void DialogLogin::mousePressEvent(QMouseEvent *event)
{ //鼠标按键被按下
    if (event->button() == Qt::LeftButton)
    {
        m_moving = true;
        //记录下鼠标相对于窗口的位置
        //event->globalPos()鼠标按下时，鼠标相对于整个屏幕位置
        //pos()->this->pos()鼠标按下时，窗口相对于整个屏幕位置
        m_lastPos = event->globalPos() - pos();
    }
    return QDialog::mousePressEvent(event);  //
}

void DialogLogin::mouseMoveEvent(QMouseEvent *event)
{//鼠标按下左键移动
    //(event->buttons() && Qt::LeftButton)按下是左键
    //鼠标移动事件需要移动窗口，窗口移动到哪里呢？就是要获取鼠标移动中，窗口在整个屏幕的坐标，然后move到这个坐标，怎么获取坐标？
    //通过事件event->globalPos()知道鼠标坐标，鼠标坐标减去鼠标相对于窗口位置，就是窗口在整个屏幕的坐标
    if (m_moving && (event->buttons() && Qt::LeftButton)
        && (event->globalPos()-m_lastPos).manhattanLength() > QApplication::startDragDistance())
    {
        move(event->globalPos()-m_lastPos);
        m_lastPos = event->globalPos() - pos();
    }
    return QDialog::mouseMoveEvent(event);
}

void DialogLogin::mouseReleaseEvent(QMouseEvent *event)
{//鼠标按键释放
    event->globalPos();
    m_moving=false; //停止移动
}
/************************************************************
*函数：readSettings()
*参数：无
*功能：读取存储的用户名和密码, 密码是经过加密的
*返回值：无
*备注：无
************************************************************/
void DialogLogin::readSettings()
{
    QString organization="WWB-Qt";//用于注册表，
    QString appName="DIP_Demo";   //HKEY_CURRENT_USER/Software/WWB-Qt/DIP_Demo
    QSettings  settings(organization,appName);//创建
    bool saved=settings.value("saved",false).toBool();//读取 saved键的值

    m_user=settings.value("Username","User").toString();//读取 Username 键的值，缺省为“user”
    m_pswd=settings.value("PSWD","12345").toString();     //读取PSWD

    ui->LoginUserName->setText(m_user);
    if(saved)  //如果之前有保存  调用  默认显示旧的账号密码
    {
        ui->LoginPSWD->setText(m_pswd);
    }
    ui->LoginRemb->setChecked(saved);
}

/************************************************************
*函数：writeSettings()
*参数：无
*功能：保存用户名，密码等设置
*返回值：无
*备注：无
************************************************************/
void DialogLogin::writeSettings()
{
    QSettings   settings("WWB-Qt","DIP_Demo"); //注册表键组
    settings.setValue("Username",m_user); //用户名
    settings.setValue("PSWD",m_pswd);   //密码
    settings.setValue("saved",ui->LoginRemb->isChecked());
}

/************************************************************
*函数：on_ButtLOGIN_clicked()
*参数：无
*功能：登陆按钮的槽函数
*返回值：无
*备注：无
************************************************************/
void DialogLogin::on_ButtLOGIN_clicked()
{
    QString user=ui->LoginUserName->text().trimmed();  //获取输入用户名
    QString pswd=ui->LoginPSWD->text().trimmed();      //获取输入密码
    if ((user == m_user)&&(pswd == m_pswd)) //如果用户名和密码正确
    {
        writeSettings();//保存设置
        this->accept(); //对话框 accept()，关闭对话框
    }
    else
    {
        m_tryCount++; //错误次数
        if (m_tryCount>3)
        {
            QMessageBox::critical(this, "错误", "输入错误次数太多，强行退出");
            this->reject(); //退出
        }
        else
            QMessageBox::warning(this, "错误提示", "用户名或密码错误");
    }
}

/************************************************************
*函数：on_ButtLoginRegister_clicked()
*参数：无
*功能：注册按钮的槽函数
*返回值：无
*备注：无
************************************************************/
void DialogLogin::on_ButtLoginRegister_clicked()
{
    QString NewUser = ui->LoginUserName->text().trimmed();  //获取输入用户名
    QString NewPSWD = ui->LoginPSWD->text().trimmed();      //获取输入密码
    if(QString(NewUser).isEmpty())
    {
        QMessageBox::warning(this, tr(""), tr("用户名格式错误   \r\n请重新输入！   "));
    }
    else if(QString(NewPSWD).isEmpty())
    {
        QMessageBox::warning(this, tr(""), tr("密码格式错误     \r\n请重新输入！   "));
    }
    else
    {
        if (QMessageBox::Yes == QMessageBox::question(this,
                                                      tr(""),
                                                      tr("新的账号密码将会覆盖旧的账户，是否创建？"),
                                                      QMessageBox::Yes | QMessageBox::No,
                                                      QMessageBox::Yes))
        {
            m_user = NewUser;
            m_pswd = NewPSWD;
            writeSettings();   //保存设置  新账户
            QMessageBox::information(this, tr(""), tr("注册成功！"));
        }
        else
        {
            QMessageBox::information(this, tr(""), tr("注册失败！"));
        }
    }
}














