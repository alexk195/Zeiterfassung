#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <sstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer.setInterval(1000*60); // call timer every minute
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
    uint64_t mins = std::chrono::duration_cast<std::chrono::minutes>(myclock::now() - this->arriveTime).count();
    QString s = QString("%1:%2").arg(uint(mins/60),2,10,QChar('0')).arg(uint(mins%60),2,10,QChar('0'));
    ui->laTime->setText(s);
    writeEvent(myclock::now(),this->filenameTmp,"LEAVE (app crashed)",false);
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
