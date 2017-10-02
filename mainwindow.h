#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void on_pushButtonComOpen_clicked();
    void sendAliveTimerHandle();
    void handleReadyRead();
    void handleErrorOccured(QSerialPort::SerialPortError);


    void on_pushButtonOn_clicked();
    void on_pushButtonOff_clicked();
    void on_pushButton_refreshCom_clicked();

private:
    Ui::MainWindow *ui;

    QSerialPort serial;
    quint32 comExchanges;
    QTimer comSendAliveTimer;

#define BUF_SIZE 500
    char buf[500];
    int bufInd;
};

#endif // MAINWINDOW_H
