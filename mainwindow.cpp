#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  QwtPlotGrid *grid = new QwtPlotGrid();
  // QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(ui->qwtPlot->canvas());
  // magnifier->setMouseButton(Qt::LeftButton);
  // QwtPlotPanner *panner = new QwtPlotPanner(ui->qwtPlot->canvas());
  // panner->setMouseButton(Qt::RightButton);

  // picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::ActiveOnly, ui->qwtPlot->canvas());
  // picker->setRubberBandPen(QColor(Qt::blue));
  // picker->setTrackerPen(QColor(Qt::gray));
  // picker->setStateMachine();//new QwtPickerDragPointMachine()
  volt_range_p = 5.0;
  volt_range_n = -5.0;
  center = 0;
  range = 5;
  t_range = 100;
  t_center = std::clamp(plotDataSize - t_range, 0, plotDataSize);
  t_range_p = std::clamp(t_center + t_range - 1, 0, plotDataSize) - t_center;
  t_range_n = std::clamp(t_center - t_range - 1, 0, plotDataSize) - t_center;

  for (int i = 0; i < plotDataSize; i++) {
    yData[i] = 0;
    xData[i] = i - t_center + 1;
  }
  curve = new QwtPlotCurve;
  // qwtPlot = new QwtPlot;
  curve->setSamples(xData, yData, plotDataSize);
  curve->attach(ui->qwtPlot);
  grid->attach(ui->qwtPlot);

  curve->setPen(QPen(QBrush(QColor::fromRgb(255, 0, 0)), 2.0));
  grid->setPen(QPen(QBrush(QColor::fromRgb(100, 100, 100)), 2.0));
  ui->qwtPlot->replot();
  ui->qwtPlot->show();
  ui->qwtPlot->setAxisTitle(0, tr("volt"));
  ui->qwtPlot->setAxisTitle(2, tr("time"));
  //  ui->qwtPlot->setAxis
  ui->qwtPlot->setAxisScale(QwtPlot::yLeft, volt_range_n, volt_range_p);
  ui->qwtPlot->setAxisScale(QwtPlot::xBottom, t_range_n, t_range_p + 1, (t_range_p - t_range_n) / 10);

  ui->lcdNumber->setSegmentStyle(QLCDNumber::Flat);
  QPalette palette = ui->lcdNumber->palette();
  palette.setColor(palette.Light, QColor(255, 255, 255));
  ui->lcdNumber->setPalette(palette);
  // ui->run->setCheckable(true);

  connect(ui->run, SIGNAL(clicked()), this, SLOT(run()));
  connect(ui->volt_range, SIGNAL(valueChanged(double)), this, SLOT(change_volt_range(double)));
  connect(ui->center, SIGNAL(valueChanged(double)), this, SLOT(change_volt_center(double)));
  connect(ui->time_range, SIGNAL(valueChanged(int)), this, SLOT(change_time_range(int)));
  connect(ui->time_center, SIGNAL(valueChanged(int)), this, SLOT(change_time_center(int)));

  connect(ui->reset_range, SIGNAL(clicked()), this, SLOT(range_reset()));
  connect(ui->samplereset, SIGNAL(clicked()), this, SLOT(samplerange_reset()));

  connect(ui->range_fine, SIGNAL(stateChanged(int)), this, SLOT(range_fine_func(int)));
  connect(ui->center_range, SIGNAL(stateChanged(int)), this, SLOT(center_fine_func(int)));
  connect(ui->time_fine, SIGNAL(stateChanged(int)), this, SLOT(time_fine_func(int)));
  connect(ui->time_center_fine, SIGNAL(stateChanged(int)), this, SLOT(time_center_fine_func(int)));

  connect(ui->Connect, SIGNAL(clicked()), this, SLOT(connect_socket()));
  connect(ui->save, SIGNAL(clicked()), this, SLOT(save_as()));
  connect(ui->config, SIGNAL(clicked()), this, SLOT(config()));
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::timerEvent(QTimerEvent *) {
  double inVal = 3.3 + 0.01 * gain * sin(M_PI * count / 10);
  ++count;

  // add the new input to the plot
  // std::move(yData + 1, yData + plotDataSize - 1, yData);
  std::rotate(yData, yData + 1, yData + plotDataSize);
  for (int i = 0; i < plotDataSize; i++) {
    xData[i] = i - t_center + 1;
  }

  yData[plotDataSize - 1] = inVal;
  // xData[0] = count;
  curve->setSamples(xData, yData, plotDataSize);
  ui->qwtPlot->replot();
  ui->lcdNumber->display(yData[std::clamp(t_center - 1, 0, plotDataSize - 1)]);
  ui->lcdNumber->show();
}

