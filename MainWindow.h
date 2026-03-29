#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>
#include <opencv2/opencv.hpp>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
protected:
    void mousePressEvent(QMouseEvent *event) override;
private slots:
    void loadImage();
    void processCrop();
    void enhanceImage();
    void saveImage();
private:
    QLabel *imageLabel, *statusLabel;
    cv::Mat rawImage, processedImage;
    std::vector<cv::Point2f> points;
    void updateDisplay(const cv::Mat& img);
};
#endif