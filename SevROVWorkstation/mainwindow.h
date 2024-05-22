#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "applicationsettings.h"
#include "sevrovcontroller.h"
#include "enumclasses.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    ApplicationSettings _appSet;
    SevROVController _sevROV;

    void setup_icons();
    void setup_window_geometry();
    void setup_camera_view_layout(CameraView layouttype = CameraView::MONO);
    void setup_connected_controls_style(bool isconnected = false);

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pbStartStop_clicked();

    void on_pbView_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
