#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QMutex>
#include <opencv2/opencv.hpp>
#include <QPushButton>
#include <QInputDialog>
#include "facedatabase.h"
#include <vector>

class CameraThread;
class FaceDetector;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewFrame(const QImage &frame);
    void refreshDisplay();
    void startCamera();

private:
    QLabel *videoLabel;
    CameraThread *camera;
    FaceDetector *detector;

    QTimer *refreshTimer;
    bool isRunning;

    QImage latestFrame;
    QMutex frameMutex;
    bool framePending;

    QImage cvMatToQImage(const cv::Mat &mat);

    QWidget *controlBar;
    QPushButton *btnRegister;
    QPushButton *btnMode;       // 切换识别/注册模式
    QLabel *labelResult;
    QLabel *labelMode;

    FaceDatabase *faceDB;       // 人脸数据库
    bool isRegisterMode;        // 当前模式
    int registerCounter;        // 注册采集计数
    std::vector<cv::Mat> registerFaces;
    QString registerName;

    int m_autoRegisterCounter;
};

#endif
