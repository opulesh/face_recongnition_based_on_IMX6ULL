QT       += core gui widgets

TARGET = face_detection_gui
TEMPLATE = app

SOURCES += \
    facedatabase.cpp \
    main.cpp \
    mainwindow.cpp \
    camerathread.cpp \
    facedetector.cpp

HEADERS += \
    facedatabase.h \
    mainwindow.h \
    camerathread.h \
    facedetector.h

INCLUDEPATH += /opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi/usr/include

LIBS += -L/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi/usr/lib \
        -lopencv_core \
        -lopencv_imgproc \
        -lopencv_objdetect \
        -lopencv_face

LIBS += -lpthread -lrt -ldl
