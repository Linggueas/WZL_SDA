#ifndef DATASHOWDIALOG_H
#define DATASHOWDIALOG_H

#include <QDialog>
#include <QtCharts>
#include <QVector>
#include <QWheelEvent>
#include <QMouseEvent>

QT_BEGIN_NAMESPACE
namespace Ui { class DataShowDialog; }
QT_END_NAMESPACE

class DataShowDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DataShowDialog(QWidget *parent = nullptr);
    ~DataShowDialog();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void on_mess_pushButton_clicked();
    void on_start_pushButton_clicked();
    void on_stop_pushButton_clicked();
    void on_save_pushButton_clicked();
    void on_start_show_data(QByteArray data);
    void on_clear_pushButton_clicked();
    void on_resetView_pushButton_clicked();
    void on_chartTypeComboBox_currentIndexChanged(int index);

private:
    void updateChartView();
    void autoScroll();

    Ui::DataShowDialog *ui;
    QChart *chart;
    QChartView *chart_view;
    QLineSeries *line_series;
    QValueAxis *axisX;
    QValueAxis *axisY;
    
    // 数据存储
    QVector<QPointF> dataPoints;
    
    // 视图控制
    int maxX;
    int maxY;
    int index;
    static const int MAX_DISPLAY_POINTS = 10;  // 最大显示点数
    
    // 鼠标交互
    bool isDragging;
    QPoint lastMousePos;
    double viewOffset;
    bool isAutoScroll;
};

#endif // DATASHOWDIALOG_H
