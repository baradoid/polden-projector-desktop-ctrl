#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&comSendAliveTimer, SIGNAL(timeout()),
            this, SLOT(sendAliveTimerHandle()));
    comSendAliveTimer.setInterval(1000);
    comSendAliveTimer.start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonComOpen_clicked()
{
   serial.setBaudRate(115200);
    if(ui->pushButtonComOpen->text() == "open"){
        if(serial.isOpen() == false){
            //QString comName = ui->comComboBox->currentText();
            QString comName("com6");
            if(comName.length() > 0){
                //UartThread.requestToStart(comName);
                serial.setPortName(comName);
                if (!serial.open(QIODevice::ReadWrite)) {
                    qDebug("%s port open FAIL", qUtf8Printable(comName));
                    return;
                }
                //qDebug("%s port opened", qUtf8Printable(comName));
                //ui->plainTextEdit->appendPlainText(QString("%1 port opened").arg(qUtf8Printable(comName)));
                connect(&serial, SIGNAL(readyRead()),
                        this, SLOT(handleReadyRead()));
//                connect(&serial, SIGNAL(bytesWritten(qint64)),
//                        this, SLOT(handleSerialDataWritten(qint64)));
                ui->pushButtonComOpen->setText("close");
                comExchanges = 0;
                //usbConnectionTime.start();
                bufInd = 0;
            }
        }
    }
    else{
        serial.close();
        //udpSocket->close();
        //qDebug("com port closed");
        //ui->plainTextEdit->appendPlainText(QString("com port closed"));
        ui->pushButtonComOpen->setText("open");
        //contrStringQueue.clear();
    }
}

void MainWindow::sendAliveTimerHandle()
{
    if(serial.isOpen()){
        qDebug("timeout");
        serial.write(QString("\r*pow=?#\r").toLatin1());

    }
}

void MainWindow::handleReadyRead()
{
    QByteArray ba = serial.readAll();
    for(int i=0; i<ba.length(); i++){
        buf[bufInd++] = ba[i];
        if(buf[bufInd-1] == '\n'){
            buf[bufInd] = 0;
            qDebug() << buf;
            bufInd = 0;
        }

    }


}

void MainWindow::on_pushButtonOn_clicked()
{
    if(serial.isOpen()){
        serial.write(QString("\r*pow=on#\r").toLatin1());
    }
}

void MainWindow::on_pushButtonOff_clicked()
{
    if(serial.isOpen()){
        serial.write(QString("\r*pow=off#\r").toLatin1());
    }
}
