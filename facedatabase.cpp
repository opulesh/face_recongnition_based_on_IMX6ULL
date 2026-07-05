#include "facedatabase.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <opencv2/face.hpp>

FaceDatabase::FaceDatabase()
{
    // OpenCV 3.1 使用 createLBPHFaceRecognizer 函数
    m_recognizer = cv::face::createLBPHFaceRecognizer(1, 8, 8, 8, 120.0);
    m_nextId = 1;
}

bool FaceDatabase::loadModel(const std::string &modelPath)
{
    m_modelPath = modelPath;

    cv::FileStorage fs(modelPath, cv::FileStorage::READ);
    if (!fs.isOpened())
        return false;

    m_recognizer->read(fs.root());
    fs.release();

    // 加载标签映射
    std::string labelPath = modelPath + ".labels";
    loadLabelMap(labelPath);
    return true;
}

bool FaceDatabase::saveModel(const std::string &modelPath)
{
    cv::FileStorage fs(modelPath, cv::FileStorage::WRITE);
    if (!fs.isOpened())
        return false;

    m_recognizer->write(fs);
    fs.release();

    // 保存标签映射
    std::string labelPath = modelPath + ".labels";
    saveLabelMap(labelPath);
    return true;
}

int FaceDatabase::registerNewUser(const std::vector<cv::Mat> &faceImages, const QString &name)
{
    if (faceImages.empty() || name.isEmpty())
        return -1;

    int id = m_nextId++;
    m_id2name[id] = name;

    std::vector<int> labels(faceImages.size(), id);
    // 如果标签映射为空（没有旧数据），调用 train；否则 update
    if (m_id2name.size() == 1) {   // 刚添加完第一个用户，之前为空
        m_recognizer->train(faceImages, labels);
    } else {
        m_recognizer->update(faceImages, labels);
    }

    saveModel(m_modelPath);
    return id;
}

QString FaceDatabase::recognize(const cv::Mat &faceImage, double &confidence)
{
    if (faceImage.empty()) return "Unknown";

    int predictedLabel = -1;
    m_recognizer->predict(faceImage, predictedLabel, confidence);
    // LBPH 的置信度是距离，越小越相似。通常阈值设为 80 左右
    if (predictedLabel > 0 && confidence < 80.0) {
        return m_id2name.value(predictedLabel, "Unknown");
    }
    return "Unknown";
}

void FaceDatabase::loadLabelMap(const std::string &labelPath)
{
    QFile file(QString::fromStdString(labelPath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    m_id2name.clear();
    int maxId = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(':');
        if (parts.size() == 2) {
            int id = parts[0].toInt();
            QString name = parts[1];
            m_id2name[id] = name;
            if (id > maxId) maxId = id;
        }
    }
    m_nextId = maxId + 1;
}

void FaceDatabase::saveLabelMap(const std::string &labelPath)
{
    QFile file(QString::fromStdString(labelPath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    for (auto it = m_id2name.begin(); it != m_id2name.end(); ++it) {
        out << it.key() << ":" << it.value() << "\n";
    }
}
