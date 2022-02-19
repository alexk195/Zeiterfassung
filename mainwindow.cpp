#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <sstream>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timer.setInterval(1000*timerSeconds); // call timer every minute
    timer.start();
    this->connect(&timer,SIGNAL(timeout()),this,SLOT(on_timer()));

    // check for app crash, copy tmp into log
    QFile tmp(QString(this->filenameTmp.c_str()));
    if (tmp.exists())
    {
        tmp.open(QIODevice::ReadOnly);
        auto buf = tmp.readAll();
        QFile log(QString(this->filename.c_str()));
        log.open(QIODevice::WriteOnly|QIODevice::Append);
        log.write(buf);
        tmp.close();
        tmp.remove();
    }

    // set arrive time
    arrive();

    lastActiveTime = arriveTime;

    // call once at start up to update time
    on_timer();
}

MainWindow::~MainWindow()
{
    writeEvent(myclock::now(),filename,"LEAVE (app terminated)");
    timer.stop();

    // gracefull shutdown -> delete tmp

    QFile f(QString(this->filenameTmp.c_str()));
    f.remove();

    delete ui;
}

void MainWindow::on_timer()
{
    auto currentTime = myclock::now();
    auto secondsSinceLastTimer = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastActiveTime).count();

    if (secondsSinceLastTimer > timerSeconds*2)
    {
        // hibernation/sleep occured
        writeEvent(lastActiveTime,filename,"LEAVE (app hibernation)");
        writeEvent(currentTime,filename,"ARRIVE (from hibernation)");

        minutesHibernation += std::chrono::duration_cast<std::chrono::minutes>(currentTime - lastActiveTime).count();
    }

    uint64_t minsActive = std::chrono::duration_cast<std::chrono::minutes>(currentTime - arriveTime).count();
    minsActive -= minutesHibernation;

    // update active time
    QString s = QString("%1:%2").arg(uint(minsActive/60),2,10,QChar('0')).arg(uint(minsActive%60),2,10,QChar('0'));
    ui->laTime->setText(s);

    // store into tmp file current time for crash recovery
    writeEvent(currentTime,filenameTmp,"LEAVE (app crashed)",false);

    lastActiveTime = currentTime;
}


void MainWindow::on_bnArrive_clicked()
{
    arrive();
}

void MainWindow::on_bnLeave_clicked()
{
    leave();
}

void MainWindow::writeEvent(mytime_point t, const std::string & fname, const std::string & s, bool append)
{
   std::ofstream ofs(fname, append ? std::ios_base::app : std::ios_base::out);
   ofs << timeToStr(t) << "," << s << std::endl;
   ofs.close();
}


void MainWindow::arrive()
{
    arriveTime = myclock::now();
    writeEvent(arriveTime,filename,"ARRIVE");
    ui->laArrived->setText(QString(timeToStr(arriveTime).c_str()));
    ui->bnArrive->setEnabled(false);
    ui->bnLeave->setEnabled(true);
}

void MainWindow::leave()
{
    writeEvent(myclock::now(),filename,"LEAVE");
    ui->bnArrive->setEnabled(true);
    ui->bnLeave->setEnabled(false);
}

std::string MainWindow::timeToStr(mytime_point t)
{
    std::time_t tt = myclock::to_time_t(t);
    std::tm tm = *std::localtime(&tt);
    std::stringstream ss;
    ss << std::put_time(&tm, "%d.%m.%Y,%H:%M:%S");
    return ss.str();
}

void MainWindow::on_bnAbout_clicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/alexk195/Zeiterfassung"));
}

void MainWindow::on_bnOpenLog_clicked()
{
    QDesktopServices::openUrl(QUrl(QString(this->filename.c_str())));
}
