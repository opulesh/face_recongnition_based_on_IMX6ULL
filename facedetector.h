#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

#include <QObject>
#include <opencv2/opencv.hpp>

class FaceDetector : public QObject
{
    Q_OBJECT

public:
    explicit FaceDetector(QObject *parent = nullptr);
    bool loadModel(const QString &modelPath);
    cv::Mat detectAndDraw(const cv::Mat &bgrImage, bool &detected);

private:
    cv::CascadeClassifier face_cascade;
    cv::Mat mat_small;
    cv::Mat mat_bgr565;
    float scale_factor;
};

#endif
