#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "console.h"
#include "plot.h"

#include "qwt_scale_engine.h"
#include <QMessageBox>
#include <QLabel>
#include <QtSerialPort/QSerialPort>
#include <QDebug>
#include <QRegularExpression>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this); //Sets up mainwindow
    setWindowTitle("Ground Station V3");
    
    
    console = new Console;
    console->setEnabled(false);
    ui->ConsoleLayout->addWidget(console);    

    settings = new SettingsDialog;
    serial = new QSerialPort(this);

    ui->openButton->setEnabled(true);
    ui->actionConfigure->setEnabled(true);
    ui->closeButton->setEnabled(false);
    ui->sendButton->setEnabled(false);

    status = new QLabel;

    init_actionConnections(); //connects MainWindow Buttons to coresponding actions


    plot1 = new Plot(this);
    plot2 = new Plot(this);
    plot3 = new Plot(this);
    plot4 = new Plot(this);
    plot5 = new Plot(this);

    positionMap = new Plot(this);

    /* Set axis titles for plots */
    plot1->setAxisTitle(QwtPlot::xBottom, "Time (s)");
    plot1->setAxisTitle(QwtPlot::yLeft, "Altitude (m)");

    plot2->setAxisTitle(QwtPlot::xBottom, "Time (s)");
    plot2->setAxisTitle(QwtPlot::yLeft, "Pressure (Pa)");

    plot3->setAxisTitle(QwtPlot::xBottom, "Time (s)");
    plot3->setAxisTitle(QwtPlot::yLeft, "Speed (m/s)");

    plot4->setAxisTitle(QwtPlot::xBottom, "Time (s)");
    plot4->setAxisTitle(QwtPlot::yLeft, "Temperature (C)");

    plot5->setAxisTitle(QwtPlot::xBottom, "Time (s)");
    plot5->setAxisTitle(QwtPlot::yLeft, "Voltage (mV)");

    positionMap->setAxisTitle(QwtPlot::xBottom, "Glider Position");

    /* end */


    plot1->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating,true);
    plot1->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating,true);

    plot2->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating, true);
    plot2->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating, true);

    plot3->axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating,true);
    plot3->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating,true);

    plot2->setAutoReplot(true);

    //Add plots to grid layout
    ui->plotLayout->addWidget(plot1,1,1);
    ui->plotLayout->addWidget(plot2,1,2);
    ui->plotLayout->addWidget(plot3,1,3);
    ui->plotLayout->addWidget(plot4,2,1);
    ui->plotLayout->addWidget(plot5,2,2);
    ui->plotLayout->addWidget(positionMap,2,3);

    positionMap->setAxisScale(QwtPlot::xBottom,-500,500);
    positionMap->setAxisScale(QwtPlot::yLeft, -500, 500);

    //Connections
    connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &MainWindow::handleSerialError);
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(console, &Console::getData, this, &MainWindow::writeData);
    connect(ui->sendButton, SIGNAL (released()), this, SLOT (handleSendButton()));

    VehicleState = CONTAINER;
    previousXcoord = 0;
    previousYcoord = 0;
}

MainWindow::~MainWindow()
{    
    delete settings;
    delete ui;
}

void MainWindow::openSerialPort()
//Opens serial port using settings from SettingsDialog
//This means settings are updated with each open signal
{
    SettingsDialog::Settings p = settings->settings();
    serial->setPortName(p.name);
    serial->setBaudRate(p.baudRate);
    serial->setDataBits(p.dataBits);
    serial->setParity(p.parity);
    serial->setStopBits(p.stopBits);
    serial->setFlowControl(p.flowControl);
    if(serial->open(QIODevice::ReadWrite))
    {
        console->setEnabled(true);
        console->setLocalEchoEnabled(true);
        ui->openButton->setEnabled(false);
        ui->closeButton->setEnabled(true);
        ui->actionConfigure->setEnabled(false);
        ui->sendButton->setEnabled(true);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));

    }
    else
    {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        showStatusMessage("Open error");
    }
}

void MainWindow::closeSerialPort()
{
    if(serial->isOpen())
        serial->close();
    console->setEnabled(false);
    ui->openButton->setEnabled(true);
    ui->closeButton->setEnabled(false);
    ui->actionConfigure->setEnabled(true);
}

void MainWindow::writeData(const QByteArray &data)
{
    serial->write(data);
}

