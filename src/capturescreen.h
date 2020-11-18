#ifndef CAPTURESCREEN_H
#define CAPTURESCREEN_H

#include <QWidget>

namespace Ui {
class CaptureScreen;
}

class CaptureScreen : public QWidget
{
    Q_OBJECT

public:
    explicit CaptureScreen(QWidget *parent = nullptr);
    ~CaptureScreen();

private:
    Ui::CaptureScreen *ui;
};

#endif // CAPTURESCREEN_H
