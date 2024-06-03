#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScreen>
#include <QDir>

#include <fstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Загрузка настроек
    _appSet.load();

    // TODO VA (23-05-2024): После подключения общей библиотеки, использовать
    // setWindowTitle("ТНПА :: Контроль :: " + QString(APP_VERSION.c_str()));
    setWindowTitle("ТНПА :: AРМ Оператора :: " + _appSet.getAppVersion());

    // Устанавливаем геометрию окна и основных элементов
    setup_window_geometry();

    // Установка иконок
    setup_icons();

    // Layout по умолчанию - одиночная камера
    setup_camera_view_layout(CameraView::MONO);
    setup_connected_controls_style(false);

    // Цвет фона главного окна приложения
    this->setStyleSheet("background-color: black;");

    QFont fontLabel("GOST type A", 8, QFont::Bold);
    ui->lbFPS->setStyleSheet("background-color : black; color : silver;");
    ui->lbFPS->setFont(fontLabel);

    _videoTimer = new QTimer(this);
    connect(_videoTimer, &QTimer::timeout, this, &MainWindow::on_video_timer);

    QObject::connect(this, SIGNAL(update_fps_value(QString)), ui->lbFPS, SLOT(setText(QString)));
}

MainWindow::~MainWindow()
{
    if (_webCamO->isOpened())
        _webCamO->release();
    if (_webCamL->isOpened())
        _webCamL->release();
    if (_webCamR->isOpened())
        _webCamR->release();

    // Освобождение ресурсов
    if (!_videoTimer->isActive())
        _videoTimer->stop();

    if (_videoTimer)
        delete _videoTimer;

    if (_webCamO)
        delete _webCamO;

    if (_webCamL)
        delete _webCamL;

    if (_webCamR)
        delete _webCamR;

    if (_toolWindow)
        delete _toolWindow;

    delete ui;
}

void MainWindow::on_pbStartStop_clicked()
{
    // Меняем состояние флага
    _sevROV.isConnected = !_sevROV.isConnected;
    _cnt = 0; // Сбрасываем счетчик
    Q_EMIT update_fps_value("CNT: " + QString::number(_cnt++));

    // Меняем иконку на кнопке
    if (_sevROV.isConnected)
    {
        ui->pbStartStop->setIcon(QIcon(":/img/on_button_icon.png"));
        ui->pbStartStop->setIconSize(QSize(64, 64));

        setup_camera_connection(CameraConnection::ON);
    }
    else
    {
        ui->pbStartStop->setIcon(QIcon(":/img/off_button_icon.png"));
        ui->pbStartStop->setIconSize(QSize(64, 64));

        setup_camera_connection(CameraConnection::OFF);
    }

    setup_connected_controls_style(_sevROV.isConnected);
}

void MainWindow::on_pbView_clicked()
{
    // Если нет соединения - выход
    if (!_sevROV.isConnected)
        return;

    switch (_sevROV.cameraView)
    {
    case CameraView::MONO:
        ui->pbView->setIcon(QIcon(":/img/video_icon.png"));
        ui->pbView->setIconSize(QSize(64, 64));
        _sevROV.cameraView = CameraView::STEREO;

        break;
    case CameraView::STEREO:
        ui->pbView->setIcon(QIcon(":/img/display_icon.png"));
        ui->pbView->setIconSize(QSize(64, 64));
        _sevROV.cameraView = CameraView::MONO;
        break;
    default:
        break;
    }

    setup_camera_view_layout(_sevROV.cameraView);
}

void MainWindow::setup_icons()
{
    // Иконка главного окна
    setWindowIcon(QIcon(":/img/sevrov.png"));

    // Установка иконок для кнопок
    ui->pbSettings->setIcon(QIcon(":/img/settings_icon.png"));
    ui->pbSettings->setIconSize(QSize(64, 64));
    ui->pbStartStop->setIcon(QIcon(":/img/off_button_icon.png"));
    ui->pbStartStop->setIconSize(QSize(64, 64));
    ui->pbView->setIcon(QIcon(":/img/display_icon.png"));
    ui->pbView->setIconSize(QSize(64, 64));
    ui->pbScreenshot->setIcon(QIcon(":/img/camera_icon.png"));
    ui->pbScreenshot->setIconSize(QSize(64, 64));
}

void MainWindow::move_window_to_center()
{
    auto primaryScreen = QGuiApplication::primaryScreen(); // Главный экран
    QRect primaryScreenRect = primaryScreen->availableGeometry(); // Размер главного экрана
    QPoint primaryScreenRectCenter = primaryScreenRect.center();
    primaryScreenRectCenter.setX(primaryScreenRectCenter.x() - (this->width()/2));
    primaryScreenRectCenter.setY(primaryScreenRectCenter.y() - (this->height()/2));
    move(primaryScreenRectCenter);
}

