#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QImage>

#include "applicationsettings.h"
#include "sevrovcontroller.h"
#include "enumclasses.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

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

    ///////////////////////////////////////////////////////////////////////////
    // OpenCV related
    QTimer *_videoTimer;

    // VA (22-05-2024) TODO: Оптимизировать кол-во переменных
    cv::VideoCapture *_webCamO; // Моно-камера
    cv::VideoCapture *_webCamL; // Стерео-камера (левая)
    cv::VideoCapture *_webCamR; // Стерео-камера (правая)

    cv::Mat _sourceMatO;
    cv::Mat _sourceMatL;
    cv::Mat _sourceMatR;

    cv::Mat _destinationMatO;
    cv::Mat _destinationMatL;
    cv::Mat _destinationMatR;

    QImage _imgCamO;
    QImage _imgCamL;
    QImage _imgCamR;
    ///////////////////////////////////////////////////////////////////////////

    void setup_icons();
    void setup_window_geometry();
    void setup_camera_view_layout(CameraView layouttype = CameraView::MONO);
    void setup_connected_controls_style(bool isconnected = false);

    void setup_camera_connection(CameraConnection connection = CameraConnection::ON);

    void on_video_timer();

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
