#ifndef DATASHOWDIALOG_H
#define DATASHOWDIALOG_H

#include <QDialog>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
namespace Ui {
class DataShowDialog;
}

class DataShowDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DataShowDialog(QWidget *parent = nullptr);
    ~DataShowDialog();

private slots:
    void on_mess_pushButton_clicked();

    void on_start_pushButton_clicked();

    void on_stop_pushButton_clicked();

    void on_save_pushButton_clicked();

    void on_start_show_data(QByteArray data);

    void on_clear_pushButton_clicked();

private:
    Ui::DataShowDialog *ui;
    QChart *chart;
    QChartView *chart_view;
    QLineSeries *line_series;
    QValueAxis *axisX;
    QValueAxis *axisY;
    int maxX;
    int maxY;
    int index;
};

#endif // DATASHOWDIALOG_H