void MainWindow::setup_window_geometry()
{
    // Установка размера главного окна// Установка размера главного окна
    int windowWidth = _appSet.CAMERA_WIDTH + _appSet.CONTROL_PANEL_WIDTH + _appSet.CAMERA_VIEW_BORDER_WIDTH * 4;
    int windowHeight = _appSet.CAMERA_HEIGHT + _appSet.CAMERA_VIEW_BORDER_WIDTH * 2;

    // Фиксируем размер окна и убираем иконку ресайза
    setFixedSize(QSize(windowWidth, windowHeight));

    // Центрируем окно в пределах экрана
    move_window_to_center();

    QRect mainWindowRect = this->geometry();

    // Геометрия окон камер
    ui->lbCamera->setGeometry(
        _appSet.CAMERA_VIEW_X0,
        _appSet.CAMERA_VIEW_Y0,
        _appSet.CAMERA_WIDTH,
        _appSet.CAMERA_HEIGHT);

    ui->lbCameraL->setGeometry(
        _appSet.CAMERA_VIEW_X0,
        (mainWindowRect.height() - _appSet.CAMERA_HEIGHT / 2) / 2,
        _appSet.CAMERA_WIDTH / 2,
        _appSet.CAMERA_HEIGHT / 2);

    ui->lbCameraR->setGeometry(
        _appSet.CAMERA_VIEW_X0 + _appSet.CAMERA_WIDTH / 2 + _appSet.CAMERA_VIEW_BORDER_WIDTH,
        (mainWindowRect.height() - _appSet.CAMERA_HEIGHT / 2) / 2,
        _appSet.CAMERA_WIDTH / 2,
        _appSet.CAMERA_HEIGHT / 2);

    // Позицианируем панель управления
    ui->gbControlButtons->setGeometry(
        mainWindowRect.width() - _appSet.CONTROL_PANEL_WIDTH - _appSet.CAMERA_VIEW_BORDER_WIDTH,
        _appSet.CAMERA_VIEW_Y0,
        _appSet.CONTROL_PANEL_WIDTH,
        mainWindowRect.height() - _appSet.CAMERA_VIEW_BORDER_WIDTH * 2);
}

void MainWindow::setup_camera_view_layout(CameraView layouttype)
{
    switch (layouttype)
    {
    case CameraView::MONO:
        ui->lbCamera->setVisible(true);
        ui->lbCameraL->setVisible(false);
        ui->lbCameraR->setVisible(false);
        break;
    case CameraView::STEREO:
        ui->lbCamera->setVisible(false);
        ui->lbCameraL->setVisible(true);
        ui->lbCameraR->setVisible(true);
        break;
    }
}

void MainWindow::setup_connected_controls_style(bool isconnected)
{
    if (isconnected)
    {
        // Обводка внешней границы окон камер
        ui->lbCamera->setStyleSheet("QLabel {"
                                    "border-style: solid;"
                                    "border-width: 1px;"
                                    "border-color: #F0BE50; "
                                    "}");
        ui->lbCameraL->setStyleSheet("QLabel {"
                                     "border-style: solid;"
                                     "border-width: 1px;"
                                     "border-color: #F0BE50; "
                                     "}");
        ui->lbCameraR->setStyleSheet("QLabel {"
                                     "border-style: solid;"
                                     "border-width: 1px;"
                                     "border-color: #F0BE50; "
                                     "}");

        // Обводка внешней границы блока управления
        ui->gbControlButtons->setStyleSheet("QGroupBox {"
                                            "border-style: solid;"
                                            "border-width: 1px;"
                                            "border-color: #F0BE50; "
                                            "}");
    }
    else
    {
        // Обводка внешней границы окон камер
        ui->lbCamera->setStyleSheet("QLabel {"
                                    "border-style: solid;"
                                    "border-width: 1px;"
                                    "border-color: silver; "
                                    "}");
        ui->lbCameraL->setStyleSheet("QLabel {"
                                     "border-style: solid;"
                                     "border-width: 1px;"
                                     "border-color: silver; "
                                     "}");
        ui->lbCameraR->setStyleSheet("QLabel {"
                                     "border-style: solid;"
                                     "border-width: 1px;"
                                     "border-color: silver; "
                                     "}");

        // Обводка внешней границы блока управления
        ui->gbControlButtons->setStyleSheet("QGroupBox {"
                                            "border-style: solid;"
                                            "border-width: 1px;"
                                            "border-color: silver; "
                                            "}");
    }
}

