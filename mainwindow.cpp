#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QPushButton>
#include <QPainter>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkManager(new QNetworkAccessManager(this))
    , currentImageIndex(0)
{
    ui->setupUi(this);

    // Fetch JSON data
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onJsonDownloaded);
    QNetworkRequest request(QUrl("https://cdn-media.brightline.tv/dev/templates/image_gallery_exercise.json"));
    networkManager->get(request);

    // Connect buttons for navigation
    connect(ui->leftArrowButton, &QPushButton::clicked, this, &MainWindow::onLeftArrowClicked);
    connect(ui->rightArrowButton, &QPushButton::clicked, this, &MainWindow::onRightArrowClicked);


    thumbnailLayout = new QHBoxLayout(ui->thumbnailWidget);
    thumbnailLayout->setAlignment(Qt::AlignLeft);
    thumbnailLayout->setContentsMargins(0,0,0,0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onJsonDownloaded(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
        QJsonObject jsonObject = jsonDoc.object();
        QJsonArray components = jsonObject["components"].toArray();

        for (const QJsonValue &value : components)
        {
            QJsonObject component = value.toObject();
            QString componentName = component["componentName"].toString();
            if(componentName == "reticle")
            {
                focusUrl = component["active"].toString();
            }
            else if (componentName == "background")
            {
                QString bgUrl = component["componentSrc"].toString();
                loadBackground(bgUrl);

            }
            else if (componentName == "carousel")
            {
                carouselItems = component["components"].toArray();

                loadImage(currentImageIndex);                
                loadThumbnails();
            }
        }
    }
    reply->deleteLater();    
}

void MainWindow::loadBackground(const QString &url)
{
    QNetworkAccessManager* m_Manager = new QNetworkAccessManager(this);
    QNetworkRequest request(url);
    m_Manager->get(request);
    connect(m_Manager, &QNetworkAccessManager::finished, this,[=](QNetworkReply* reply){
        if(reply->error() != QNetworkReply::NoError) return;
        QByteArray data = reply->readAll();
        QImage image;
        image .loadFromData(data);
        QPixmap bkgnd(QPixmap::fromImage(image));
        bkgnd = bkgnd.scaled(this->size(), Qt::KeepAspectRatio);
        QPalette palette;
        palette.setBrush(QPalette::Window, bkgnd);
        this->setPalette(palette);
        reply->deleteLater();
    });
}

void MainWindow::loadImage(int index)
{
    if (index < 0 || index >= carouselItems.size()) return;

    QJsonObject imageObject = carouselItems[index].toObject();
    QString imageUrl = imageObject["displayImgSrc"].toString();

    QNetworkAccessManager* m_Manager = new QNetworkAccessManager(this);
    QNetworkRequest request(imageUrl);
    m_Manager->get(request);

    connect(m_Manager, &QNetworkAccessManager::finished, this,[=](QNetworkReply* reply){
        if(reply->error() != QNetworkReply::NoError) return;
        QByteArray data = reply->readAll();
        QImage image;
        image .loadFromData(data);
        QPixmap pixmap(QPixmap::fromImage(image));
        pixmap = pixmap.scaled(this->size());
        ui->mainImageLabel->setPixmap(pixmap.scaled(image.width(), image.height(),Qt::KeepAspectRatio));
        reply->deleteLater();
    });
}
void MainWindow::loadThumbnails()
{
    //Clear existing thumbnails
    while (QLayoutItem* item = thumbnailLayout->takeAt(0))
    {
        delete item->widget();
        delete item;
    }

    //Fill thumbnail layout
    for (int i = 0; i < carouselItems.size(); ++i)
    {
        QWidget * thumbnailBackWidjet = new QWidget(ui->thumbnailWidget);
        QString thumbnailUrl;
        QJsonObject thumbnailObject = carouselItems[i].toObject();
        QGridLayout *thumbnailItemLayout = new QGridLayout(thumbnailBackWidjet);
        thumbnailItemLayout->setContentsMargins(0,0,0,0);

        if(i == currentImageIndex)
        {
            QNetworkAccessManager* m_Manager = new QNetworkAccessManager(this);
            QNetworkRequest request(focusUrl);
            m_Manager->get(request);
            connect(m_Manager, &QNetworkAccessManager::finished, this,[=](QNetworkReply* reply){
                if(reply->error() != QNetworkReply::NoError) return;
                QByteArray data = reply->readAll();
                QImage image;
                image .loadFromData(data);
                QPixmap thumbnailPixmap(QPixmap::fromImage(image));
                thumbnailPixmap = thumbnailPixmap.scaled(this->size());
                QLabel *thumbnailFocusLabel = new QLabel(ui->thumbnailWidget);
                thumbnailFocusLabel->setPixmap(thumbnailPixmap.scaled(image.width(), image.height(), Qt::IgnoreAspectRatio));
                thumbnailItemLayout->addWidget(thumbnailFocusLabel, 0, 0);
                reply->deleteLater();
            });

            thumbnailUrl = thumbnailObject["focusSrc"].toString();
        }
        else
        {
            thumbnailUrl = thumbnailObject["normalSrc"].toString();
        }

        QLabel *thumbnailLabel = new QLabel(ui->thumbnailWidget);
        thumbnailItemLayout->addWidget(thumbnailLabel);

        QNetworkAccessManager* m_Manager = new QNetworkAccessManager(this);
        QNetworkRequest request(thumbnailUrl);
        m_Manager->get(request);
        connect(m_Manager, &QNetworkAccessManager::finished, this,[=](QNetworkReply* reply){
            if(reply->error() != QNetworkReply::NoError) return;
            QByteArray data = reply->readAll();
            QImage image;
            image .loadFromData(data);
            QPixmap thumbnailPixmap(QPixmap::fromImage(image));
            thumbnailPixmap = thumbnailPixmap.scaled(this->size());
            thumbnailLabel->setPixmap(thumbnailPixmap.scaled(image.width(), image.height(), Qt::IgnoreAspectRatio));
            reply->deleteLater();
        });

        thumbnailLayout->addWidget(thumbnailBackWidjet);
    }
}

void MainWindow::onLeftArrowClicked()
{
    if (currentImageIndex > 0)
    {
        currentImageIndex--;
        loadImage(currentImageIndex);
        loadThumbnails();
    }
}

void MainWindow::onRightArrowClicked()
{
    if (currentImageIndex < carouselItems.size() - 1)
    {
        currentImageIndex++;
        loadImage(currentImageIndex);
        loadThumbnails();
    }
}
