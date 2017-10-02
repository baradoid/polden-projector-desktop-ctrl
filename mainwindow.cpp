#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QTime>

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
            QString comName = ui->comComboBox->currentText();
            //QString comName("com9");
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
                connect(&serial, SIGNAL(errorOccurred(QSerialPort::SerialPortError)),
                        this, SLOT(handleErrorOccured(QSerialPort::SerialPortError)));

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
        qint64 iWritten = serial.write(QString("\r*pow=?#\r").toLatin1());
        qDebug() << QTime::currentTime().msecsSinceStartOfDay() << "timeout" << iWritten;
    }
}

void MainWindow::handleReadyRead()
{
    QByteArray ba = serial.readAll();
    for(int i=0; i<ba.length(); i++){
        buf[bufInd++] = ba[i];
        if( (buf[bufInd-1] == '\n') ||
            (bufInd == BUF_SIZE) ){
            buf[bufInd] = 0;
            qDebug() << buf;
            bufInd = 0;
        }
    }
    qDebug() << "*";
}

void MainWindow::handleErrorOccured(QSerialPort::SerialPortError error)
{
    qDebug() <<"!!!!!!!" <<error;
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
        qint64 iWritten = serial.write(QString("\r*pow=off#\r").toLatin1());
        qDebug() << iWritten;
    }
}

void MainWindow::on_pushButton_refreshCom_clicked()
{
    ui->comComboBox->clear();
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    const QString blankString = QObject::tr("N/A");
      QString description;
      QString manufacturer;
      QString serialNumber;
    for (const QSerialPortInfo &serialPortInfo : serialPortInfos) {
           description = serialPortInfo.description();
           manufacturer = serialPortInfo.manufacturer();
           serialNumber = serialPortInfo.serialNumber();
           qDebug() << endl
               << QObject::tr("Port: ") << serialPortInfo.portName() << endl
               << QObject::tr("Location: ") << serialPortInfo.systemLocation() << endl
               << QObject::tr("Description: ") << (!description.isEmpty() ? description : blankString) << endl
               << QObject::tr("Manufacturer: ") << (!manufacturer.isEmpty() ? manufacturer : blankString) << endl
               << QObject::tr("Serial number: ") << (!serialNumber.isEmpty() ? serialNumber : blankString) << endl
               << QObject::tr("Vendor Identifier: ") << (serialPortInfo.hasVendorIdentifier() ? QByteArray::number(serialPortInfo.vendorIdentifier(), 16) : blankString) << endl
               << QObject::tr("Product Identifier: ") << (serialPortInfo.hasProductIdentifier() ? QByteArray::number(serialPortInfo.productIdentifier(), 16) : blankString) << endl
               << QObject::tr("Busy: ") << (serialPortInfo.isBusy() ? QObject::tr("Yes") : QObject::tr("No")) << endl;
           ui->comComboBox->addItem(serialPortInfo.portName());
    }

}