void MainWindow::setup_camera_connection(CameraConnection connection)
{
    switch (connection)
    {
    case CameraConnection::ON:

        // Выделяем ресурсы
        //switch (_sevROV.cameraView)
        //{
        //case CameraView::MONO:
        //    _webCam = new cv::VideoCapture(camID);
        //    break;
        //case CameraView::STEREO:
        //    _webCamL = new cv::VideoCapture(camIDL);
        //    _webCamR = new cv::VideoCapture(camIDR);
        //    break;
        //default:
        //    break;
        //}

        _webCamO = new cv::VideoCapture(_appSet.CAMERA_ID);
        _webCamL = new cv::VideoCapture(_appSet.CAMERA_LEFT_ID);
        _webCamR = new cv::VideoCapture(_appSet.CAMERA_RIGHT_ID);

        // TODO VA (23-05-2024): Оно работает вообще?
        _webCamO->set(cv::CAP_PROP_FPS, _appSet.CAMERA_FPS);
        _webCamL->set(cv::CAP_PROP_FPS, _appSet.CAMERA_FPS);
        _webCamR->set(cv::CAP_PROP_FPS, _appSet.CAMERA_FPS);

        // Запускаем таймер
        if (!_videoTimer->isActive())
            _videoTimer->start(_appSet.VIDEO_TIMER_INTERVAL);

        break;
    case CameraConnection::OFF:

        // Освобождаем ресурсы
        if (_webCamO->isOpened())
            _webCamO->release();
        if (_webCamL->isOpened())
            _webCamL->release();
        if (_webCamR->isOpened())
            _webCamR->release();

        // Остановка таймера
        if (!_videoTimer->isActive())
            _videoTimer->stop();

        // Стереть старое изображение
        QPixmap pixmap;
        QColor color;

        color = QColor(0, 0, 0, 255);
        pixmap = QPixmap(ui->lbCamera->size());
        pixmap.fill(color);
        ui->lbCamera->setPixmap(pixmap);

        pixmap = QPixmap(ui->lbCameraL->size());
        pixmap.fill(color);
        ui->lbCameraL->setPixmap(pixmap);
        ui->lbCameraR->setPixmap(pixmap);

        // Останавливаем таймер
        if (_videoTimer->isActive())
            _videoTimer->stop();

        break;
    }
}

