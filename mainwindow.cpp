#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QTime>
#include <QNetworkDatagram>
#include <QSettings>

#define def(a,b) \
    pComboBoxArr[a] = ui->comComboBox_##b; \
    pOpenComButtonArr[a] = ui->pushButtonComOpen_##b; \
    pPlainTextEditArr[a] = ui->plainTextEdit_##b; \
    pLineEditStatus[a] = ui->lineEditStatus_##b; \
    pOnButtonArr[a] =  ui->pushButtonOn_##b; \
    pOffButtonArr[a] =  ui->pushButtonOff_##b; \
    pLineEditLampHour1Arr[a] = ui->lineEditLampHour1_##b

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    sendAliveCnt(0)
{
    ui->setupUi(this);

    def(0, 1);
    def(1, 2);
    def(2, 3);
    def(3, 4);

    paletteGrey = new QPalette();
    paletteGrey->setColor(QPalette::Base,Qt::lightGray);
    paletteRed = new QPalette();
    paletteRed->setColor(QPalette::Base,Qt::red);
    paletteGreen = new QPalette();
    paletteGreen->setColor(QPalette::Base,Qt::green);


    for(int i=0; i<PROJ_NUM; i++){
        connect(pOpenComButtonArr[i], &QPushButton::clicked, [this, i](){ pushButtonComOpen_clicked(i);} );
        connect(pOnButtonArr[i], &QPushButton::clicked,  [this, i](){ pushButtonOn_clicked(i);} );
        connect(pOffButtonArr[i], &QPushButton::clicked,  [this, i](){ pushButtonOff_clicked(i);} );

        pLineEditStatus[i]->setText("n/a");
        pLineEditStatus[i]->setPalette(*paletteGrey);

        QSerialPort *sp = new QSerialPort(this);
        serialArr[i] = sp;
        connect(sp, &QSerialPort::readyRead, [this, i](){ handleReadyRead(i);});
        //connect(sp, SIGNAL(errorOccurred(QSerialPort::SerialPortError)), &serialErrorOccuredMapper, SLOT(map()));
        connect(sp, &QSerialPort::errorOccurred, [this, i](QSerialPort::SerialPortError error){ handleErrorOccured(i, error);});
        bResponseArr[i] = na;
    }
    on_pushButton_refreshCom_clicked();

    QSettings settings("Murinets", "ProjectorControl");

    for(int i=0; i<PROJ_NUM; i++){
        QString curComPortName = settings.value(QString("projector%1ComNum").arg(i), "").toString();
        QComboBox *cb = pComboBoxArr[i];
        for(int j=0; j<cb->count(); j++){
            if(cb->itemText(j).compare(curComPortName) == 0){
                cb->setCurrentIndex(j);
                break;
            }
        }
    }

    connect(&comSendAliveTimer, SIGNAL(timeout()), this, SLOT(sendAliveTimerHandle()));
    comSendAliveTimer.setInterval(1000);
    comSendAliveTimer.start();    

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::LocalHost, 8052);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(handleUdpReadyRead()));

}

MainWindow::~MainWindow()
{
    QSettings settings("Murinets", "ProjectorControl");

    for(int i=0; i<PROJ_NUM; i++){
        settings.setValue(QString("projector%1ComNum").arg(i), pComboBoxArr[i]->currentText());
    }
    delete ui;
}


void MainWindow::comPortOpen(int id)
{
    QPushButton *pb = pOpenComButtonArr[id];
    QComboBox *cb = pComboBoxArr[id];

    if(serialArr[id]->isOpen() == false){
        serialArr[id]->setBaudRate(115200);
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
                //bufInd = 0;
                buf[id].clear();
            }
        }
    }
}

void MainWindow::comPortClose(int id)
{
    pComboBoxArr[id]->setEnabled(true);
    pOpenComButtonArr[id]->setText("open");
    if(serialArr[id]->isOpen() == true){
        pPlainTextEditArr[id]->appendPlainText(QString("%1 closed").arg(serialArr[id]->portName()));
        serialArr[id]->close();
    }
}


void MainWindow::pushButtonComOpen_clicked(int id)
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
                    //bufInd = 0;
                    buf[id].clear();
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
    QString msg;
    switch(sendAliveCnt&3){
    case 1: msg = QString("\r*ltim=?#\r"); break;
    case 2: msg = QString("\r*ltim2=?#\r"); break;
    case 0:
    default:
        msg = QString("\r*pow=?#\r");
        break;

    }

    for(int i=0; i<PROJ_NUM; i++){
        if(serialArr[i]->isOpen()){
            qint64 iWritten = serialArr[i]->write(msg.toLatin1());
            //qDebug() << QTime::currentTime().msecsSinceStartOfDay() << "timeout" << iWritten;

            if(bResponseArr[i] == noResp)
                pLineEditStatus[i]->setPalette(*paletteRed);
            //else
            //    pLineEditStatus[i]->setPalette(*paletteGrey);
            bResponseArr[i] = noResp;
        }
    }
    sendAliveCnt++;
}

