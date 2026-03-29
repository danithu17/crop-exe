#include "MainWindow.h"
#include <QFileDialog>
#include <QMouseEvent>
#include <algorithm> 

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("FlashPrint Pro - High Quality");
    setMinimumSize(1000, 850);
    
    setStyleSheet(
        "QMainWindow { background-color: #000000; }"
        "QPushButton { background-color: #1c1c1e; color: #0A84FF; border-radius: 15px; font-size: 14px; font-weight: bold; border: 1px solid #3a3a3c; }"
        "QPushButton:hover { background-color: #2c2c2e; border: 1px solid #0A84FF; }"
        "QLabel#ImageCanvas { background-color: #1c1c1e; border: 2px solid #2c2c2e; border-radius: 25px; }"
    );

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(40, 20, 40, 40);

    QLabel *header = new QLabel("FlashPrint", this);
    header->setStyleSheet("font-size: 32px; font-weight: 800; color: white; margin-bottom: 10px;");
    layout->addWidget(header, 0, Qt::AlignCenter);

    imageLabel = new QLabel(this);
    imageLabel->setObjectName("ImageCanvas");
    imageLabel->setFixedSize(850, 550);
    imageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(imageLabel, 1, Qt::AlignCenter);

    statusLabel = new QLabel("READY TO SCAN", this);
    statusLabel->setStyleSheet("color: #8e8e93; letter-spacing: 2px; font-size: 12px;");
    layout->addWidget(statusLabel, 0, Qt::AlignCenter);

    QHBoxLayout *btns = new QHBoxLayout();
    btns->setSpacing(20);

    QPushButton *b1 = new QPushButton("OPEN PHOTO", this);
    QPushButton *b2 = new QPushButton("CROP DOCUMENT", this);
    QPushButton *b3 = new QPushButton("ENHANCE", this);
    QPushButton *b4 = new QPushButton("SAVE TO PC", this);

    b1->setFixedSize(180, 55); b2->setFixedSize(180, 55);
    b3->setFixedSize(180, 55); b4->setFixedSize(180, 55);

    btns->addWidget(b1); btns->addWidget(b2); btns->addWidget(b3); btns->addWidget(b4);
    layout->addLayout(btns);

    connect(b1, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(b2, &QPushButton::clicked, this, &MainWindow::processCrop);
    connect(b3, &QPushButton::clicked, this, &MainWindow::enhanceImage);
    connect(b4, &QPushButton::clicked, this, &MainWindow::saveImage);
}

void MainWindow::loadImage() {
    QString path = QFileDialog::getOpenFileName(this, "Select Image", "", "Images (*.jpg *.png)");
    if (!path.isEmpty()) {
        rawImage = cv::imread(path.toStdString());
        points.clear();
        updateDisplay(rawImage);
        statusLabel->setText("TAP 4 CORNERS OF THE DOCUMENT");
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (rawImage.empty()) return;
    QPoint p = imageLabel->mapFromParent(event->pos());
    if (imageLabel->rect().contains(p) && points.size() < 4) {
        float sx = (float)rawImage.cols / imageLabel->width();
        float sy = (float)rawImage.rows / imageLabel->height();
        points.push_back(cv::Point2f(p.x() * sx, p.y() * sy));
        updateDisplay(rawImage);
    }
}

void MainWindow::updateDisplay(const cv::Mat& img) {
    if (img.empty()) return;
    cv::Mat t; img.copyTo(t);
    for (auto p : points) cv::circle(t, p, 25, cv::Scalar(0, 255, 0), -1);
    cv::Mat rgb; cv::cvtColor(t, rgb, cv::COLOR_BGR2RGB);
    QImage qi(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
    imageLabel->setPixmap(QPixmap::fromImage(qi).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

// Helper to order points
void orderPoints(std::vector<cv::Point2f>& pts) {
    std::sort(pts.begin(), pts.end(), [](const cv::Point2f& a, const cv::Point2f& b) { return a.x < b.x; });
    std::vector<cv::Point2f> left = {pts[0], pts[1]}, right = {pts[2], pts[3]};
    std::sort(left.begin(), left.end(), [](const cv::Point2f& a, const cv::Point2f& b) { return a.y < b.y; });
    std::sort(right.begin(), right.end(), [](const cv::Point2f& a, const cv::Point2f& b) { return a.y < b.y; });
    pts = {left[0], right[0], right[1], left[1]};
}

void MainWindow::processCrop() {
    if (points.size() == 4 && !rawImage.empty()) {
        orderPoints(points);
        float w1 = std::sqrt(std::pow(points[2].x - points[3].x, 2) + std::pow(points[2].y - points[3].y, 2));
        float w2 = std::sqrt(std::pow(points[1].x - points[0].x, 2) + std::pow(points[1].y - points[0].y, 2));
        int mW = std::max((int)w1, (int)w2);
        float h1 = std::sqrt(std::pow(points[1].x - points[2].x, 2) + std::pow(points[1].y - points[2].y, 2));
        float h2 = std::sqrt(std::pow(points[0].x - points[3].x, 2) + std::pow(points[0].y - points[3].y, 2));
        int mH = std::max((int)h1, (int)h2);

        cv::Point2f dst[] = {{0,0}, {(float)mW, 0}, {(float)mW, (float)mH}, {0, (float)mH}};
        cv::Mat M = cv::getPerspectiveTransform(points.data(), dst);
        cv::warpPerspective(rawImage, processedImage, M, cv::Size(mW, mH));
        
        points.clear();
        updateDisplay(processedImage);
        statusLabel->setText("CROP COMPLETE");
    }
}

void MainWindow::enhanceImage() {
    if (processedImage.empty()) return;
    cv::Mat gray, enhanced;
    cv::cvtColor(processedImage, gray, cv::COLOR_BGR2GRAY);
    
    // Step 1: Remove Noise
    cv::GaussianBlur(gray, gray, cv::Size(3, 3), 0);
    
    // Step 2: Adaptive Threshold for scan effect
    cv::adaptiveThreshold(gray, enhanced, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 15, 8);
    
    // Step 3: Morphological operation to bold the text
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
    cv::erode(enhanced, enhanced, kernel); 

    cv::cvtColor(enhanced, processedImage, cv::COLOR_GRAY2BGR);
    updateDisplay(processedImage);
    statusLabel->setText("SCAN QUALITY ENHANCED");
}

void MainWindow::saveImage() {
    if (processedImage.empty()) return;
    QString s = QFileDialog::getSaveFileName(this, "Save Image", "", "PNG (*.png)");
    if (!s.isEmpty()) {
        cv::imwrite(s.toStdString(), processedImage);
        statusLabel->setText("SAVED TO DISK");
    }
}