// https://stackoverflow.com/questions/18973103/how-to-draw-a-rounded-rectangle-rectangle-with-rounded-corners-with-opencv
void MainWindow::rounded_rectangle(
    cv::Mat& src,
    cv::Point topLeft,
    cv::Point bottomRight,
    const cv::Scalar lineColor,
    const int thickness,
    const int lineType,
    const int cornerRadius)
{
     // Сorners:
     // p1 - p2
     // |     |
     // p4 - p3

    cv::Point p1 = topLeft;
    cv::Point p2 = cv::Point(bottomRight.x, topLeft.y);
    cv::Point p3 = bottomRight;
    cv::Point p4 = cv::Point(topLeft.x, bottomRight.y);

    // Draw Straight Lines
    cv::line(src, cv::Point(p1.x + cornerRadius, p1.y), cv::Point(p2.x - cornerRadius, p2.y), lineColor, thickness, lineType);
    cv::line(src, cv::Point(p2.x, p2.y + cornerRadius), cv::Point(p3.x, p3.y - cornerRadius), lineColor, thickness, lineType);
    cv::line(src, cv::Point(p4.x + cornerRadius, p4.y), cv::Point(p3.x-cornerRadius, p3.y), lineColor, thickness, lineType);
    cv::line(src, cv::Point(p1.x, p1.y + cornerRadius), cv::Point(p4.x, p4.y - cornerRadius), lineColor, thickness, lineType);

    // Draw Arcs
    cv::ellipse(src, p1 + cv::Point(cornerRadius, cornerRadius), cv::Size(cornerRadius, cornerRadius), 180.0, 0, 90, lineColor, thickness, lineType);
    cv::ellipse(src, p2 + cv::Point(-cornerRadius, cornerRadius), cv::Size(cornerRadius, cornerRadius), 270.0, 0, 90, lineColor, thickness, lineType);
    cv::ellipse(src, p3 + cv::Point(-cornerRadius, -cornerRadius), cv::Size(cornerRadius, cornerRadius), 0.0, 0, 90, lineColor, thickness, lineType);
    cv::ellipse(src, p4 + cv::Point(cornerRadius, -cornerRadius), cv::Size(cornerRadius, cornerRadius), 90.0, 0, 90, lineColor, thickness, lineType);
}
void MainWindow::on_video_timer()
{
    int X0 = _appSet.CAMERA_WIDTH / 2;
    int Y0 = _appSet.CAMERA_HEIGHT / 2;


    int GRID_SMALL_SIZE = 15;
    int GRID_BIG_SIZE = 30;

    int GRID_V_DELTA = _appSet.CAMERA_HEIGHT / 75;
    int GRID_V_MAX = 5;

    int GRID_H_DELTA = _appSet.CAMERA_WIDTH / 100;
    int GRID_H_MAX = 5;

    int XV0 = _appSet.CAMERA_WIDTH / 5;
    int YV0 = _appSet.CAMERA_HEIGHT / 2 - GRID_V_DELTA * 2 * 10;

    int XH0 = _appSet.CAMERA_WIDTH / 2 - GRID_H_DELTA * 2 * 10;
    int YH0 = _appSet.CAMERA_HEIGHT / 10;

    int SIGHT_SIZE = 50;
    int SIGHT_TICK = 10;
    int SIGHT_CROSS = 20;
    int SIGHT_DELTA = 5;

    cv::Mat overlayImage;
    cv::Mat transparencyiImage;
    double alpha = 0.5;

    cv::Mat resizedMatO;
    cv::Mat resizedMatL;
    cv::Mat resizedMatR;

    // VA (23-05-2024) Не работает...
    // double fps;
    // fps = _webCamO->get(cv::CAP_PROP_FPS);

    Q_EMIT update_fps_value("CNT: " + QString::number(_cnt++));

    switch (_sevROV.cameraView)
    {
    case CameraView::MONO:

        _webCamO->read(_sourceMatO);

        if (_sourceMatO.empty())
            return;

        cv::resize(_sourceMatO, resizedMatO, cv::Size(_appSet.CAMERA_WIDTH, _appSet.CAMERA_HEIGHT));

        cv::cvtColor(resizedMatO, _destinationMatO, cv::COLOR_BGR2RGB);

        ///////////////////////////////////////////////////////////////////////
        // Отрисовка прицела
        ///////////////////////////////////////////////////////////////////////
        //cv::rectangle(_destinationMatO,
        //              cv::Point(X0 - SIGHT_SIZE, Y0 - SIGHT_SIZE),
        //              cv::Point(X0 + SIGHT_SIZE, Y0 + SIGHT_SIZE),
        //              CV_RGB(255, 255, 255), 1, 0);

        _destinationMatO.copyTo(overlayImage);

        // Внешний контур
        rounded_rectangle(_destinationMatO,
                          cv::Point(X0 - SIGHT_SIZE, Y0 - SIGHT_SIZE),
                          cv::Point(X0 + SIGHT_SIZE, Y0 + SIGHT_SIZE),
                          CV_RGB(0, 255, 255),
                          2,
                          cv::LINE_8,
                          10);

        // Рисочки внешнего контура
        cv::line(_destinationMatO,
                 cv::Point(X0, Y0 - SIGHT_SIZE),
                 cv::Point(X0, Y0 - SIGHT_SIZE + SIGHT_TICK),
                 CV_RGB(0, 255, 255),
                 1,
                 cv::LINE_8);
        cv::line(_destinationMatO,
                 cv::Point(X0, Y0 + SIGHT_SIZE),
                 cv::Point(X0, Y0 + SIGHT_SIZE - SIGHT_TICK),
                 CV_RGB(0, 255, 255),
                 1,
                 cv::LINE_8);
        cv::line(_destinationMatO,
                 cv::Point(X0 - SIGHT_SIZE, Y0 ),
                 cv::Point(X0 - SIGHT_SIZE + SIGHT_TICK, Y0),
                 CV_RGB(0, 255, 255),
                 1,
                 cv::LINE_8);
        cv::line(_destinationMatO,
                 cv::Point(X0 + SIGHT_SIZE, Y0 ),
                 cv::Point(X0 + SIGHT_SIZE - SIGHT_TICK, Y0),
                 CV_RGB(0, 255, 255),
                 1,
                 cv::LINE_8);

        // Рисочки внутреннего прицела
        cv::line(_destinationMatO,
                 cv::Point(X0 - SIGHT_DELTA, Y0),
                 cv::Point(X0 - SIGHT_DELTA - SIGHT_CROSS, Y0),
                 CV_RGB(255, 255, 255),
                 1,
                 cv::LINE_8);
        cv::line(_destinationMatO,
                 cv::Point(X0 + SIGHT_DELTA, Y0),
                 cv::Point(X0 + SIGHT_DELTA + SIGHT_CROSS, Y0),
                 CV_RGB(255, 255, 255),
                 1,
                 cv::LINE_8);

        cv::line(_destinationMatO,
                 cv::Point(X0, Y0 - SIGHT_DELTA),
                 cv::Point(X0, Y0 - SIGHT_DELTA - SIGHT_CROSS),
                 CV_RGB(255, 255, 255),
                 1,
                 cv::LINE_8);
        cv::line(_destinationMatO,
                 cv::Point(X0, Y0 + SIGHT_DELTA),
                 cv::Point(X0, Y0 + SIGHT_DELTA + SIGHT_CROSS),
                 CV_RGB(255, 255, 255),
                 1,
                 cv::LINE_8);

        ///////////////////////////////////////////////////////////////////////
        // Риски вертикальные (левые)
        ///////////////////////////////////////////////////////////////////////
        for (int i = 1; i < GRID_V_MAX; i++)
        {
            cv::line(_destinationMatO,
                     cv::Point(XV0, YV0 + GRID_V_DELTA * 10 * (i - 1)),
                     cv::Point(XV0 + GRID_BIG_SIZE, YV0 + GRID_V_DELTA * 10 * (i - 1)),
                     CV_RGB(255, 255, 255),
                     2,
                     cv::LINE_8);

            for (int j = 1; j < 10; j++)
            {
                cv::line(_destinationMatO,
                         cv::Point(XV0 + GRID_SMALL_SIZE, YV0 + GRID_V_DELTA * 10 * (i - 1) + j * GRID_V_DELTA),
                         cv::Point(XV0 + 2 * GRID_SMALL_SIZE, YV0 + GRID_V_DELTA * 10 * (i - 1) + j * GRID_V_DELTA),
                         CV_RGB(255, 255, 255),
                         1,
                         cv::LINE_8);
            }
        }
        // Завершающая
        cv::line(_destinationMatO,
                 cv::Point(XV0, YV0 + GRID_V_DELTA * 10 * (GRID_V_MAX - 1)),
                 cv::Point(XV0 + GRID_BIG_SIZE, YV0 + GRID_V_DELTA * 10 * (GRID_V_MAX - 1)),
                 CV_RGB(255, 255, 255),
                 2,
                 cv::LINE_8);


        ///////////////////////////////////////////////////////////////////////
        // Риски вертикальные (правые)
        ///////////////////////////////////////////////////////////////////////
        for (int i = 1; i < GRID_V_MAX; i++)
        {
            cv::line(_destinationMatO,
                     cv::Point(X0 + (X0 - XV0), YV0 + GRID_V_DELTA * 10 * (i - 1)),
                     cv::Point(X0 + (X0 - XV0) + GRID_BIG_SIZE, YV0 + GRID_V_DELTA * 10 * (i - 1)),
                     CV_RGB(255, 255, 255),
                     2,
                     cv::LINE_8);

            for (int j = 1; j < 10; j++)
            {
                cv::line(_destinationMatO,
                         cv::Point(X0 + (X0 - XV0), YV0 + GRID_V_DELTA * 10 * (i - 1) + j * GRID_V_DELTA),
                         cv::Point(X0 + (X0 - XV0) + GRID_SMALL_SIZE, YV0 + GRID_V_DELTA * 10 * (i - 1) + j * GRID_V_DELTA),
                         CV_RGB(255, 255, 255),
                         1,
                         cv::LINE_8);
            }
        }
        // Завершающая
        cv::line(_destinationMatO,
                 cv::Point(X0 + (X0 - XV0), YV0 + GRID_V_DELTA * 10 * (GRID_V_MAX - 1)),
                 cv::Point(X0 + (X0 - XV0) + GRID_BIG_SIZE, YV0 + GRID_V_DELTA * 10 * (GRID_V_MAX - 1)),
                 CV_RGB(255, 255, 255),
                 2,
                 cv::LINE_8);


        ///////////////////////////////////////////////////////////////////////
        // Риски горизонтальные
        ///////////////////////////////////////////////////////////////////////
        for (int i = 1; i < GRID_H_MAX; i++)
        {
            cv::line(_destinationMatO,
                     cv::Point(XH0 + GRID_H_DELTA * 10 * (i - 1), YH0),
                     cv::Point(XH0 + GRID_H_DELTA * 10 * (i - 1), YH0 + GRID_BIG_SIZE),
                     CV_RGB(255, 255, 255),
                     2,
                     cv::LINE_8);

            for (int j = 1; j < 10; j++)
            {
                cv::line(_destinationMatO,
                         cv::Point(XH0 + GRID_H_DELTA * 10 * (i - 1) + j * GRID_H_DELTA, YH0 + GRID_SMALL_SIZE),
                         cv::Point(XH0 + GRID_H_DELTA * 10 * (i - 1) + j * GRID_H_DELTA , YH0 + 2 * GRID_SMALL_SIZE),
                         CV_RGB(255, 255, 255),
                         1,
                         cv::LINE_8);
            }
        }
        // Завершающая
        cv::line(_destinationMatO,
                 cv::Point(XH0 + GRID_H_DELTA * 10 * (GRID_H_MAX - 1), YH0),
                 cv::Point(XH0 + GRID_H_DELTA * 10 * (GRID_H_MAX - 1), YH0 + GRID_BIG_SIZE),
                 CV_RGB(255, 255, 255),
                 2,
                 cv::LINE_8);
        ///////////////////////////////////////////////////////////////////////
        // Табличка
        cv::rectangle(_destinationMatO,
                      cv::Point(XV0, _appSet.CAMERA_HEIGHT - 50),
                      cv::Point(X0 + (X0 - XV0) + GRID_BIG_SIZE, _appSet.CAMERA_HEIGHT - 100),
                      CV_RGB(255, 255, 255), 2, cv::LINE_8);
        cv::rectangle(_destinationMatO,
                      cv::Point(XV0, _appSet.CAMERA_HEIGHT - 50),
                      cv::Point(X0 + (X0 - XV0) + GRID_BIG_SIZE, _appSet.CAMERA_HEIGHT - 100),
                      CV_RGB(0, 0, 0), -1);

        ///////////////////////////////////////////////////////////////////////
        // Диагностика

        cv::putText(_destinationMatO, "DIAGNOSTIC: 0.00 [-]; 0.00 [-] " + QTime::currentTime().toString("hh:mm:ss").toStdString(), cv::Point(XV0 + 20, _appSet.CAMERA_HEIGHT - 65), cv::FONT_HERSHEY_SIMPLEX, 1, CV_RGB(255, 255, 255), 2);
        ///////////////////////////////////////////////////////////////////////
        // Склейка
        cv::addWeighted(overlayImage, alpha, _destinationMatO, 1 - alpha, 0, transparencyiImage);
        ///////////////////////////////////////////////////////////////////////

        _imgCamO = QImage((uchar*) transparencyiImage.data,
                          transparencyiImage.cols,
                          transparencyiImage.rows,
                          transparencyiImage.step,
                          QImage::Format_RGB888);


        //_imgCamO = QImage((uchar*) _destinationMatO.data,
        //                  _destinationMatO.cols,
        //                  _destinationMatO.rows,
        //                  _destinationMatO.step,
        //                  QImage::Format_RGB888);

        ui->lbCamera->setPixmap(QPixmap::fromImage(_imgCamO));
        break;
    case CameraView::STEREO:
        ///////////////////////////////////////////////////////////////////////
        // Left Camera
        _webCamL->read(_sourceMatL);

        if (_sourceMatL.empty())
            return;

        cv::resize(_sourceMatL, resizedMatL, cv::Size(_appSet.CAMERA_WIDTH / 2, _appSet.CAMERA_HEIGHT / 2));

        cv::cvtColor(resizedMatL, _destinationMatL, cv::COLOR_BGR2RGB);

        _imgCamL = QImage((uchar*) _destinationMatL.data,
                          _destinationMatL.cols,
                          _destinationMatL.rows,
                          _destinationMatL.step,
                          QImage::Format_RGB888);

        ui->lbCameraL->setPixmap(QPixmap::fromImage(_imgCamL));

        ///////////////////////////////////////////////////////////////////////
        // Right Camera
        _webCamR->read(_sourceMatR);

        if (_sourceMatR.empty())
            return;

        cv::resize(_sourceMatR, resizedMatR, cv::Size(_appSet.CAMERA_WIDTH / 2, _appSet.CAMERA_HEIGHT / 2));

        cv::cvtColor(resizedMatR, _destinationMatR, cv::COLOR_BGR2RGB);

        _imgCamR = QImage((uchar*) _destinationMatR.data,
                          _destinationMatR.cols,
                          _destinationMatR.rows,
                          _destinationMatR.step,
                          QImage::Format_RGB888);

        ui->lbCameraR->setPixmap(QPixmap::fromImage(_imgCamR));
        break;
    default:
        break;
    }
}

