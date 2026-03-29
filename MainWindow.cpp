#include "MainWindow.h"
#include <QFileDialog>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("FlashPrint Enhancer v1.0");
    setMinimumSize(1000, 800);
    applyAppleStyle();

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(20);

    // Header Label
    QLabel *titleLabel = new QLabel("FlashPrint Enhancer", this);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #FFFFFF;");
    mainLayout->addWidget(titleLabel, 0, Qt::AlignCenter);

    // Image Display Area (Glassmorphism effect placeholder)
    imageLabel = new QLabel("Load an image to start cropping...", this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("background-color: #2c2c2e; border: 2px solid #3a3a3c; border-radius: 20px; color: #8e8e93; font-size: 16px;");
    imageLabel->setFixedSize(800, 500);
    mainLayout->addWidget(imageLabel, 1, Qt::AlignCenter);

    statusLabel = new QLabel("Ready", this);
    statusLabel->setStyleSheet("color: #0A84FF; font-weight: bold;");
    mainLayout->addWidget(statusLabel, 0, Qt::AlignCenter);

    // Control Buttons Container
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);

    auto createStyledBtn = [&](QString text, QString color) {
        QPushButton *btn = new QPushButton(text, this);
        btn->setFixedSize(180, 50);
        btn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: white; border-radius: 12px; font-weight: bold; font-size: 14px; border: none; }"
            "QPushButton:hover { background-color: #5AC8FA; }"
            "QPushButton:pressed { background-color: #0071e3; }"
        ).arg(color));
        return btn;
    };

    QPushButton *btnLoad = createStyledBtn("Load Image", "#3a3a3c");
    QPushButton *btnCrop = createStyledBtn("Crop Now", "#34C759");
    QPushButton *btnEnhance = createStyledBtn("Enhance", "#0A84FF");
    QPushButton *btnSave = createStyledBtn("Save Final", "#FF9500");

    btnLayout->addWidget(btnLoad);
    btnLayout->addWidget(btnCrop);
    btnLayout->addWidget(btnEnhance);
    btnLayout->addWidget(btnSave);
    mainLayout->addLayout(btnLayout);

    // Connections
    connect(btnLoad, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(btnCrop, &QPushButton::clicked, this, &MainWindow::processCrop);
    connect(btnEnhance, &QPushButton::clicked, this, &MainWindow::enhanceImage);
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::saveImage);
}

void MainWindow::applyAppleStyle() {
    this->setStyleSheet("QMainWindow { background-color: #1c1c1e; }");
}

void MainWindow::loadImage() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select Photo", "", "Images (*.png *.jpg *.jpeg)");
    if (!fileName.isEmpty()) {
        rawImage = cv::imread(fileName.toStdString());
        points.clear();
        updateDisplay(rawImage);
        statusLabel->setText("Select 4 corners on the document...");
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    // Map globally to the imageLabel local coordinates
    QPoint localPos = imageLabel->mapFromParent(event->pos());
    if (imageLabel->rect().contains(localPos) && points.size() < 4 && !rawImage.empty()) {
        float scaleX = (float)rawImage.cols / imageLabel->width();
        float scaleY = (float)rawImage.rows / imageLabel->height();
        points.push_back(cv::Point2f(localPos.x() * scaleX, localPos.y() * scaleY));
        updateDisplay(rawImage);
    }
}

void MainWindow::updateDisplay(const cv::Mat& img) {
    if (img.empty()) return;
    cv::Mat temp;
    img.copyTo(temp);
    for (auto p : points) cv::circle(temp, p, 20, cv::Scalar(0, 255, 0), -1);

    cv::Mat rgb;
    cv::cvtColor(temp, rgb, cv::COLOR_BGR2RGB);
    QImage qimg(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
    imageLabel->setPixmap(QPixmap::fromImage(qimg).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::processCrop() {
    if (points.size() == 4) {
        cv::Point2f dst_pts[] = {{0,0}, {800,0}, {800,1000}, {0,1000}};
        cv::Mat M = cv::getPerspectiveTransform(points.data(), dst_pts);
        cv::warpPerspective(rawImage, processedImage, M, cv::Size(800, 1000));
        updateDisplay(processedImage);
        statusLabel->setText("Cropped! Now Enhance or Save.");
    }
}

void MainWindow::enhanceImage() {
    if (processedImage.empty()) return;
    cv::Mat gray, thresh;
    cv::cvtColor(processedImage, gray, cv::COLOR_BGR2GRAY);
    cv::adaptiveThreshold(gray, thresh, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 11, 2);
    processedImage = thresh;
    
    // Convert back to BGR for display consistency
    cv::cvtColor(processedImage, processedImage, cv::COLOR_GRAY2BGR);
    updateDisplay(processedImage);
    statusLabel->setText("Enhanced for Printing!");
}

void MainWindow::saveImage() {
    if (processedImage.empty()) return;
    QString fileName = QFileDialog::getSaveFileName(this, "Save Print Ready Image", "", "PNG (*.png);;JPG (*.jpg)");
    if (!fileName.isEmpty()) {
        cv::imwrite(fileName.toStdString(), processedImage);
        statusLabel->setText("Saved successfully!");
    }
}