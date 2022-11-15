#include <QMainWindow>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QThread>
#include <QMutex>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <QFileDialog>
#include <QDateTime>
#include <algorithm>
#include <iostream>
#include <fstream>

#include <unistd.h>
#include <iomanip>

#include "ADS1256.h"

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

 protected:
  void closeEvent(QCloseEvent *) override;

 private:
  QwtPlotCurve *curve;
  Ui::MainWindow *ui;
  QMutex mutex;
  QTcpSocket *sock;

  void timerEvent(QTimerEvent *);

  static constexpr int _plotDataSize = 100000;
  static constexpr double _gain = 1;

  QVBoxLayout *vLayout;  
  QHBoxLayout *hLayout;  
  int _len;
  int64_t t_0;

  int64_t xData_buf[_plotDataSize] = {0};
  double xData[_plotDataSize] = {0};
  double yData[_plotDataSize] = {0};

  int timerID;
  double volt_range_p, volt_range_n, center, range;
  int t_range, t_center, t_range_p, t_range_n;

  QString hostname;
  QString Port;

  struct COMMAND command;
  struct read_data data;

  int l_sum = 0;
  int writepoint = 0;
  QByteArray buffer;

 private slots:
  void run_measure();
  void change_volt_range(double value);
  void change_time_range(int value);
  void change_time_center(int value);

  void change_volt_center(double value);
  void range_reset(void);
  void samplerange_reset(void);

  void range_fine_func(int checked);
  void center_fine_func(int checked);
  void time_fine_func(int checked);
  void time_center_fine_func(int checked);

  void connect_socket();
  void save_as();
  void config();

  void read_task();
};
#endif  // MAINWINDOW_H
