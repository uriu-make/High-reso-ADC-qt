#include <QMainWindow>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QThread>
#include <QMutex>
#include <QTcpSocket>

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

  // layout elements from Qt itself http://qt-project.org/doc/qt-4.8/classes.html
  QVBoxLayout *vLayout;  // vertical layout
  QHBoxLayout *hLayout;  // horizontal layout

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

  uint8_t rate_list[16] = {
      DATARATE_30000,
      DATARATE_15000,
      DATARATE_7500,
      DATARATE_3750,
      DATARATE_2000,
      DATARATE_1000,
      DATARATE_500,
      DATARATE_100,
      DATARATE_60,
      DATARATE_50,
      DATARATE_30,
      DATARATE_25,
      DATARATE_15,
      DATARATE_10,
      DATARATE_5,
      DATARATE_2_5};

  uint8_t gain_list[7] = {
      GAIN_1,
      GAIN_2,
      GAIN_4,
      GAIN_8,
      GAIN_16,
      GAIN_32,
      GAIN_64};

  uint8_t ain_list[9] = {
      AIN0,
      AIN1,
      AIN2,
      AIN3,
      AIN4,
      AIN5,
      AIN6,
      AIN7,
      AGND};

  uint8_t mode_list[2] = {0, 1};
  struct COMMAND command;
  struct read_data data;
//   union T_DATA tx;
//   union R_DATA rx;
  int l_sum = 0;
  int writepoint = 0;

  // struct read_data data[_plotDataSize];

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