t_vuxyzrgb MainWindow:: get_cloud_3D_points(int rows, int cols, bool norm = true)
{
    t_vuxyzrgb data;

    // Путь к папке с данными
    auto dataPath = QDir::cleanPath(qApp->applicationDirPath() +
                                    QDir::separator() + "data");
    // qDebug() << "Data path : " << dataPath;

    // Чтение данных
    auto fileName = QFileDialog::getOpenFileName(this,
                                                 tr("Open Data File"),
                                                 dataPath,
                                                 tr("TXT Files (*.txt)"));
    QFile file(fileName);
    QStringList lineData;
    Data3DItem data3DItem;

    int vuX;
    int vuY;
    double xyzX;
    double xyzY;
    double xyzZ;
    int rgbR;
    int rgbG;
    int rgbB;
    int clst;

    int Xmin = INT_MAX;
    int Xmax = INT_MIN;
    int Ymin = INT_MAX;
    int Ymax = INT_MIN;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd()) {

            // Строка данных
            lineData = in.readLine().split("\t");

            // Парсинг
            vuX = lineData[0].trimmed().toInt();
            vuY = lineData[1].trimmed().toInt();

            if (vuX > Xmax)
                Xmax = vuX;
            if (vuX < Xmin)
                Xmin = vuX;

            if (vuY > Ymax)
                Ymax = vuY;
            if (vuY < Ymin)
                Ymin = vuY;

            xyzX = lineData[2].trimmed().toDouble();
            xyzY = lineData[3].trimmed().toDouble();
            xyzZ = lineData[4].trimmed().toDouble();

            rgbR = lineData[5].trimmed().toInt();
            rgbG = lineData[6].trimmed().toInt();
            rgbB = lineData[7].trimmed().toInt();

            clst = lineData[8].trimmed().toInt();

            // Очистка структуры
            data3DItem.vu.clear();
            data3DItem.vu.clear();

            data3DItem.xyz.clear();
            data3DItem.xyz.clear();
            data3DItem.xyz.clear();

            data3DItem.rgb.clear();
            data3DItem.rgb.clear();
            data3DItem.rgb.clear();

            // Заполнение структуры
            data3DItem.vu.push_back(vuX);
            data3DItem.vu.push_back(vuY);

            data3DItem.xyz.push_back(xyzX);
            data3DItem.xyz.push_back(xyzY);
            data3DItem.xyz.push_back(xyzZ);

            data3DItem.rgb.push_back(rgbR);
            data3DItem.rgb.push_back(rgbG);
            data3DItem.rgb.push_back(rgbB);

            data3DItem.cluster = clst;

            // Накопление данных
            data.vu.push_back(data3DItem.vu);
            data.xyz.push_back(data3DItem.xyz);
            data.rgb.push_back(data3DItem.rgb);
            data.cluster.push_back(data3DItem.cluster);
        }


        if (norm)
        {
            for (size_t i = 0; i < data.vu.size(); i++)
            {
                if (Xmax != Xmin)
                    data.vu.at(i).at(1) = cols * (data.vu.at(i).at(1) - Xmin) /
                                          (Xmax - Xmin);
                if (Ymax != Ymin)
                    data.vu.at(i).at(0) = rows * (data.vu.at(i).at(0) - Ymin) /
                                          (Ymax - Ymin);
            }
        }

    }

    return data;
}

