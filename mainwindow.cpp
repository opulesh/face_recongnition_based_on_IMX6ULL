#include "mainwindow.h"
#include "camerathread.h"
#include "facedetector.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <cstdio>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , isRunning(false)
    , framePending(false)
    , m_autoRegisterCounter(1)
{
    printf("[DEBUG] MainWindow start\n");
    fflush(stdout);

    setWindowTitle("Face Detection");

    // ======================== 界面布局 (叠加方式) ========================
    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 视频标签 (填满整个窗口)
    videoLabel = new QLabel(this);
    videoLabel->setStyleSheet("background-color: black;");
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true); // 允许事件穿透
    layout->addWidget(videoLabel, 0, 0);

    // 控制条 (半透明，悬浮在顶部)
    controlBar = new QWidget(this);
    controlBar->setStyleSheet("background-color: rgba(255, 255, 255, 50);");
    controlBar->setFixedHeight(50);
    QHBoxLayout *ctlLayout = new QHBoxLayout(controlBar);
    ctlLayout->setContentsMargins(10, 5, 10, 5);

    btnMode = new QPushButton("Recognition", controlBar);
    btnMode->setFixedSize(120, 30);
    btnMode->setStyleSheet("color: white; background-color: #555; border: 1px solid #888; border-radius: 5px;");
    btnRegister = new QPushButton("Register", controlBar);
    btnRegister->setFixedSize(120, 30);
    btnRegister->setStyleSheet("color: white; background-color: #555; border: 1px solid #888; border-radius: 5px;");

    labelMode = new QLabel("Mode: Recognition", controlBar);
    labelMode->setStyleSheet("color: white;");
    labelResult = new QLabel("", controlBar);
    labelResult->setStyleSheet("color: yellow; font-size: 18px;");

    ctlLayout->addWidget(btnMode);
    ctlLayout->addWidget(btnRegister);
    ctlLayout->addWidget(labelMode);
    ctlLayout->addStretch();
    ctlLayout->addWidget(labelResult);

    layout->addWidget(controlBar, 0, 0, Qt::AlignTop | Qt::AlignLeft);

    // ======================== 功能模块 ========================
    camera = new CameraThread(this);
    detector = new FaceDetector(this);
    camera->setDetector(detector);

    faceDB = new FaceDatabase();
    faceDB->loadModel("/opt/face_model.yml");
    camera->setFaceDatabase(faceDB);   // 传入 faceDB
    if (faceDB->isTrained()) {
        // 模型已训练，默认识别模式
        camera->setRecognizeMode(true);
        isRegisterMode = false;
        btnMode->setText("Register New");
        labelMode->setText("Mode: Recognition");
    } else {
        // 模型未训练，自动进入注册模式
        camera->setRecognizeMode(false);
        isRegisterMode = true;
        registerName = QString("user%1").arg(m_autoRegisterCounter++);
        camera->triggerRegister(registerName);
        btnMode->setText("Back to Recognize");
        labelMode->setText("Mode: Register");
        labelResult->setText("Registering: " + registerName);
    }

    isRegisterMode = false;
    registerCounter = 0;

    // ======================== 定时器 ========================
    refreshTimer = new QTimer(this);
    refreshTimer->setInterval(33); // 30fps
    connect(refreshTimer, &QTimer::timeout, this, &MainWindow::refreshDisplay);
    refreshTimer->start();

    // ======================== 信号连接 ========================
    connect(camera, &CameraThread::newFrameAvailable,
            this, &MainWindow::onNewFrame);
    connect(camera, &CameraThread::cameraError,
            this, [](const QString &err) {
                printf("[ERROR] %s\n", err.toLocal8Bit().constData());
                fflush(stdout);
            });
    connect(camera, &CameraThread::recognized, this, [this](const QString &name, double /*conf*/) {
        labelResult->setText(name);
    });
    connect(camera, &CameraThread::registerFinished, this, [this](bool ok, const QString &name) {
        printf("registerFinished triggered\n");
        if (ok)
            labelResult->setText("Registered: " + name);
        else
            labelResult->setText("Registration failed");
        // 注册完毕，自动切回识别模式
        isRegisterMode = false;
        btnMode->setText("Recognition");
        labelMode->setText("Mode: Recognition");
        camera->setRecognizeMode(true);
    });

    // ======================== 按钮动作 ========================
    connect(btnMode, &QPushButton::clicked, [this]() {
        if (!isRegisterMode) {
            switchToRegisterMode();
        } else {
            switchToRecognizeMode();
        }
    });

    connect(btnRegister, &QPushButton::clicked, [this]() {
        if (!isRegisterMode) return;
        registerCounter = 0;
        registerFaces.clear();
        camera->triggerRegister(registerName);
        labelResult->setText("Registering: " + registerName + " (retake)");
    });

    connect(camera, &CameraThread::registerProgress, this, [this](int count) {
        printf("[GUI] registerProgress: %d\n", count);
        fflush(stdout);
        labelResult->setText(QString("Registering: %1 (%2/10)")
                             .arg(registerName).arg(count));
    });

    // ======================== 启动 ========================
    if (!detector->loadModel("/usr/haarcascade_frontalface_default.xml"))
        printf("[WARN] Face model not loaded\n");

    showFullScreen();
    QTimer::singleShot(100, this, &MainWindow::startCamera);
    printf("[DEBUG] MainWindow end\n");
    fflush(stdout);
}

void MainWindow::switchToRegisterMode()
{
    registerName = QString("user%1").arg(m_autoRegisterCounter++);
    registerCounter = 0;
    registerFaces.clear();
    isRegisterMode = true;
    btnMode->setText("Back to Recognize");
    labelMode->setText("Mode: Register");
    labelResult->setText("Registering: " + registerName);
    camera->setRecognizeMode(false);
    camera->triggerRegister(registerName);
}

void MainWindow::switchToRecognizeMode()
{
    isRegisterMode = false;
    btnMode->setText("Register New");
    labelMode->setText("Mode: Recognition");
    labelResult->setText("");
    camera->setRecognizeMode(true);
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
