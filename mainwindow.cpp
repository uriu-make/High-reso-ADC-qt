#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  QwtPlotGrid *grid = new QwtPlotGrid();

  volt_range_p = 5.0;
  volt_range_n = -5.0;
  center = 0;
  range = 5;
  t_range = 100;
  t_center = std::clamp(_plotDataSize - t_range, 0, _plotDataSize);
  t_range_p = std::clamp(t_center + t_range - 1, 0, _plotDataSize) - t_center;
  t_range_n = std::clamp(t_center - t_range - 1, 0, _plotDataSize) - t_center;

  curve = new QwtPlotCurve;

  curve->setSamples(xData, yData, _plotDataSize);
  curve->attach(ui->qwtPlot);
  grid->attach(ui->qwtPlot);

  curve->setPen(QPen(QBrush(QColor::fromRgb(0xAFAF0F)), 2.0));
  grid->setPen(QPen(QBrush(QColor::fromRgb(100, 100, 100)), 2.0));
  ui->qwtPlot->replot();
  ui->qwtPlot->show();
  ui->qwtPlot->setAxisTitle(0, tr("volt"));
  ui->qwtPlot->setAxisTitle(2, tr("time"));
  //  ui->qwtPlot->setAxis
  // ui->qwtPlot->setAxisScale(QwtPlot::yLeft, volt_range_n, volt_range_p);
  // ui->qwtPlot->setAxisScale(QwtPlot::xBottom, t_range_n, t_range_p + 1, (t_range_p - t_range_n) / 10);

  ui->lcdNumber->setSegmentStyle(QLCDNumber::Flat);
  QPalette palette = ui->lcdNumber->palette();
  palette.setColor(palette.Light, QColor(255, 255, 255));
  ui->lcdNumber->setPalette(palette);
  // ui->run->setCheckable(true);
  ui->in_n->setCurrentIndex(8);

  connect(ui->run, SIGNAL(clicked()), this, SLOT(run_measure()));
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
  mutex.lock();
  curve->setSamples(&xData[writepoint], &yData[writepoint], _plotDataSize - writepoint);
  ui->qwtPlot->setAxisScale(QwtPlot::xBottom, xData[writepoint], xData[_plotDataSize - 1]);
  ui->qwtPlot->replot();
  ui->lcdNumber->display(yData[std::clamp(t_center - 1, 0, _plotDataSize - 1)]);
  ui->lcdNumber->show();
  mutex.unlock();
}

void MainWindow::change_volt_range(double value) {
  volt_range_p = center + value;
  volt_range_n = center - value;
  range = value;
  volt_range_p = std::clamp(volt_range_p, -5.0, 5.0);
  volt_range_n = std::clamp(volt_range_n, -5.0, 5.0);

  // ui->qwtPlot->setAxisScale(QwtPlot::yLeft, volt_range_n, volt_range_p);
  ui->qwtPlot->replot();
}

void MainWindow::change_time_range(int value) {
  t_range = value;
  t_center = std::clamp(_plotDataSize - t_range, 0, _plotDataSize);
  t_range_p = std::clamp(t_center + t_range - 1, 0, _plotDataSize) - t_center;
  t_range_n = std::clamp(t_center - t_range - 1, 0, _plotDataSize) - t_center;
  for (int i = 0; i < _plotDataSize; i++) {
    xData[i] = i - t_center + 1;
  }
  t_center = std::clamp(t_center, 0, _plotDataSize - 1);
  // ui->qwtPlot->setAxisScale(QwtPlot::xBottom, t_range_n, t_range_p + 1, (t_range_p - t_range_n) / 10);
  curve->setSamples(xData, yData, _plotDataSize);

  ui->qwtPlot->replot();
  ui->lcdNumber->display(yData[std::clamp(t_center - 1, 0, _plotDataSize - 1)]);
  ui->lcdNumber->show();
}

void MainWindow::change_volt_center(double value) {
  volt_range_p = range + value;
  volt_range_n = value - range;
  center = value;
  volt_range_p = std::clamp(volt_range_p, -5.0, 5.0);
  volt_range_n = std::clamp(volt_range_n, -5.0, 5.0);
  // ui->qwtPlot->setAxisScale(QwtPlot::yLeft, volt_range_n, volt_range_p);
  ui->qwtPlot->replot();
}