std::vector<Cloud3DItem> MainWindow::get_cloud_3D_points(std::string pathtofile)
{
    std::vector<Cloud3DItem> cloud;

    Cloud3DItem item;

    int screenX;
    int screenY;
    double worldX;
    double worldY;
    double worldZ;

    std::string datarow;

    // TODO: Add path checking
    std::ifstream infile(pathtofile);

    // Чтения файла
    if (infile.is_open())
    {
        while (infile >> screenX >> screenY >> worldX >> worldY >> worldZ)
        {
            item.screenX = screenX;
            item.screenY = screenY;
            item.worldX = worldX;
            item.worldY = worldY;
            item.worldZ = worldZ;

            cloud.push_back(item);
        }

        infile.close();
    }

    return cloud;
}

t_vuxyzrgb MainWindow:: MainWindow::convert_cloud_3D_points(std::vector<Cloud3DItem> cloud, bool norm = true)
{
    t_vuxyzrgb data;
    Data3DItem data3DItem;

    int vuX;
    int vuY;
    double xyzX;
    double xyzY;
    double xyzZ;
    int rgbR;
    int rgbG;
    int rgbB;
    int clst;

    int Xmin = INT_MAX;
    int Xmax = INT_MIN;
    int Ymin = INT_MAX;
    int Ymax = INT_MIN;

    for (auto &item : cloud) // access by reference to avoid copying
    {
        vuX = item.screenX;
        vuY = item.screenY;
        xyzX = item.worldX;
        xyzY = item.worldY;
        xyzZ = item.worldZ;

        ///////////////////////////////////////////////////////////////////////
        // Min Max
        ///////////////////////////////////////////////////////////////////////

        if (vuX > Xmax)
            Xmax = vuX;
        if (vuX < Xmin)
            Xmin = vuX;

        if (vuY > Ymax)
            Ymax = vuY;
        if (vuY < Ymin)
            Ymin = vuY;

        ///////////////////////////////////////////////////////////////////////

        rgbR = 0;
        rgbG = 0;
        rgbB = 0;

        clst = 0;

        // Очистка структуры
        data3DItem.vu.clear();
        data3DItem.vu.clear();

        data3DItem.xyz.clear();
        data3DItem.xyz.clear();
        data3DItem.xyz.clear();

        data3DItem.rgb.clear();
        data3DItem.rgb.clear();
        data3DItem.rgb.clear();

        // Заполнение структуры
        data3DItem.vu.push_back(vuX);
        data3DItem.vu.push_back(vuY);

        data3DItem.xyz.push_back(xyzX);
        data3DItem.xyz.push_back(xyzY);
        data3DItem.xyz.push_back(xyzZ);

        data3DItem.rgb.push_back(rgbR);
        data3DItem.rgb.push_back(rgbG);
        data3DItem.rgb.push_back(rgbB);

        data3DItem.cluster = clst;

        // Накопление данных
        data.vu.push_back(data3DItem.vu);
        data.xyz.push_back(data3DItem.xyz);
        data.rgb.push_back(data3DItem.rgb);
        data.cluster.push_back(data3DItem.cluster);
    }

    // Нормирование
    //if (norm)
    //{
    //    for (size_t i = 0; i < data.vu.size(); i++)
    //    {
    //        if (Xmax != Xmin)
    //            data.vu.at(i).at(1) = cols * (data.vu.at(i).at(1) - Xmin) /
    //                                  (Xmax - Xmin);
    //        if (Ymax != Ymin)
    //            data.vu.at(i).at(0) = rows * (data.vu.at(i).at(0) - Ymin) /
    //                                  (Ymax - Ymin);
    //    }
    //}

    return data;
}

