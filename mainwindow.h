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

    void on_pushButtonOpenAll_clicked();

    void on_pushButtonCloseAll_clicked();

private:
    Ui::MainWindow *ui;

    void handleProjectorMessage(int id, QString msg);

    QSerialPort *serialArr[PROJ_NUM];
    quint32 comExchanges;
    QTimer comSendAliveTimer;
    QComboBox *pComboBoxArr[PROJ_NUM];
    QPushButton *pOpenComButtonArr[PROJ_NUM];
    QPushButton *pOnButtonArr[PROJ_NUM], *pOffButtonArr[PROJ_NUM];
    QPlainTextEdit *pPlainTextEditArr[PROJ_NUM];
    QLineEdit *pLineEditStatus[PROJ_NUM];
    QLineEdit *pLineEditLampHour1Arr[PROJ_NUM];

    QUdpSocket *udpSocket;

    enum{
        na=0,
        noResp,
        resp
    }bResponseArr[PROJ_NUM];
#define BUF_SIZE 500
    QByteArray buf[PROJ_NUM];

    QPalette *paletteGrey, *paletteRed, *paletteGreen;

    quint32 sendAliveCnt;
};

#endif // MAINWINDOW_H
