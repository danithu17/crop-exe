#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QImage>
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

private:
    QLabel *imageLabel;
    cv::Mat rawImage;
    cv::Mat processedImage;
    std::vector<cv::Point2f> points;
    void updateDisplay();
};

#endif