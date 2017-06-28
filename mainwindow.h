#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>

#include <QMainWindow>

#include <QtSerialPort/QSerialPort>

#include <QFile>

class QLabel;
class Plot;

class IncompletePacket
{
    /* Empty class for Incomplete packet error
     * class is thrown when data read from serial port
     * is an incomplete packet
     */
};

class StartIncomplete
{
    /* Empty class for incomplete packet that starts
     * with 2694
     */
};

class EndIncomplete
{
    /* Empty class to catch end of packet exception
     *
     */
};

class InValidPacket
{
    /*Empty class for InvalidPacket error
     * handling. Class is thrown when data read
     * from serial port is completely invalid
     */
};

namespace Ui {
class MainWindow;
}

class Console;
class SettingsDialog;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

/*public:
    bool operator==(QCharRef leftop, QString rightop); //overloaded operator to compare item in QStringList with QString
*/
private slots:
    void openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void readData();

    void handleSendButton();
    void handleSerialError(QSerialPort::SerialPortError error);

private:
    void init_actionConnections();
    void save_to_file(QByteArray data);
    void format_in_data(QString data);
    QString packetIsValid(QByteArray data);
    QString MakePacketValid(QByteArray data);
    void HandlePositionReading(void);

private:
    struct TelemetryPacket{
        int Time;
        int Altitude;
        int Pressure;
        int Speed;
        int Temp;
        int Voltage;
        int Heading;
        int State;
    } Packet;

    enum VehicleState {
        CONTAINER,
        GLIDER
    };

private Q_SLOTS:
    void appendPoints();

private:
    int previousXcoord;
    int previousYcoord;
    int x_coord;
    int y_coord;

private:
    void showStatusMessage(const QString &message);

    Ui::MainWindow *ui;
    QLabel *status;
    Console *console;
    SettingsDialog *settings;
    QSerialPort *serial;
    QFile logFile;    
    Plot *plot1;
    Plot *plot2;
    Plot *plot3;
    Plot *plot4;
    Plot *plot5;

    Plot *positionMap;
    VehicleState VehicleState;
};

#endif // MAINWINDOW_H
