#include "facedetector.h"

FaceDetector::FaceDetector(QObject *parent)
    : QObject(parent), scale_factor(0.25f)
{
}

bool FaceDetector::loadModel(const QString &modelPath)
{
    return face_cascade.load(modelPath.toStdString());
}

cv::Mat FaceDetector::detectAndDraw(const cv::Mat &bgrImage, bool &detected)
{
    cv::Mat result = bgrImage.clone();
    std::vector<cv::Rect> faces;
    cv::resize(bgrImage, mat_small, cv::Size(), scale_factor, scale_factor, cv::INTER_LINEAR);
    face_cascade.detectMultiScale(mat_small, faces, 1.2, 3, 0, cv::Size(40, 40));
    detected = (faces.size() > 0);
    m_lastFaces.clear();
    if (detected) {
        for (size_t i = 0; i < faces.size(); i++) {
            faces[i].x = (int)(faces[i].x / scale_factor);
            faces[i].y = (int)(faces[i].y / scale_factor);
            faces[i].width = (int)(faces[i].width / scale_factor);
            faces[i].height = (int)(faces[i].height / scale_factor);

            cv::rectangle(result, faces[i], cv::Scalar(0, 255, 0), 2);
        }
        m_lastFaces = faces;
    }

    return result;
}
