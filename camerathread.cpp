#include "camerathread.h"
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdio>

CameraThread::CameraThread(QObject *parent)
    : QThread(parent), v4l2_fd(-1), cam_width(800), cam_height(480), running(false), detector(nullptr)
{
}

CameraThread::~CameraThread()
{
    closeCamera();
}

bool CameraThread::openCamera(const char *device)
{
    printf("[DEBUG] openCamera: %s\n", device);
    fflush(stdout);

    if (v4l2_fd >= 0) {
        printf("[WARN] Camera already open, closing first\n");
        fflush(stdout);
        closeCamera();
    }

    if (!cameraInit(device)) {
        printf("[ERROR] cameraInit failed\n");
        fflush(stdout);
        return false;
    }
    printf("[INFO] cameraInit OK\n"); fflush(stdout);

    if (!setFormat()) {
        printf("[ERROR] setFormat failed\n");
        fflush(stdout);
        close(v4l2_fd);
        v4l2_fd = -1;
        return false;
    }
    printf("[INFO] setFormat OK\n"); fflush(stdout);

    if (!requestBuffers()) {
        printf("[ERROR] requestBuffers failed\n");
        fflush(stdout);
        close(v4l2_fd);
        v4l2_fd = -1;
        return false;
    }
    printf("[INFO] requestBuffers OK\n"); fflush(stdout);

    if (!startStream()) {
        printf("[ERROR] startStream failed\n");
        fflush(stdout);
        // unmap buffers
        for (int i = 0; i < FRAMEBUFFER_COUNT; i++) {
            if (buf_infos[i].start && buf_infos[i].length > 0) {
                munmap(buf_infos[i].start, buf_infos[i].length);
            }
        }
        close(v4l2_fd);
        v4l2_fd = -1;
        return false;
    }
    printf("[INFO] startStream OK\n"); fflush(stdout);

    mat_rgb565.create(cam_height, cam_width, CV_8UC2);
    mat_bgr.create(cam_height, cam_width, CV_8UC3);
    mat_bgr565.create(cam_height, cam_width, CV_8UC2);

    return true;
}

void CameraThread::closeCamera()
{
    printf("[DEBUG] closeCamera\n");
    fflush(stdout);
    if (v4l2_fd >= 0) {
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(v4l2_fd, VIDIOC_STREAMOFF, &type);
        for (int i = 0; i < FRAMEBUFFER_COUNT; i++) {
            if (buf_infos[i].start && buf_infos[i].length > 0) {
                munmap(buf_infos[i].start, buf_infos[i].length);
            }
        }
        close(v4l2_fd);
        v4l2_fd = -1;
    }
}

void CameraThread::startCapture()
{
    running = true;
}

void CameraThread::stopCapture()
{
    running = false;
}