void MainWindow::readData()
{
    QByteArray data = serial->readAll();
    console->putData(data);

    save_to_file(data);
    //QString DataStr = packetIsValid(data);
    QString DataStr = MakePacketValid(data);
    qDebug() << DataStr << endl;
    if(DataStr != "")
    {
        format_in_data(DataStr);
        appendPoints();
    }

}

void MainWindow::handleSendButton()
{
    QByteArray command;
    if(ui->commandCB->currentIndex() == 0)
    {
        QString temp = QString::number(ui->stateCB->currentIndex());
        command = temp.toUtf8();
    }
    else if(ui->commandCB->currentIndex() == 1)
    {
        QString temp = "Picture Code Here";
        command = temp.toUtf8();
    }
    else if(ui->commandCB->currentIndex() == 2)
    {
        QString temp = "4";
        command = temp.toUtf8();
    }
    qDebug() << command << endl;
    writeData(command);

}

void MainWindow::handleSerialError(QSerialPort::SerialPortError error)
//Handles and serial errors that may occur
{
    if(error == QSerialPort::ResourceError)
    {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::init_actionConnections()
// Initialization of all Buttons/Actions/ComboBox/etc
{
    connect(ui->actionConfigure, &QAction::triggered, settings, &MainWindow::show);
    connect(ui->closeButton, &QPushButton::clicked, this, &MainWindow::closeSerialPort);
    connect(ui->openButton, &QPushButton::clicked, this, &MainWindow::openSerialPort);

    ui->commandCB->addItem("Set FS");       /* Adding items to */
    ui->commandCB->addItem("Picture");      /* Command ComboBox */
    ui->commandCB->addItem("CUTDOWN");

    ui->stateCB->addItem("0");
    ui->stateCB->addItem("1");
    ui->stateCB->addItem("2");
    ui->stateCB->addItem("3");
    ui->stateCB->addItem("4");
    ui->stateCB->addItem("5");
    ui->stateCB->addItem("6");
    ui->stateCB->addItem("7");
}

void MainWindow::showStatusMessage(const QString &message)
{
    status->setText(message);
}

void MainWindow::save_to_file(QByteArray data)
/* Saves incoming data from serial read to output file */
/* NOTE: add ability to change filename from settings dialog */
{
    QString dataAsAString = data; //Convert QByteArray to QString

    QFile file("CANSAT2017_TLM_2694_POWERGLIDE.csv");
    if(file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        QTextStream stream(&file);
        stream << dataAsAString;
    }
}

void MainWindow::format_in_data(QString data)
/* Converts the Qbyte array to integers in the TelemetryPacket struct */
{
    //QString dataAsAString = data; //Convert QByteArray to QString
    QStringList Telemetry = data.split(',');

    if(VehicleState == GLIDER)
    {
        QString time = Telemetry[2];
        QString altitude = Telemetry[4];
        QString pressure = Telemetry[5];
        QString speed = Telemetry[6];
        QString temp = Telemetry[7];
        QString voltage = Telemetry[8];
        QString heading = Telemetry[10];
        QString state = Telemetry[10];

        bool ok;

        Packet.Time = time.toInt(&ok);
        Packet.Altitude = altitude.toInt(&ok);
        Packet.Pressure = pressure.toInt(&ok);
        Packet.Speed = speed.toInt(&ok);
        Packet.Temp = temp.toInt(&ok);
        Packet.Voltage = voltage.toInt(&ok);
        Packet.Heading = heading.toInt(&ok);
        Packet.State = state.toInt(&ok);
    }
    else if(VehicleState == CONTAINER)
    {
        QString time = Telemetry[2];
        QString altitude = Telemetry[4];
        QString temp = Telemetry[5];
        QString voltage = Telemetry[6];
        QString state = Telemetry[7];

        bool ok;

        Packet.Time = time.toInt(&ok);
        Packet.Altitude = altitude.toInt(&ok);
        Packet.Temp = temp.toInt(&ok);
        Packet.Voltage = voltage.toInt(&ok);
        Packet.State = state.toInt(&ok);
    }
}

//TODO: Partial pattern matching
QString MainWindow::MakePacketValid(QByteArray data)
{    
    QString currentPacket;
    QString matchedPacket;
    currentPacket = data;

    //Packet format:
    //TeamID,GLIDER,Mission Time, packet count, altitude, pressure, speed, temp, voltage, heading, sofware state, cameracount
    QRegularExpression GliderValid("2694,GLIDER,\\d{1,5},\\d{1,5},\\d{1,3},\\d{1,6},\\d{1,2},\\d{1,2},\\d{1,4},\\d{1,3},\\d,\\d{1,5}");

    //Container packet format:
    //TeamID,CONTAINER, Mission Time, packet count, altitude,temperature, voltage, software state
    QRegularExpression ContainerValid("2694,CONTAINER,\\d{1,5},\\d{1,5},\\d{1,3},\\d{1,2},\\d{1,4},\\d");

    QRegularExpressionMatch Gmatch = GliderValid.match(currentPacket);
    QRegularExpressionMatch Cmatch = ContainerValid.match(currentPacket);

    bool GhasMatch = Gmatch.hasMatch();
    bool ChasMatch = Cmatch.hasMatch();

    if(GhasMatch)
    {
        VehicleState = GLIDER;
        return matchedPacket;
    }
    else if(ChasMatch)
    {
        qDebug() << ChasMatch;
        VehicleState = CONTAINER;
        return currentPacket;
    }
    return "";

}

//This is complete garbage don't use
QString MainWindow::packetIsValid(QByteArray data)
//Determines if an incoming packet is valid
//if packet is incomplete, throws IncompletePacket() and saves that data until the remainder of
//the packet can be appended
//if packet is completely invalid, throws InvalidPacket() and ignores section of data
//
{
    QString temp;
    QString start = "2694";
    QString wait = ""; //returns wait if the rest of the packet needs to be waited on

    static QString currentPacket;
    QString returnPacket;
    static int packetLength;
    temp = data;
    QStringList tempList = temp.split(',');
    try
    {
        if(tempList[0] == start)
        {
            throw StartIncomplete();
        }
        else if(temp.endsWith('\n'))
        {
            throw EndIncomplete();
        }
        else if(!(packetLength >= 12))
        {
            currentPacket.append(temp);
        }
        else if(packetLength >= 12)
        {
            currentPacket = "";
            packetLength = 0;
        }
        else
            return wait;
    }
    catch(StartIncomplete)
    {
        currentPacket.append(temp);
        qDebug() << currentPacket;
    }
    catch(EndIncomplete)
    {
        currentPacket.append(temp);
        returnPacket = currentPacket;
        currentPacket = "";
        packetLength = 0;
        qDebug() << "Finished packet: " << returnPacket;
        return returnPacket;
    }

    return wait;
}

void MainWindow::HandlePositionReading(void)
{
    x_coord = qRadiansToDegrees(sin(Packet.Heading)) * Packet.Speed + previousXcoord;
    y_coord = qRadiansToDegrees(cos(Packet.Heading)) * Packet.Speed + previousYcoord;

    previousXcoord = x_coord;
    previousYcoord = y_coord;
}


void MainWindow::appendPoints()
{    
    qDebug() << "append" << endl;
    plot1->append(Packet.Time, Packet.Altitude);
    plot1->setAxisScale(QwtPlot::yLeft, Packet.Altitude -50, Packet.Altitude + 50);
    plot1->setAxisScale(QwtPlot::xBottom, 0, Packet.Time+50);
    plot1->replot();

    if(VehicleState == GLIDER)
    {
        plot2->append(Packet.Time, Packet.Pressure);
        plot2->setAxisScale(QwtPlot::yLeft, 0, Packet.Pressure+10000);
        plot2->setAxisScale(QwtPlot::xBottom, 0, Packet.Time+50);
        plot2->replot();


        plot3->append(Packet.Time, Packet.Speed);
        plot3->setAxisScale(QwtPlot::yLeft, 0, Packet.Speed+100);
        plot3->setAxisScale(QwtPlot::xBottom, 0, Packet.Time+50);
        plot3->replot();
    }

    plot4->append(Packet.Time, Packet.Temp);
    plot4->setAxisScale(QwtPlot::yLeft, 0, Packet.Pressure+100);
    plot4->setAxisScale(QwtPlot::xBottom, 0, Packet.Time+50);
    plot4->replot();

    plot5->append(Packet.Time, Packet.Voltage);
    plot5->setAxisScale(QwtPlot::yLeft, 0, Packet.Voltage+1000);
    plot5->setAxisScale(QwtPlot::xBottom, 0, Packet.Time+50);
    plot5->replot();

    if(VehicleState == GLIDER)
    {
        HandlePositionReading();
        positionMap->append(x_coord, y_coord);
        positionMap->setAxisScale(QwtPlot::yLeft,y_coord - 100, y_coord + 100);
        positionMap->setAxisScale(QwtPlot::xBottom, x_coord -100, x_coord +100);
        positionMap->replot();
    }
}
