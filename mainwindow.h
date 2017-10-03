#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QComboBox>
#include <QPushButton>
#include <QPlainTextEdit>

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

    void on_pushButtonComOpen_clicked(int);
    void on_pushButtonOn_clicked();
    void on_pushButtonOff_clicked();
    void on_pushButton_refreshCom_clicked();

private:
    Ui::MainWindow *ui;

    QSerialPort *serialArr[PROJ_NUM];
    quint32 comExchanges;
    QTimer comSendAliveTimer;
    QComboBox *pComboBoxArr[PROJ_NUM];
    QPushButton *pOpenComButtonArr[PROJ_NUM];
    QPushButton *pOnButtonErr[PROJ_NUM], *pOffButtonErr[PROJ_NUM];
    QPlainTextEdit *pPlainTextEditArr[PROJ_NUM];
    QLineEdit *pLineEditStatus[PROJ_NUM];


#define BUF_SIZE 500
    char buf[500];
    int bufInd;
};

#endif // MAINWINDOW_H
