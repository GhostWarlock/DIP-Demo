#ifndef QFUNCSELECT_H
#define QFUNCSELECT_H

#include <QMainWindow>
#include <qedufunc.h>

namespace Ui {
class QFuncSelect;
}

class QFuncSelect : public QMainWindow
{
    Q_OBJECT

public:
    explicit QFuncSelect(QWidget *parent = nullptr);
    ~QFuncSelect();

private:
    Ui::QFuncSelect *ui;

private slots:  //声明槽函数
    void  on_ButtEdu_clicked();
    void  on_ButtAdvancedFunc_clicked();
};

#endif // QFUNCSELECT_H
