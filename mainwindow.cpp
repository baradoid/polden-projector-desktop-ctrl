#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QTime>

#define def(a,b) \
    pComboBoxArr[a] = ui->comComboBox_##b; \
    pOpenComButtonArr[a] = ui->pushButtonComOpen_##b; \
    pPlainTextEditArr[a] = ui->plainTextEdit_##b; \
    pLineEditStatus[a] = ui->lineEditStatus_##b; \
    pOnButtonErr[a] =  ui->pushButtonOn_##b; \
    pOffButtonErr[a] =  ui->pushButtonOff_##b

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    def(0, 1);
    def(1, 2);
    def(2, 3);
    def(3, 4);

    QPalette *palette = new QPalette();
    palette->setColor(QPalette::Base,Qt::lightGray);

    for(int i=0; i<PROJ_NUM; i++){
        QPushButton *pb = pOpenComButtonArr[i];
        //connect(pb, SIGNAL(clicked(bool)),
        //        &comOpenButtonMapper, SLOT(map()));
        connect(pb, &QPushButton::clicked, [this, i](){ on_pushButtonComOpen_clicked(i);} );
        //comOpenButtonMapper.setMapping(pb, i);

        pLineEditStatus[i]->setText("n/a");
        pLineEditStatus[i]->setPalette(*palette);

        QSerialPort *sp = new QSerialPort(this);
        serialArr[i] = sp;
        connect(sp, &QSerialPort::readyRead, [this, i](){ handleReadyRead(i);});
        //connect(sp, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), &serialErrorOccuredMapper, SLOT(map()));
        connect(sp, &QSerialPort::errorOccurred, [this, i](QSerialPort::SerialPortError error){ handleErrorOccured(i, error);});
    }

    connect(&comSendAliveTimer, SIGNAL(timeout()), this, SLOT(sendAliveTimerHandle()));
    comSendAliveTimer.setInterval(1000);
    comSendAliveTimer.start();
    on_pushButton_refreshCom_clicked();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonComOpen_clicked(int id)
{
   serialArr[id]->setBaudRate(115200);
   QPushButton *pb = pOpenComButtonArr[id];
   QComboBox *cb = pComboBoxArr[id];

    if(pb->text() == "open"){
        if(serialArr[id]->isOpen() == false){
            QString comName = cb->currentText();
            //QString comName("com9");
            if(comName.length() > 0){
                //UartThread.requestToStart(comName);
                serialArr[id]->setPortName(comName);
                if (!serialArr[id]->open(QIODevice::ReadWrite)) {
                    //qDebug("%s port open FAIL", qUtf8Printable(comName));
                    QString msg = QString("%1 open FAIL").arg(qUtf8Printable(comName));
                    pPlainTextEditArr[id]->appendPlainText(QString("%1 open FAIL").arg(qUtf8Printable(comName)));
                    return;
                }
                else{
                    cb->setEnabled(false);
                    //qDebug("%s port opened", qUtf8Printable(comName));
                    QString msg = QString("%1 opened").arg(qUtf8Printable(comName));
                    pPlainTextEditArr[id]->appendPlainText(msg);
                    //ui->plainTextEdit->appendPlainText(QString("%1 port opened").arg(qUtf8Printable(comName)));
                    ui->statusBar->showMessage(msg);

                    pb->setText("close");
                    comExchanges = 0;
                    //usbConnectionTime.start();
                    bufInd = 0;
                }
            }
        }
    }
    else{       
        serialArr[id]->close();
        cb->setEnabled(true);
        //udpSocket->close();
        //qDebug("com port closed");
        pPlainTextEditArr[id]->appendPlainText(QString("%1 closed").arg(serialArr[id]->portName()));
        pb->setText("open");
        //contrStringQueue.clear();
    }
}

void MainWindow::sendAliveTimerHandle()
{
    if(serialArr[0]->isOpen()){
        qint64 iWritten = serialArr[0]->write(QString("\r*pow=?#\r").toLatin1());
        qDebug() << QTime::currentTime().msecsSinceStartOfDay() << "timeout" << iWritten;
    }
}

void MainWindow::handleReadyRead(int id)
{
    QByteArray ba = serialArr[id]->readAll();
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

void MainWindow::handleErrorOccured(int id, QSerialPort::SerialPortError error)
{
    qDebug() <<"!!!!!!!" << id <<error;
}


void MainWindow::on_pushButtonOn_clicked()
{
    if(serialArr[0]->isOpen()){
        serialArr[0]->write(QString("\r*pow=on#\r").toLatin1());
    }
}

void MainWindow::on_pushButtonOff_clicked()
{
    if(serialArr[0]->isOpen()){
        qint64 iWritten = serialArr[0]->write(QString("\r*pow=off#\r").toLatin1());
        qDebug() << iWritten;
    }
}

void MainWindow::on_pushButton_refreshCom_clicked()
{
    for(int i=0; i<PROJ_NUM; i++){
        if(pComboBoxArr[i]->isEnabled())
            pComboBoxArr[i]->clear();
    }

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

           for(int i=0; i<PROJ_NUM; i++){
               if(pComboBoxArr[i]->isEnabled())
                   pComboBoxArr[i]->addItem(serialPortInfo.portName());
           }

    }

}

