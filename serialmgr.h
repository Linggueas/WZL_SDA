#ifndef SERIALMGR_H
#define SERIALMGR_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <memory>
class MainWindow;
class SerialMgr : public QObject
{
    friend MainWindow;
    Q_OBJECT
public:
    ~SerialMgr();
    static std::shared_ptr<SerialMgr> GetInstance();
private:
    explicit SerialMgr(QObject *parent = nullptr);
    QList<QSerialPortInfo>now_port;
    static std::shared_ptr<SerialMgr>ser_mgr;
    QSerialPort *serialport;



private slots:
    void read_data();
    void send_data(const QString&data);
    void openSerial(QString serialNum,
                    QString serialBaud,
                    QString serialData,
                    QString serialVerify,
                    QString serialStop,
                    QString serialStream);
    void closeSerial();
    void get_now_port();
signals:
    void sig_read_data(QByteArray data);
    void connect_state(bool _bool);
    void sig_get_port(QList<QSerialPortInfo>);
};

#endif // SERIALMGR_H
