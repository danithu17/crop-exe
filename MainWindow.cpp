#include "MainWindow.h"
#include <QFileDialog>
#include <QMouseEvent>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("FlashPrint Pro - Apple Edition");
    setMinimumSize(1000, 850);
    
    // Apple Dark Theme Stylesheet
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
        statusLabel->setText("TAP 4 CORNERS OF THE PAPER");
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    QPoint p = imageLabel->mapFromParent(event->pos());
    if (imageLabel->rect().contains(p) && points.size() < 4 && !rawImage.empty()) {
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

void MainWindow::processCrop() {
    if (points.size() == 4) {
        cv::Point2f dst[] = {{0,0}, {800,0}, {800,1100}, {0,1100}};
        cv::Mat M = cv::getPerspectiveTransform(points.data(), dst);
        cv::warpPerspective(rawImage, processedImage, M, cv::Size(800, 1100));
        updateDisplay(processedImage);
        statusLabel->setText("CROPPED SUCCESSFULLY");
    }
}

void MainWindow::enhanceImage() {
    if (processedImage.empty()) return;
    cv::Mat g, e;
    cv::cvtColor(processedImage, g, cv::COLOR_BGR2GRAY);
    cv::adaptiveThreshold(g, e, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 13, 5);
    cv::cvtColor(e, processedImage, cv::COLOR_GRAY2BGR);
    updateDisplay(processedImage);
    statusLabel->setText("QUALITY ENHANCED");
}

void MainWindow::saveImage() {
    if (processedImage.empty()) return;
    QString s = QFileDialog::getSaveFileName(this, "Save Image", "", "PNG (*.png)");
    if (!s.isEmpty()) {
        cv::imwrite(s.toStdString(), processedImage);
        statusLabel->setText("SAVED TO DISK");
    }
}