#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    typedef std::chrono::system_clock myclock;
    typedef std::chrono::time_point<std::chrono::system_clock> mytime_point;
private slots:
    void on_bnArrive_clicked();

    void on_bnLeave_clicked();

    void on_timer();
    void on_bnAbout_clicked();

    void on_bnOpenLog_clicked();

private:
    const std::string filename="Timelog.txt";
    const std::string filenameTmp = "Timelog_tmp.txt";
    const uint32_t timerSeconds = 60;

    uint32_t minutesHibernation = 0;

    Ui::MainWindow *ui;
    mytime_point arriveTime;
    mytime_point lastActiveTime;

    QTimer timer;

    static std::string timeToStr(mytime_point t);
    void writeEvent(mytime_point t, const std::string & fname, const std::string & s, bool append=true);

    void arrive();
    void leave();
};
#endif // MAINWINDOW_H
