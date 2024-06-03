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

void MainWindow::setup_window_geometry()
{
    // Установка размера главного окна// Установка размера главного окна
    int windowWidth = _appSet.CAMERA_WIDTH + _appSet.CONTROL_PANEL_WIDTH + _appSet.CAMERA_VIEW_BORDER_WIDTH * 4;
    int windowHeight = _appSet.CAMERA_HEIGHT + _appSet.CAMERA_VIEW_BORDER_WIDTH * 2;

    // Фиксируем размер окна и убираем иконку ресайза
    setFixedSize(QSize(windowWidth, windowHeight));

    // Центрируем окно в пределах экрана
    move(screen()->geometry().center() - frameGeometry().center());

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
    int camID = 0;
    int camIDL = 0;
    int camIDR = 0;

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

        _webCamO = new cv::VideoCapture(camID);
        _webCamL = new cv::VideoCapture(camIDL);
        _webCamR = new cv::VideoCapture(camIDR);

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

void MainWindow::on_video_timer()
{
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

        _imgCamO = QImage((uchar*) _destinationMatO.data,
                          _destinationMatO.cols,
                          _destinationMatO.rows,
                          _destinationMatO.step,
                          QImage::Format_RGB888);

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
}