void MainWindow::change_volt_range(double value) {
  volt_range_p = center + value;
  volt_range_n = center - value;
  range = value;
  volt_range_p = std::clamp(volt_range_p, -5.0, 5.0);
  volt_range_n = std::clamp(volt_range_n, -5.0, 5.0);

  ui->qwtPlot->setAxisScale(QwtPlot::yLeft, volt_range_n, volt_range_p);
  ui->qwtPlot->replot();
}

void MainWindow::change_time_range(int value) {
  t_range = value;
  t_center = std::clamp(plotDataSize - t_range, 0, plotDataSize);
  t_range_p = std::clamp(t_center + t_range - 1, 0, plotDataSize) - t_center;
  t_range_n = std::clamp(t_center - t_range - 1, 0, plotDataSize) - t_center;
  for (int i = 0; i < plotDataSize; i++) {
    xData[i] = i - t_center + 1;
  }
  t_center = std::clamp(t_center, 0, plotDataSize - 1);
  ui->qwtPlot->setAxisScale(QwtPlot::xBottom, t_range_n, t_range_p + 1, (t_range_p - t_range_n) / 10);
  curve->setSamples(xData, yData, plotDataSize);

  ui->qwtPlot->replot();
  ui->lcdNumber->display(yData[std::clamp(t_center - 1, 0, plotDataSize - 1)]);
  ui->lcdNumber->show();
}

void MainWindow::change_volt_center(double value) {
  volt_range_p = range + value;
  volt_range_n = value - range;
  center = value;
  volt_range_p = std::clamp(volt_range_p, -5.0, 5.0);
  volt_range_n = std::clamp(volt_range_n, -5.0, 5.0);
  ui->qwtPlot->setAxisScale(QwtPlot::yLeft, volt_range_n, volt_range_p);
  ui->qwtPlot->replot();
}

void MainWindow::change_time_center(int value) {
  t_center = std::clamp(plotDataSize - t_range + value, 0, plotDataSize);
  t_range_p = std::clamp(t_center + t_range - 1, 0, plotDataSize) - t_center;
  t_range_n = std::clamp(t_center - t_range - 1, 0, plotDataSize) - t_center;

  for (int i = 0; i < plotDataSize; i++) {
    xData[i] = i - t_center + 1;
  }
  ui->qwtPlot->setAxisScale(QwtPlot::xBottom, t_range_n, t_range_p + 1, (t_range_p - t_range_n) / 10);
  curve->setSamples(xData, yData, plotDataSize);
  // ui->time_center->setValue(std::clamp(value, -t_range, t_range));
  ui->qwtPlot->replot();
  ui->lcdNumber->display(yData[std::clamp(t_center - 1, 0, plotDataSize - 1)]);
  ui->lcdNumber->show();
}

void MainWindow::run() {
  if (ui->run->text().compare("Run", Qt::CaseSensitive) == 0) {
    ui->save->setEnabled(false);
    ui->config->setEnabled(false);
    ui->Connect->setEnabled(false);
    ui->run->setText("Stop");
    for (int index = 0; index < plotDataSize; ++index) {
      xData[index] = index;
      yData[index] = 0;
    }
    timerID = this->startTimer(0);
  } else {
    ui->save->setEnabled(true);
    ui->config->setEnabled(true);
    ui->Connect->setEnabled(true);
    this->killTimer(timerID);
    ui->run->setText("Run");
  }
}

void MainWindow::range_reset() {
  ui->volt_range->setValue(5.0);
  ui->center->setValue(0.0);

  volt_range_p = 5.0;
  volt_range_n = -5.0;
  center = 0;
  range = 5;
  ui->qwtPlot->setAxisScale(QwtPlot::yLeft, volt_range_n, volt_range_p);
  ui->qwtPlot->replot();
}

void MainWindow::samplerange_reset() {
  ui->time_range->setValue(100);
  ui->time_center->setValue(0);

  t_range = 100;
  t_center = std::clamp(plotDataSize - t_range, 0, plotDataSize);
  t_range_p = std::clamp(t_center + t_range - 1, 0, plotDataSize) - t_center;
  t_range_n = std::clamp(t_center - t_range - 1, 0, plotDataSize) - t_center;
  ui->qwtPlot->setAxisScale(QwtPlot::xBottom, t_range_n, t_range_p + 1, (t_range_p - t_range_n) / 10);
  ui->qwtPlot->replot();
}

