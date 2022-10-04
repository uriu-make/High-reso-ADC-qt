#include <QMainWindow>
#include <QBoxLayout>
#include <QCloseEvent>
// #include <QtConcurrent/QtConcurrent>
#include <QThread>
#include <QMutex>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
// #include <qwt_plot_zoomer.h>
// #include <qwt_picker.h>
// #include <qwt_picker_machine.h>
// #include <qwt_plot_magnifier.h>
// #include <qwt_plot_panner.h>
#include <QFileDialog>
#include <QDateTime>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "ADS1256.h"

int open_socket(const char *hostname, int Port);
int kill(int fd);
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
  void closeEvent(QCloseEvent *);

 private:
  QwtPlotCurve *curve;
  Ui::MainWindow *ui;
  QMutex mutex;
  QThread *test = new QThread(this);

  void timerEvent(QTimerEvent *);
  void run(void);

  static constexpr int plotDataSize = 100;
  static constexpr double gain = 1;

  // layout elements from Qt itself http://qt-project.org/doc/qt-4.8/classes.html
  QVBoxLayout *vLayout;  // vertical layout
  QHBoxLayout *hLayout;  // horizontal layout

  bool stopped = false;
  int len;
  int64_t t_0;
  double xData[plotDataSize] = {0};
  int64_t xData_buf[plotDataSize] = {0};
  double yData[plotDataSize] = {0};

  int timerID;
  double volt_range_p, volt_range_n, center, range;
  int t_range, t_center, t_range_p, t_range_n;
  std::string hostname, Port;

  int sock;
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

  uint8_t mode_list[3] = {0};

  struct COMMAND com;
  // struct read_data data[plotDataSize];

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
};
#endif  // MAINWINDOW_H
