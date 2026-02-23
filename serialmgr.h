#ifndef SERIALMGR_H
#define SERIALMGR_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
class SerialMgr : public QObject
{
    Q_OBJECT
public:
    explicit SerialMgr(QObject *parent = nullptr);
    ~SerialMgr();
    static SerialMgr*GetInstance();
    QList<QSerialPortInfo> get_now_port();
    bool openSerial(QString serialNum,
                    QString serialBaud,
                    QString serialData,
                    QString serialVerify,
                    QString serialStop,
                    QString serialStream);
    void closeSerial();
    void send_data(const QString&data);
private:
    QList<QSerialPortInfo>now_port;
    static SerialMgr *ser_mgr;
    QSerialPort *serialport;



private slots:
    void read_data();
signals:
    void sig_read_data(QByteArray data);
};

#endif // SERIALMGR_H
