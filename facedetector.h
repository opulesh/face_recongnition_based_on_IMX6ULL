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
    std::vector<cv::Rect> getLastFaces() const { return m_lastFaces; }

private:
    cv::CascadeClassifier face_cascade;
    cv::Mat mat_small;
    cv::Mat mat_bgr565;
    float scale_factor;
    std::vector<cv::Rect> m_lastFaces;
};

#endif
