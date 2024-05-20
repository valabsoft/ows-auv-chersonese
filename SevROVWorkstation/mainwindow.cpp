#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QScreen>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Загрузка настроек
    _appSet.load();

    setWindowIcon(QIcon(":/img/sevrov.png"));

    // Фиксируем размер окна и убираем иконку ресайза
    setFixedSize(QSize(800 - 10, 500));

    // Установка иконок для кнопок
    ui->pbSettings->setIcon(QIcon(":/img/settings_icon.png"));
    ui->pbSettings->setIconSize(QSize(64, 64));
    ui->pbStartStop->setIcon(QIcon(":/img/off_button_icon.png"));
    ui->pbStartStop->setIconSize(QSize(64, 64));
    ui->pbView->setIcon(QIcon(":/img/display_icon.png"));
    ui->pbView->setIconSize(QSize(64, 64));
    ui->pbScreenshot->setIcon(QIcon(":/img/camera_icon.png"));
    ui->pbScreenshot->setIconSize(QSize(64, 64));

    // Центрируем окно в пределах экрана
    move(screen()->geometry().center() - frameGeometry().center());

    // TODO: После подключения общей библиотеки, использовать
    // setWindowTitle("ТНПА :: Контроль :: " + QString(APP_VERSION.c_str()));
    setWindowTitle("ТНПА :: AРМ Оператора :: " + _appSet.getAppVersion());

    // Геометрия окон камер
    ui->lbCamera->setGeometry(20, 10, 640, 480);
    ui->lbCameraL->setGeometry(10, 10, 320, 240);
    ui->lbCameraR->setGeometry(10 + 320 + 10, 10, 320, 240);

    // Layout по умолчанию - одиночная камера
    setSingleCameraLayout();
    setConnectedCtrlStyle(false);

    // Цвет фона главного окна приложения
    this->setStyleSheet("background-color: black;");    
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pbStartStop_clicked()
{
    // Меняем состояние флага
    _sevROV.isConnected = !_sevROV.isConnected;

    // Меняем иконку на кнопке
    if (_sevROV.isConnected)
    {
        ui->pbStartStop->setIcon(QIcon(":/img/on_button_icon.png"));
        ui->pbStartStop->setIconSize(QSize(64, 64));
    }
    else
    {
        ui->pbStartStop->setIcon(QIcon(":/img/off_button_icon.png"));
        ui->pbStartStop->setIconSize(QSize(64, 64));
    }

    setConnectedCtrlStyle(_sevROV.isConnected);
}
void MainWindow::on_pbView_clicked()
{
    // Если нет соединения - выход
    if (!_sevROV.isConnected)
        return;

    switch (_sevROV.viewMode)
    {
    case ViewMode::SINGLE:
        ui->pbView->setIcon(QIcon(":/img/video_icon.png"));
        ui->pbView->setIconSize(QSize(64, 64));
        _sevROV.viewMode = ViewMode::STEREO;

        setStereoCameraLayout();
        break;
    case ViewMode::STEREO:
        ui->pbView->setIcon(QIcon(":/img/display_icon.png"));
        ui->pbView->setIconSize(QSize(64, 64));
        _sevROV.viewMode = ViewMode::SINGLE;

        setSingleCameraLayout();
        break;
    default:
        break;
    }
}
void MainWindow::setSingleCameraLayout()
{
    ui->lbCamera->setVisible(true);
    ui->lbCameraL->setVisible(false);
    ui->lbCameraR->setVisible(false);
}
void MainWindow::setStereoCameraLayout()
{
    ui->lbCamera->setVisible(false);
    ui->lbCameraL->setVisible(true);
    ui->lbCameraR->setVisible(true);
}
void MainWindow::setConnectedCtrlStyle(bool isconnected)
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

