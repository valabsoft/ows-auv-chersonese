QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    applicationsettings.cpp \
    main.cpp \
    mainwindow.cpp \
    sevrovcontroller.cpp

HEADERS += \
    applicationsettings.h \
    enumclasses.h \
    mainwindow.h \
    sevrovcontroller.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# OpenCV include path and libs

win32 {
    INCLUDEPATH += C:\OpenCV\build\include

    LIBS += C:\opencv-build\bin\libopencv_core455.dll
    LIBS += C:\opencv-build\bin\libopencv_highgui455.dll
    LIBS += C:\opencv-build\bin\libopencv_imgcodecs455.dll
    LIBS += C:\opencv-build\bin\libopencv_features2d455.dll
    LIBS += C:\opencv-build\bin\libopencv_calib3d455.dll
    LIBS += C:\opencv-build\bin\libopencv_videoio455.dll
    LIBS += C:\opencv-build\bin\libopencv_imgproc455.dll
}
unix {
    INCLUDEPATH += /usr/include/opencv4
    LIBS += -L/usr/local/lib -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_features2d -lopencv_calib3d -lopencv_videoio -lopencv_imgproc
}

RESOURCES += \
    SevROVWorkstation.qrc