void CameraThread::run()
{
    printf("[DEBUG] CameraThread::run start\n");
    fflush(stdout);

    struct v4l2_buffer buf;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    while (running) {
        for (buf.index = 0; buf.index < FRAMEBUFFER_COUNT && running; buf.index++) {
            memset(&buf, 0, sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;

            if (ioctl(v4l2_fd, VIDIOC_DQBUF, &buf) < 0) {
                if (errno == EINTR) continue;
                perror("[ERROR] VIDIOC_DQBUF");
                emit cameraError("DQBUF failed");
                break;
            }

            copyImageLines(mat_rgb565.data, buf_infos[buf.index].start,
                           mat_rgb565.step, cam_width * 2,
                           cam_width * 2, cam_height);

            cv::cvtColor(mat_rgb565, mat_bgr, cv::COLOR_BGR5652BGR);

            cv::Mat displayMat;
            if (detector) {
                bool face_detected = false;
                // 这里直接调用 detectAndDraw，它内部已经做了缩放，返回带框图像
                displayMat = detector->detectAndDraw(mat_bgr, face_detected);
            } else {
                displayMat = mat_bgr;
            }

            QImage qimg = cvMatToQImage(displayMat);
            if (!qimg.isNull()) {
                emit newFrameAvailable(qimg);
            }

            if (ioctl(v4l2_fd, VIDIOC_QBUF, &buf) < 0) {
                perror("[ERROR] VIDIOC_QBUF");
                emit cameraError("QBUF failed");
                break;
            }
        }
    }

    printf("[DEBUG] CameraThread::run end\n");
    fflush(stdout);
}

bool CameraThread::cameraInit(const char *device)
{
    struct v4l2_capability cap;

    v4l2_fd = open(device, O_RDWR);
    if (v4l2_fd < 0) {
        perror("[ERROR] open camera");
        return false;
    }

    if (ioctl(v4l2_fd, VIDIOC_QUERYCAP, &cap) < 0) {
        perror("[ERROR] VIDIOC_QUERYCAP");
        close(v4l2_fd);
        v4l2_fd = -1;
        return false;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        printf("[ERROR] Device does not support capture\n");
        close(v4l2_fd);
        v4l2_fd = -1;
        return false;
    }
    return true;
}

bool CameraThread::setFormat()
{
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = cam_width;
    fmt.fmt.pix.height = cam_height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;

    if (ioctl(v4l2_fd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("[ERROR] VIDIOC_S_FMT");
        return false;
    }

    if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB565) {
        printf("[ERROR] RGB565 not supported\n");
        return false;
    }

    cam_width = fmt.fmt.pix.width;
    cam_height = fmt.fmt.pix.height;
    printf("[INFO] Camera format: %dx%d RGB565\n", cam_width, cam_height);
    fflush(stdout);
    return true;
}

bool CameraThread::requestBuffers()
{
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = FRAMEBUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(v4l2_fd, VIDIOC_REQBUFS, &req) < 0) {
        perror("[ERROR] VIDIOC_REQBUFS");
        return false;
    }

    struct v4l2_buffer buf;
    for (int i = 0; i < FRAMEBUFFER_COUNT; i++) {
        memset(&buf, 0, sizeof(buf));
        buf.index = i;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        if (ioctl(v4l2_fd, VIDIOC_QUERYBUF, &buf) < 0) {
            perror("[ERROR] VIDIOC_QUERYBUF");
            // clean previously mapped
            for (int j = 0; j < i; j++) {
                if (buf_infos[j].start)
                    munmap(buf_infos[j].start, buf_infos[j].length);
            }
            return false;
        }
        buf_infos[i].length = buf.length;
        buf_infos[i].start = (unsigned char *)mmap(NULL, buf.length,
            PROT_READ | PROT_WRITE, MAP_SHARED, v4l2_fd, buf.m.offset);
        if (buf_infos[i].start == MAP_FAILED) {
            perror("[ERROR] mmap");
            for (int j = 0; j < i; j++) {
                if (buf_infos[j].start)
                    munmap(buf_infos[j].start, buf_infos[j].length);
            }
            return false;
        }
    }

    for (int i = 0; i < FRAMEBUFFER_COUNT; i++) {
        memset(&buf, 0, sizeof(buf));
        buf.index = i;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        if (ioctl(v4l2_fd, VIDIOC_QBUF, &buf) < 0) {
            perror("[ERROR] VIDIOC_QBUF");
            return false;
        }
    }
    return true;
}

bool CameraThread::startStream()
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(v4l2_fd, VIDIOC_STREAMON, &type) < 0) {
        perror("[ERROR] VIDIOC_STREAMON");
        return false;
    }
    return true;
}

void CameraThread::copyImageLines(unsigned char *dst, const unsigned char *src,
                                  int dstStep, int srcStep, int lineBytes, int height)
{
    for (int i = 0; i < height; i++) {
        memcpy(dst + i * dstStep, src + i * srcStep, lineBytes);
    }
}

QImage CameraThread::cvMatToQImage(const cv::Mat &mat)
{
    if (mat.type() == CV_8UC3) {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    }
    return QImage();
}
