#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QThread>
#include <QMutex>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <linux/videodev2.h>
#include "facedetector.h"

#define FRAMEBUFFER_COUNT 3

typedef struct {
    unsigned char *start;
    unsigned long length;
} cam_buf_info;

class CameraThread : public QThread
{
    Q_OBJECT

public:
    explicit CameraThread(QObject *parent = nullptr);
    ~CameraThread();

    bool openCamera(const char *device);
    void closeCamera();
    void startCapture();
    void stopCapture();

    void setDetector(FaceDetector *detector) { this->detector = detector; }

signals:
    void newFrameAvailable(const QImage &frame);
    void cameraError(const QString &error);

protected:
    void run() override;

private:
    FaceDetector *detector;

    int v4l2_fd;
    cam_buf_info buf_infos[FRAMEBUFFER_COUNT];
    int cam_width;
    int cam_height;
    volatile bool running;

    QMutex frameMutex;
    cv::Mat mat_rgb565;
    cv::Mat mat_bgr;
    cv::Mat mat_bgr565;

    bool cameraInit(const char *device);
    bool setFormat();
    bool requestBuffers();
    bool startStream();
    void copyImageLines(unsigned char *dst, const unsigned char *src,
                        int dstStep, int srcStep, int lineBytes, int height);
    QImage cvMatToQImage(const cv::Mat &mat);
};

#endif
