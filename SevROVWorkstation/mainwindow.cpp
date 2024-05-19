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

    // Обводка внешней границы
    ui->lbCamera->setStyleSheet("QLabel {"
                                "border-style: solid;"
                                "border-width: 1px;"
                                "border-color: yellow; "
                                "}");

    ui->gbControlButtons->setStyleSheet("QGroupBox {"
                                        "border-style: solid;"
                                        "border-width: 1px;"
                                        "border-color: silver; "
                                        "}");

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
        break;
    case ViewMode::STEREO:
        ui->pbView->setIcon(QIcon(":/img/display_icon.png"));
        ui->pbView->setIconSize(QSize(64, 64));
        _sevROV.viewMode = ViewMode::SINGLE;
        break;
    default:
        break;
    }
}

