#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QMutex>
#include <opencv2/opencv.hpp>

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
};

#endif