void MainWindow::handleProjectorMessage(int id, QString msg)\
{
    //qDebug() << msg;
    bResponseArr[id] = resp;
    pLineEditStatus[id]->setPalette(*paletteGreen);

    if(msg.compare("*POW=OFF#\r\n") == 0){
        pLineEditStatus[id]->setText(msg);
    }
    else if(msg.compare("*POW=ON#\r\n") == 0){
        pLineEditStatus[id]->setText(msg);
    }
    else if(msg.compare("*Block item#\r\n") == 0){
    }
    else if(msg.startsWith("*LTIM")){
        pLineEditLampHour1Arr[id]->setText(msg);
    }
}


void MainWindow::handleReadyRead(int id)
{
    QByteArray ba = serialArr[id]->readAll();
    buf[id].append(ba);
    while(1){
        int ind = buf[id].indexOf('\n');
        if(ind == -1)
            break;
        //QString msg(buf[id].left(ind+1));

        handleProjectorMessage(id, QString(buf[id].left(ind+1)));
        buf[id].remove(0, ind+1);
    }
    if(buf[id].length() > BUF_SIZE){
        buf[id].remove(0, buf[id].length() - BUF_SIZE);
    }

//    for(int i=0; i<ba.length(); i++){
//        buf[bufInd++] = ba[i];
//        if( (buf[bufInd-1] == '\n') ||
//            (bufInd == BUF_SIZE) ){
//            buf[bufInd] = 0;
//            qDebug() << buf;
//            bufInd = 0;
//        }
//    }
//    qDebug() << "*";
}

void MainWindow::handleErrorOccured(int id, QSerialPort::SerialPortError error)
{
    if(error != QSerialPort::NoError){
        QString errorStr;
        switch(error){
            case QSerialPort::DeviceNotFoundError: errorStr = "DeviceNotFoundError"; break;
            case QSerialPort::PermissionError: errorStr = "PermissionError"; break;
            case QSerialPort::OpenError: errorStr = "OpenError"; break;
            case QSerialPort::ParityError: errorStr = "ParityError"; break;
            case QSerialPort::FramingError: errorStr = "FramingError"; break;
            case QSerialPort::BreakConditionError: errorStr = "BreakConditionError"; break;
            case QSerialPort::WriteError: errorStr = "WriteError"; break;
            case QSerialPort::ReadError: errorStr = "ReadError"; break;
            case QSerialPort::ResourceError: errorStr = "ResourceError"; break;
            case QSerialPort::UnsupportedOperationError: errorStr = "UnsupportedOperationError"; break;            
            case QSerialPort::TimeoutError: errorStr = "TimeoutError"; break;
            case QSerialPort::NotOpenError: errorStr = "NotOpenError"; break;
            default:
            case QSerialPort::UnknownError: errorStr = "UnknownError"; break;

        }

        QString msg = QString("%1 error: %2").arg(qUtf8Printable(serialArr[id]->portName())).arg(errorStr);
        pPlainTextEditArr[id]->appendPlainText(msg);
        //qDebug() <<"!!!!!!!" << id <<error;
        if(error == QSerialPort::ResourceError){
            pushButtonComOpen_clicked(id);
        }
    }
}


void MainWindow::pushButtonOn_clicked(int id)
{
    if(serialArr[id]->isOpen()){
        qint64 iWritten = serialArr[id]->write(QString("\r*pow=on#\r").toLatin1());
        //qDebug() << id << iWritten;
    }
}

void MainWindow::pushButtonOff_clicked(int id)
{
    if(serialArr[id]->isOpen()){
        qint64 iWritten = serialArr[id]->write(QString("\r*pow=off#\r").toLatin1());
        //qDebug() << id << iWritten;
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

void MainWindow::handleUdpReadyRead()
{
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        QString msg(datagram.data());
        //qDebug() << msg.toLatin1();
        ui->statusBar->showMessage(QString("UDP cmd: \"%1\"").arg(msg));
        if(msg.compare("pon_all\r\n") == 0){
            //qDebug() << "pOn";
            for(int i=0; i<PROJ_NUM; i++){
                pushButtonOn_clicked(i);
            }
        }
        else if(msg.compare("poff_all\r\n") == 0){
            //qDebug() << "pOff";
            for(int i=0; i<PROJ_NUM; i++){
                pushButtonOff_clicked(i);
            }
        }
    }
}


void MainWindow::on_pushButtonOpenAll_clicked()
{
    for(int i=0; i<PROJ_NUM; i++){
        comPortOpen(i);
    }
}

void MainWindow::on_pushButtonCloseAll_clicked()
{
    for(int i=0; i<PROJ_NUM; i++){
        comPortClose(i);
    }
}
