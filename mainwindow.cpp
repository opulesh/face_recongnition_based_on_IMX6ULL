#include "mainwindow.h"
#include "camerathread.h"
#include "facedetector.h"
#include <QVBoxLayout>
#include <QTimer>
#include <cstdio>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , isRunning(false)
    , framePending(false)
{
    printf("[DEBUG] MainWindow start\n");
    fflush(stdout);

    setWindowTitle("Face Detection");

    // 直接全屏，无边框
    showFullScreen();

    // 全屏布局：只有一个 QLabel 充满整个窗口
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    videoLabel = new QLabel(this);
    videoLabel->setStyleSheet("background-color: black;");
    videoLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(videoLabel);

    // 摄像头和检测器
    camera = new CameraThread(this);
    detector = new FaceDetector(this);
    camera->setDetector(detector);

    // 刷新定时器（30fps）
    refreshTimer = new QTimer(this);
    refreshTimer->setInterval(33);
    connect(refreshTimer, &QTimer::timeout, this, &MainWindow::refreshDisplay);
    refreshTimer->start();

    // 摄像头信号
    connect(camera, &CameraThread::newFrameAvailable,
            this, &MainWindow::onNewFrame);
    connect(camera, &CameraThread::cameraError,
            this, [](const QString &err) {
                printf("[ERROR] %s\n", err.toLocal8Bit().constData());
                fflush(stdout);
            });

    // 加载人脸模型
    if (!detector->loadModel("/usr/haarcascade_frontalface_default.xml"))
        printf("[WARN] Face model not loaded\n");

    // 自动启动摄像头
    QTimer::singleShot(100, this, &MainWindow::startCamera);

    printf("[DEBUG] MainWindow end\n");
    fflush(stdout);
}

MainWindow::~MainWindow()
{
    if (isRunning) {
        camera->stopCapture();
        camera->wait();
    }
}

void MainWindow::startCamera()
{
    printf("[INFO] Auto-starting camera...\n");
    fflush(stdout);
    if (!camera->openCamera("/dev/video1")) {
        printf("[ERROR] Cannot open /dev/video1\n");
        fflush(stdout);
        return;
    }
    camera->startCapture();
    camera->start();
    isRunning = true;
}

void MainWindow::onNewFrame(const QImage &frame)
{
    QImage scaled = frame.scaled(videoLabel->size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    {
        QMutexLocker locker(&frameMutex);
        latestFrame = scaled;
        framePending = true;
    }
}

void MainWindow::refreshDisplay()
{
    if (framePending) {
        QMutexLocker locker(&frameMutex);
        if (!latestFrame.isNull()) {
            videoLabel->setPixmap(QPixmap::fromImage(latestFrame));
        }
        framePending = false;
    }
}

QImage MainWindow::cvMatToQImage(const cv::Mat &mat)
{
    if (mat.type() == CV_8UC3) {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    }
    return QImage();
}
