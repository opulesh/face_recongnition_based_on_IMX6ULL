#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QThread>
#include <QMutex>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <linux/videodev2.h>
#include "facedetector.h"
#include "facedatabase.h"

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

    void setDetector(FaceDetector *detector) { m_detector = detector; }

    void setRecognizeMode(bool enabled) { m_recognizeMode = enabled; }
    void setFaceDatabase(FaceDatabase *db) { m_faceDB = db; }
    void triggerRegister(const QString &name) {
        printf("[DEBUG] triggerRegister: name=%s\n", name.toLocal8Bit().constData());
        fflush(stdout);
        m_registerRequest = true;
        m_registerName = name;
        m_registerCounter = 0;   // 确保每次触发都重置计数器
        m_registerFaces.clear();
    }

signals:
    void newFrameAvailable(const QImage &frame);
    void cameraError(const QString &error);
    void recognized(const QString &name, double confidence);
    void registerProgress(int count);
    void registerFinished(bool success, const QString &name);

protected:
    void run() override;

private:


    int v4l2_fd;
    cam_buf_info buf_infos[FRAMEBUFFER_COUNT];
    int cam_width;
    int cam_height;
    volatile bool running;
    FaceDetector *m_detector;
    bool m_recognizeMode;
    FaceDatabase *m_faceDB;
    bool m_registerRequest;
    QString m_registerName;
    std::vector<cv::Mat> m_registerFaces;
    int m_registerCounter;

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
