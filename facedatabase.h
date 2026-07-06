#ifndef FACEDATABASE_H
#define FACEDATABASE_H

// 解决 Qt 的 emit 宏与 OpenCV face 模块中的 emit 函数冲突
#ifdef emit
#  define QT_EMIT_WAS_DEFINED
#  undef emit
#endif

#include <opencv2/face.hpp>

// 恢复 emit 宏（如果之前被定义了）
#ifdef QT_EMIT_WAS_DEFINED
#  define emit
#  undef QT_EMIT_WAS_DEFINED
#endif

#include <opencv2/opencv.hpp>
#include <QString>
#include <QMap>
#include <vector>


class FaceDatabase
{
public:
    FaceDatabase();
    bool loadModel(const std::string &modelPath);
    bool saveModel(const std::string &modelPath);

    // 注册新用户: 提供一组人脸灰度图，返回分配的ID
    int registerNewUser(const std::vector<cv::Mat> &faceImages, const QString &name);

    // 识别：输入一张人脸灰度图，返回姓名和置信度(越低越好)
    QString recognize(const cv::Mat &faceImage, double &confidence);

    bool isTrained() const { return m_trained; }

private:
    cv::Ptr<cv::face::LBPHFaceRecognizer> m_recognizer;
    QMap<int, QString> m_id2name;   // ID -> 名字
    int m_nextId = 1;
    std::string m_modelPath;        // 模型文件路径
    bool m_trained = false;         //检测模型是否已经训练

    void loadLabelMap(const std::string &labelPath);
    void saveLabelMap(const std::string &labelPath);
};

#endif
