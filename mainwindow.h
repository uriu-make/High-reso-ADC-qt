#include <QMainWindow>
#include <QBoxLayout>

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

//サンプリングレート
#define DATARATE_30000 0b11110000
#define DATARATE_15000 0b11100000
#define DATARATE_7500  0b11010000
#define DATARATE_3750  0b11000000
#define DATARATE_2000  0b10110000
#define DATARATE_1000  0b10100001
#define DATARATE_500   0b10010010
#define DATARATE_100   0b10000010
#define DATARATE_60    0b01110010
#define DATARATE_50    0b01100011
#define DATARATE_30    0b01010011
#define DATARATE_25    0b01000011
#define DATARATE_15    0b00110011
#define DATARATE_10    0b00100011
#define DATARATE_5     0b00010011
#define DATARATE_2_5   0b00000011

// PGAゲイン
#define GAIN_1  0b000
#define GAIN_2  0b001
#define GAIN_4  0b010
#define GAIN_8  0b011
#define GAIN_16 0b100
#define GAIN_32 0b101
#define GAIN_64 0b110

#define AIN0 0b0000
#define AIN1 0b0001
#define AIN2 0b0010
#define AIN3 0b0011
#define AIN4 0b0100
#define AIN5 0b0101
#define AIN6 0b0110
#define AIN7 0b0111
#define AGND 0b1000

struct COMMAND {
  uint8_t rate;
  uint8_t gain;
  uint8_t positive;
  uint8_t negative;
  uint8_t buf;
  uint8_t sync;
  uint8_t mode;
  uint8_t run;
  uint8_t kill = 0;
};

int open_socket(const char *hostname, int Port);

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

 private:
  QwtPlotCurve *curve;
  Ui::MainWindow *ui;
  void timerEvent(QTimerEvent *);
  static constexpr int plotDataSize = 10000;
  static constexpr double gain = 1;

  // layout elements from Qt itself http://qt-project.org/doc/qt-4.8/classes.html
  QVBoxLayout *vLayout;  // vertical layout
  QHBoxLayout *hLayout;  // horizontal layout
  // QwtPlotPicker *picker;
  // data arrays for the plot
  double xData[plotDataSize] = {0};
  double yData[plotDataSize] = {0};

  long count = 0;
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

 private slots:
  void run();
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