void MainWindow::range_fine_func(int checked) {
  if (checked) {
    ui->volt_range->setSingleStep(0.5);
    ui->volt_range->setMinimum(0.5);

  } else {
    ui->volt_range->setSingleStep(0.001);
    ui->volt_range->setMinimum(0.001);
  }
}

void MainWindow::center_fine_func(int checked) {
  if (checked) {
    ui->center->setSingleStep(0.5);
    ui->center->setMinimum(0.0);

  } else {
    ui->center->setSingleStep(0.001);
    ui->center->setMinimum(0.000);
  }
}

void MainWindow::time_fine_func(int checked) {
  if (checked) {
    ui->time_range->setSingleStep(100);
    ui->time_range->setMinimum(100);
  } else {
    ui->time_range->setSingleStep(10);
    ui->time_range->setMinimum(10);
  }
}

void MainWindow::time_center_fine_func(int checked) {
  if (checked) {
    ui->time_center->setSingleStep(10);
    // ui->time_center->setMinimum(-5000);
  } else {
    ui->time_center->setSingleStep(1);
    // ui->time_center->setMinimum(-5000);
  }
}

void MainWindow::save_as() {
  QString filters("CSV files (.csv);;All files(*.*)");
  QString defaultFilter("CSV files (.csv)");
  QString filepath;
  QDateTime date = QDateTime::currentDateTime();
  QFileDialog dialog;
  dialog.setOption(QFileDialog::HideNameFilterDetails, false);
  filepath = dialog.getSaveFileName(0, tr("名前を付けて保存"), "plot-" + date.toString("yyyy-MM-dd-HH-mm") + ".csv", filters, &defaultFilter);
  std::ofstream output{filepath.toUtf8().data()};
  output << "Volt,time" << std::endl;
  for (int i = t_center - t_range; i < t_center + t_range; i++) {
    output << std::to_string(yData[i]) + "," + std::to_string(xData[i]) << std::endl;
  }
  output.close();
}

void MainWindow::connect_socket() {
  if (ui->Connect->text().compare("Connect", Qt::CaseSensitive) == 0) {
    hostname = ui->Host->text().toStdString();
    Port = ui->Port->text().toStdString();
    struct addrinfo hints = {0}, *info;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(hostname.c_str(), NULL, &hints, &info) < 0) {
      ui->statusBar->showMessage(QString::fromStdString(hostname) + " is not found.");
      return;
    }
    sock = open_socket(hostname.c_str(), std::stoi(Port));
    if (sock < 0) {
      ui->statusBar->showMessage(QString::fromStdString(hostname) + ":" + QString::fromStdString(Port) + " is not Connect.", 5000);
      return;
    } else {
      ui->statusBar->showMessage(QString::fromStdString(hostname) + ":" + QString::fromStdString(Port) + " is Connect.", 5000);
      ui->Connect->setText("Disconnect");
      ui->config->setEnabled(true);
    }
  } else {
    ui->Connect->setText("Connect");
    ui->statusBar->showMessage(QString::fromStdString(hostname) + ":" + QString::fromStdString(Port) + " is Disconnect.", 5000);
    ui->config->setEnabled(false);
    ui->run->setEnabled(false);
    ui->save->setEnabled(false);
  }
}

void MainWindow::config() {
  int rate = ui->Rate->currentIndex();
  int pga = ui->PGA->currentIndex();
  int pin = ui->in_p->currentIndex();
  int nin = ui->in_n->currentIndex();
  int mode = ui->mode->currentIndex();

  com.rate = rate_list[rate];
  com.gain = gain_list[pga];
  com.positive = ain_list[pin];
  com.negative = ain_list[nin];
  com.mode = mode_list[mode];

  ui->run->setEnabled(true);
  /*
  uint8_t rate;
  uint8_t gain;
  uint8_t positive;
  uint8_t negative;
  uint8_t buf;
  uint8_t sync;
  uint8_t mode;
  uint8_t run;
  uint8_t kill = 0;
  */
}

int open_socket(const char *hostname, int Port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  struct addrinfo hints = {0}, *info;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(hostname, NULL, &hints, &info) < 0) {
    return -1;
  }

  struct sockaddr_in server_addr = {0};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = ((struct sockaddr_in *)(info->ai_addr))->sin_addr.s_addr;
  server_addr.sin_port = htons(Port);
  connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
  return sock;
}