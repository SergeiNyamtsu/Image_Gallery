#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonArray>
#include <QJsonObject>
#include <QPixmap>
#include <QHBoxLayout>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onJsonDownloaded(QNetworkReply *reply);
    void onLeftArrowClicked();
    void onRightArrowClicked();

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *networkManager;
    QJsonArray carouselItems;
    int currentImageIndex;

    void loadImage(int index);
    void loadBackground(const QString &url);
    void loadThumbnails();
    QString focusUrl;

    QHBoxLayout* thumbnailLayout;
};

#endif // MAINWINDOW_H
