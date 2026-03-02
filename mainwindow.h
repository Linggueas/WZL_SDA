#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <serialmgr.h>
#include <QTimer>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QThread>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <datashowdialog.h>
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_open_stop_pushButton_clicked();

    void on_send_pushButton_clicked();

    void on_close_rev_pushButton_clicked();

    void on_save_rev_pushButton_clicked();


    void on_time_checkBox_clicked(bool checked);

    void read_data(QByteArray data);
    void on_pushButton_13_clicked();

    void on_show_text_pushButton_clicked();

    void on_more_text_send();
    void on_while_checkBox_clicked(bool checked);
    void while_send_text();

    void on_Reset_pushButton_clicked();

    void on_save_pushButton_clicked();

    void on_down_pushButton_clicked();

    void on_data_show_pushButton_clicked();
signals:
    void sig_open_serial(QString serialNum, QString serialBaud, QString serialData, QString serialVerify, QString serialStop, QString serialStream);
    void sig_close_serial();
    void sig_get_port();
    void sig_write(const QString&data);
private:
    Ui::MainWindow *ui;
    bool if_open;
    bool his_open;
    bool more_text_open;
    bool data_show_open;
    QTimer*timer;

    QTimer*more_text_timer;

    int more_text_index;
    int serial_index;
    QThread *serial_thread;

    QChart *a;

    DataShowDialog*ds_dlg;

    QVector<QPushButton*> more_text_button;
    QVector<QLineEdit*> more_text_lineedit;
    QVector<QCheckBox*> more_text_checkbox;
};
#endif // MAINWINDOW_H
