#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QComboBox>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QUdpSocket>

namespace Ui {
class MainWindow;
}
#define PROJ_NUM 4

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:

    void sendAliveTimerHandle();
    void handleReadyRead(int);
    void handleErrorOccured(int, QSerialPort::SerialPortError);

    void pushButtonComOpen_clicked(int);
    void pushButtonOn_clicked(int);
    void pushButtonOff_clicked(int);
    void on_pushButton_refreshCom_clicked();

    void handleUdpReadyRead();

private:
    Ui::MainWindow *ui;

    QSerialPort *serialArr[PROJ_NUM];
    quint32 comExchanges;
    QTimer comSendAliveTimer;
    QComboBox *pComboBoxArr[PROJ_NUM];
    QPushButton *pOpenComButtonArr[PROJ_NUM];
    QPushButton *pOnButtonArr[PROJ_NUM], *pOffButtonArr[PROJ_NUM];
    QPlainTextEdit *pPlainTextEditArr[PROJ_NUM];
    QLineEdit *pLineEditStatus[PROJ_NUM];

    QUdpSocket *udpSocket;

#define BUF_SIZE 500
    char buf[500];
    int bufInd;
};

#endif // MAINWINDOW_H