void MainWindow::on_pbScreenshot_clicked()
{
    // Создаем инструмент Линейка
    _toolWindow = new ToolWindow(this);

    // Get current Image from camera
    cv::Mat image;
    _webCamO->read(image);

    // FOR DEBUG ONLY
    // Ресайз картинки под размер камеры
    cv::Mat image_resized;
    cv::resize(image,
               image_resized,
               cv::Size(_appSet.CAMERA_WIDTH, _appSet.CAMERA_HEIGHT),
               0,
               0,
               cv::INTER_LINEAR);

    // FOR DEBUG ONLY
    // Загрузка данных
    std::vector<Cloud3DItem> cloud = get_cloud_3D_points("C:\\TEMP\\cloud_3D.txt");
    // std::vector<Cloud3DItem> cloud = get_cloud_3D_points("C:\\TEMP\\3d_points.txt");

    // Конвертация в старый формат
    t_vuxyzrgb data = convert_cloud_3D_points(cloud);

    _toolWindow->setup_window_geometry();
    // TODO: Переделать под новый формат данных
    // _toolWindow->set_data_cloud_3D(image_resized, cloud);
    _toolWindow->set_data_cloud_3D(image_resized, data);
    _toolWindow->setWindowTitle("ТНПА :: AРМ Оператора :: " + _appSet.getAppVersion());

    // Центрировать инструментальную панель
    QRect screenGeometry = QGuiApplication::screens()[0]->geometry();
    int x = (screenGeometry.width() - _toolWindow->width()) / 2;
    int y = (screenGeometry.height() - _toolWindow->height()) / 2;

    _toolWindow->move(x, y);
    _toolWindow->exec();

    // Очищаем ресурсы
    delete _toolWindow;

    ///////////////////////////////////////////////////////////////////////////
    // Реализация работы с Пашиным облоком
    /*

    // Массив данных описывающий облоко 3D точек
    t_vuxyzrgb data = get_cloud_3D_points(image.rows, image.cols);

    // Show tool window    
    _toolWindow->set_data_cloud_3D(image, data);

    // Центрировать инструментальную панель
    QRect screenGeometry = QGuiApplication::screens()[0]->geometry();
    int x = (screenGeometry.width() - _toolWindow->width()) / 2;
    int y = (screenGeometry.height() - _toolWindow->height()) / 2;
    _toolWindow->move(x, y);

    _toolWindow->exec();

    // Очищаем ресурсы
    delete _toolWindow;

    */
    ///////////////////////////////////////////////////////////////////////////
}