void MainWindow::change_time_center(int value) {
  t_center = std::clamp(_plotDataSize - t_range + value, 0, _plotDataSize);
  t_range_p = std::clamp(t_center + t_range - 1, 0, _plotDataSize) - t_center;
  t_range_n = std::clamp(t_center - t_range - 1, 0, _plotDataSize) - t_center;

  for (int i = 0; i < _plotDataSize; i++) {
    xData[i] = i - t_center + 1;
  }
  // ui->qwtPlot->setAxisScale(QwtPlot::xBottom, t_range_n, t_range_p + 1, (t_range_p - t_range_n) / 10);
  curve->setSamples(xData, yData, _plotDataSize);
  // ui->time_center->setValue(std::clamp(value, -t_range, t_range));
  ui->qwtPlot->replot();
  ui->lcdNumber->display(yData[std::clamp(t_center - 1, 0, _plotDataSize - 1)]);
  ui->lcdNumber->show();
}

void MainWindow::run_measure() {
  if (ui->run->text().compare("Run", Qt::CaseSensitive) == 0) {
    ui->save->setEnabled(false);
    ui->config->setEnabled(false);
    ui->Connect->setEnabled(false);
    ui->run->setText("Stop");
    for (int i = 0; i < _plotDataSize; i++) {
      xData[i] = 0.0;
      yData[i] = 0.0;
    }
    com.kill = 0;
    com.run = 1;
    _stopped = false;
    write(sock, &com, sizeof(com));
    timerID = this->startTimer(0);
  } else {
    ui->save->setEnabled(true);
    ui->config->setEnabled(true);
    ui->Connect->setEnabled(true);
    com.run = 0;
    write(sock, &com, sizeof(com));
    this->killTimer(timerID);
    _stopped = true;
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
  // ui->qwtPlot->setAxisScale(QwtPlot::yLeft, volt_range_n, volt_range_p);
  ui->qwtPlot->replot();
}

void MainWindow::samplerange_reset() {
  ui->time_range->setValue(100);
  ui->time_center->setValue(0);

  t_range = 100;
  t_center = std::clamp(_plotDataSize - t_range, 0, _plotDataSize);
  t_range_p = std::clamp(t_center + t_range - 1, 0, _plotDataSize) - t_center;
  t_range_n = std::clamp(t_center - t_range - 1, 0, _plotDataSize) - t_center;
  // ui->qwtPlot->setAxisScale(QwtPlot::xBottom, t_range_n, t_range_p + 1, (t_range_p - t_range_n) / 10);
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
  for (int i = 0; i < _plotDataSize; i++) {
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
      ui->statusBar->showMessage(QString::fromStdString(hostname) + ":" + QString::fromStdString(Port) + " is not Connected.", 5000);
      return;
    } else {
      ui->statusBar->showMessage(QString::fromStdString(hostname) + ":" + QString::fromStdString(Port) + " is Connected.", 5000);
      ui->Connect->setText("Disconnect");
      ui->config->setEnabled(true);
      worker = new Worker(&mutex, &_stopped, sock, _plotDataSize, xData, yData, &writepoint);
      _stopped = true;
      worker->start();
    }
  } else {
    _stopped = true;
    worker->requestInterruption();
    while (worker->isRunning())
      ;
    worker->terminate();
    worker->wait();
    ui->Connect->setText("Connect");
    ui->statusBar->showMessage(QString::fromStdString(hostname) + ":" + QString::fromStdString(Port) + " is Disconnected.", 5000);
    ui->config->setEnabled(false);
    ui->run->setEnabled(false);
    ui->save->setEnabled(false);

    com.kill = 1;
    write(sock, &com, sizeof(com));
    kill(sock);
    free(worker);
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
  if (ui->analogbuffer->isChecked()) {
    com.buf = 1;
  } else {
    com.buf = 0;
  }

  if (ui->sync->isChecked()) {
    com.sync = 1;
  } else {
    com.sync = 0;
  }

  com.run = 0;
  com.kill = 0;
  ui->run->setEnabled(true);

  write(sock, &com, sizeof(com));
}

void MainWindow::closeEvent(QCloseEvent *event) {
  com.run = 0;
  com.kill = 1;
  write(sock, &com, sizeof(com));
  kill(sock);
  event->accept();
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
  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    return -1;
  } else {
    return sock;
  }
}

int kill(int fd) {
  return close(fd);
}