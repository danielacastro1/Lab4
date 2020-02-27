#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QTimeZone>
#include <QStringListModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
    timer(new QTimer),
    httpManager(new HTTPManager)
{   
    ui->setupUi(this);

    QList<QByteArray> timeZoneIDs = QTimeZone::availableTimeZoneIds();
    QTimeZone timeZone(timeZoneIDs.at(0));
    QString name = timeZone.displayName(QTimeZone::GenericTime);
    qDebug() << name;

    timeZones = new QList<QTimeZone*>();
    QStringList timeZoneNames;
    foreach(QByteArray timeZoneId, timeZoneIDs){
        QTimeZone* timeZone = new QTimeZone(timeZoneId);
        QString timeZoneName = timeZone->displayName(QTimeZone::GenericTime);
        if(!timeZoneNames.contains(timeZoneName)){
            timeZones->append(timeZone);
            timeZoneNames.append(timeZoneName);
        }
    }

    QStringListModel *model = new QStringListModel();
    model->setStringList(timeZoneNames);

    ui->timeZoneComboBox->setModel(model);
    ui->timeZoneComboBox->setCurrentIndex(0);

    connect(timer, SIGNAL(timeout()),
            this, SLOT(setCurrentTime()));

    setCurrentTime();
    timer->start(1000);

    connect(httpManager, SIGNAL(ImageReady(QPixmap *)),
            this, SLOT(processImage(QPixmap *)));

    connect(httpManager, SIGNAL(WeatherJsonReady(QJsonObject *)),
            this, SLOT(processWeatherJson(QJsonObject *)));

    QTimer::singleShot(0, this, SLOT(showFullScreen()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setCurrentTime()
{
    QDateTime dateTime = QDateTime::currentDateTime();

    QString hour = dateTime.toString("hh");
    QString minute = dateTime.toString("mm");
    QString second = dateTime.toString("ss");

    ui->hourLCD->display(hour);
    ui->minuteLCD->display(minute);
    ui->secondLCD->display(second);

    QDateTime worldDateTime = dateTime.toTimeZone(*(timeZones->at(ui->timeZoneComboBox->currentIndex())));

    QString hour2 = worldDateTime.toString("hh");
    QString minute2 = worldDateTime.toString("mm");
    QString second2 = worldDateTime.toString("ss");

    ui->worldHourLCD->display(hour2);
    ui->worldMinuteLCD->display(minute2);
    ui->worldSecondLCD->display(second2);


}

void MainWindow::processImage(QPixmap *image)
{
    ui->imageLabel->setPixmap(*image);
}

void MainWindow::processWeatherJson(QJsonObject *json)
{
    qDebug() << "Json Ready";
    QString weather = json->value("weather").toArray()[0].toObject()["main"].toString();
    QString weatherDesc = json->value("weather").toArray()[0].toObject()["description"].toString();
    double temp = json->value("main").toObject()["temp"].toDouble();
    double temp_min = json->value("main").toObject()["temp_min"].toDouble();
    double temp_max = json->value("main").toObject()["temp_max"].toDouble();

    qDebug() << weather;
    qDebug() << weatherDesc;
    qDebug() << temp;
    qDebug() << temp_min;
    qDebug() << temp_max;

    ui->weatherLabel->setText("Current Weather: " + weather + ", temp: " + QString::number(temp));

}

void MainWindow::on_imageDownloadPushButton_clicked()
{
    QString zip = ui->zipCodeEdit->text();
    qDebug() << zip;

    httpManager->sendImageRequest(zip);
}

void MainWindow::on_weatherDownloadPushButton_clicked()
{
    QString zip = ui->zipCodeEdit->text();
    qDebug() << zip;
    httpManager->sendWeatherRequest(zip);
}
