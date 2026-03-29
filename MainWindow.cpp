#include "MainWindow.h"
#include <QFileDialog>
#include <QMouseEvent>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    this->setWindowTitle("FlashPrint Enhancer (C++)");
    this->setFixedSize(900, 700);
    this->setStyleSheet("background-color: #1e1e1e; color: white;"); // Apple Dark Mode vibe

    imageLabel = new QLabel(this);
    imageLabel->setGeometry(50, 50, 800, 500);
    imageLabel->setStyleSheet("border: 2px dashed #555; border-radius: 10px;");

    QPushButton *btnLoad = new QPushButton("Load Image", this);
    btnLoad->setGeometry(50, 580, 200, 50);
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadImage);

    QPushButton *btnCrop = new QPushButton("Crop Document", this);
    btnCrop->setGeometry(350, 580, 200, 50);
    connect(btnCrop, &QPushButton::clicked, this, &MainWindow::processCrop);

    QPushButton *btnEnhance = new QPushButton("Enhance Quality", this);
    btnEnhance->setGeometry(650, 580, 200, 50);
    connect(btnEnhance, &QPushButton::clicked, this, &MainWindow::enhanceImage);
}

void MainWindow::loadImage() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select Photo");
    if (!fileName.isEmpty()) {
        rawImage = cv::imread(fileName.toStdString());
        points.clear();
        updateDisplay();
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (points.size() < 4 && !rawImage.empty()) {
        // Map click to image coordinates (simplified)
        points.push_back(cv::Point2f(event->x() - 50, event->y() - 50));
        updateDisplay();
    }
}

void MainWindow::updateDisplay() {
    if (rawImage.empty()) return;
    cv::Mat temp;
    rawImage.copyTo(temp);
    
    for (auto p : points) {
        cv::circle(temp, p, 10, cv::Scalar(0, 255, 0), -1); // Draw Green Dots
    }

    QImage qimg(temp.data, temp.cols, temp.rows, temp.step, QImage::Format_BGR888);
    imageLabel->setPixmap(QPixmap::fromImage(qimg).scaled(imageLabel->size(), Qt::KeepAspectRatio));
}

void MainWindow::processCrop() {
    if (points.size() == 4) {
        cv::Point2f dst_pts[] = {{0,0}, {400,0}, {400,600}, {0,600}};
        cv::Mat M = cv::getPerspectiveTransform(points.data(), dst_pts);
        cv::warpPerspective(rawImage, processedImage, M, cv::Size(400, 600));
        
        rawImage = processedImage; // Focus on cropped area
        points.clear();
        updateDisplay();
    }
}

void MainWindow::enhanceImage() {
    if (processedImage.empty()) return;
    cv::cvtColor(processedImage, processedImage, cv::COLOR_BGR2GRAY);
    cv::adaptiveThreshold(processedImage, processedImage, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 11, 2);
    updateDisplay();
}