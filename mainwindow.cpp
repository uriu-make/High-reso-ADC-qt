#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  QwtPlotGrid *grid = new QwtPlotGrid();
  this->sock = new QTcpSocket(this);

  volt_range_p = 5.0;
  volt_range_n = -5.0;
  center = 0;
  range = 5;
  t_range = _plotDataSize / 2;
  t_center = _plotDataSize / 2;
  t_range_p = _plotDataSize - 1;
  t_range_n = 0;

  curve = new QwtPlotCurve;

  curve->setSamples(xData, yData, _plotDataSize);
  curve->attach(ui->qwtPlot);
  grid->attach(ui->qwtPlot);

  curve->setPen(QPen(QBrush(QColor::fromRgb(0xAF, 0xAF, 0x0F, 200)), 2.0));
  grid->setPen(QPen(QBrush(QColor::fromRgb(100, 100, 100)), 2.0));
  ui->qwtPlot->replot();
  ui->qwtPlot->show();
  ui->qwtPlot->setAxisScale(QwtPlot::yLeft, volt_range_n, volt_range_p);

  ui->volt->setText(QString::number(0.0, 'g', 9) + "V");

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
  connect(ui->saveall, SIGNAL(clicked()), this, SLOT(save_all()));
  connect(ui->config, SIGNAL(clicked()), this, SLOT(config()));
  connect(ui->set_t0, SIGNAL(clicked()), this, SLOT(change_t0()));

  connect(this->sock, SIGNAL(readyRead()), this, SLOT(read_task()));
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::timerEvent(QTimerEvent *) {
  mutex.lock();
  QByteArray temp;
  if (ui->mode->currentIndex() == 1) {
    if ((unsigned int)buffer.size() >= sizeof(fft_read)) {
      temp = buffer.mid(0, sizeof(fft_read));
      buffer.remove(0, sizeof(fft_read));
      memcpy(&fft_read, temp.constData(), sizeof(fft_read));
      temp.clear();

      for (int i = 0; i < N; i++) {
        xData[i] = fft_read.freq_bin * i;
        yData[i] = std::abs(fft_read.F[i]) / static_cast<double>(N);
      }
      ui->label_7->setText("Range");
      ui->qwtPlot->setAxisTitle(0, tr(""));
      ui->qwtPlot->setAxisTitle(2, tr(""));
      ui->volt->setText("");
      ui->qwtPlot->setAxisScale(QwtPlot::xBottom, -10, N / 2);
      curve->setSamples(xData, yData, N);
      ui->qwtPlot->replot();
    }
  } else {
    while ((unsigned int)buffer.size() >= sizeof(read_data)) {
      temp = buffer.mid(0, sizeof(read_data));

      buffer.remove(0, sizeof(read_data));
      memcpy(&data, temp.constData(), sizeof(data));
      temp.clear();

      if (data.len < 0) {
        break;
      }
      l_sum = std::clamp(l_sum + data.len, 0, _plotDataSize - 1);

      writepoint = _plotDataSize - l_sum;
      memmove(yData, &yData[data.len], sizeof(double) * (_plotDataSize - data.len));
      memmove(xData_buf, &xData_buf[data.len], sizeof(int64_t) * (_plotDataSize - data.len));
      memcpy(&yData[_plotDataSize - data.len], data.volt, sizeof(double) * data.len);
      memcpy(&xData_buf[_plotDataSize - data.len], data.t, sizeof(int64_t) * data.len);
    }

    if (writepoint > _plotDataSize / 2) {
      t_0 = xData_buf[writepoint];
    } else {
      t_0 = xData_buf[_plotDataSize / 2];
    }
    double average = 0.0;
    int ave_count = 0;
    for (int i = writepoint - 1; i < _plotDataSize; i++) {
      xData[i] = (xData_buf[i] - t_0) / 1000000.0;
      if (i >= t_range_n && i <= t_range_p) {
        average += yData[i];
        ave_count++;
      }
    }
    average = average / ave_count;
    curve->setSamples(&xData[writepoint], &yData[writepoint], _plotDataSize - writepoint);
    if (writepoint > t_range_n) {
      ui->qwtPlot->setAxisScale(QwtPlot::xBottom, xData[writepoint], xData[t_range_p]);
    } else {
      ui->qwtPlot->setAxisScale(QwtPlot::xBottom, xData[t_range_n], xData[t_range_p]);
    }
    ui->label_7->setText("Range[V]");
    ui->qwtPlot->setAxisTitle(0, tr("volt"));
    ui->qwtPlot->setAxisTitle(2, tr("time"));
    ui->qwtPlot->replot();
    ui->volt->setText(QString::number(average, 'g', 9) + "V");
  }
  mutex.unlock();
}

void MainWindow::change_volt_range(double value) {
  if (ui->mode->currentIndex() == 1) {
    ui->qwtPlot->setAxisScale(QwtPlot::yLeft, 0, value);
    ui->qwtPlot->replot();

  } else {
    volt_range_p = center + value;
    volt_range_n = center - value;
    range = value;
    volt_range_p = std::clamp(volt_range_p, -5.0, 5.0);
    volt_range_n = std::clamp(volt_range_n, -5.0, 5.0);

    ui->qwtPlot->setAxisScale(QwtPlot::yLeft, volt_range_n, volt_range_p);
    ui->qwtPlot->replot();
  }
}

void MainWindow::change_time_range(int value) {
  t_range = value;
  t_range_p = std::clamp(t_center + t_range, 0, _plotDataSize - 1);
  t_range_n = std::clamp(t_center - t_range, 0, _plotDataSize - 1);
  double average = 0.0;
  int ave_count = 0;
  for (int i = writepoint; i < _plotDataSize; i++) {
    if (i >= t_range_n && i <= t_range_p) {
      average += yData[i];
      ave_count++;
    }
  }
  average = average / ave_count;
  t_center = std::clamp(t_center, 0, _plotDataSize - 1);
  ui->qwtPlot->setAxisScale(QwtPlot::xBottom, xData[t_range_n], xData[t_range_p]);
  ui->qwtPlot->replot();
  ui->volt->setText(QString::number(average, 'g', 9) + "V");
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
  t_center = std::clamp(_plotDataSize / 2 + value, 0, _plotDataSize);
  t_range_p = std::clamp(t_center + t_range, 0, _plotDataSize - 1);
  t_range_n = std::clamp(t_center - t_range, 0, _plotDataSize - 1);
  double average = 0.0;
  int ave_count = 0;
  for (int i = writepoint; i < _plotDataSize; i++) {
    if (i >= t_range_n && i <= t_range_p) {
      average += yData[i];
      ave_count++;
    }
  }
  average = average / ave_count;
  ui->qwtPlot->setAxisScale(QwtPlot::xBottom, xData[t_range_n], xData[t_range_p]);
  ui->qwtPlot->replot();
  ui->volt->setText(QString::number(average, 'g', 9) + "V");
}

void MainWindow::run_measure() {
  measure_mode = ui->mode->currentIndex();
  if (measure_mode == 1) {
    ui->Rate->setCurrentIndex(0);
    ui->Rate->setEnabled(false);
    ui->center_range->setEnabled(false);
    ui->center->setEnabled(false);
    ui->reset_range->setEnabled(false);

    ui->time_fine->setEnabled(false);
    ui->time_range->setEnabled(false);
    ui->time_center_fine->setEnabled(false);
    ui->time_center->setEnabled(false);
    ui->samplereset->setEnabled(false);
    ui->set_t0->setEnabled(false);
  } else {
    ui->Rate->setEnabled(true);
    ui->range_fine->setEnabled(true);
    ui->volt_range->setEnabled(true);
    ui->center_range->setEnabled(true);
    ui->center->setEnabled(true);
    ui->reset_range->setEnabled(true);

    ui->time_fine->setEnabled(true);
    ui->time_range->setEnabled(true);
    ui->time_center_fine->setEnabled(true);
    ui->time_center->setEnabled(true);
    ui->samplereset->setEnabled(true);
    ui->set_t0->setEnabled(true);
  }
  if (ui->run->text().compare("Run", Qt::CaseSensitive) == 0) {
    MainWindow::config();
    ui->save->setEnabled(false);
    ui->saveall->setEnabled(false);
    ui->config->setEnabled(false);
    ui->Connect->setEnabled(false);
    ui->Rate->setEnabled(false);
    ui->PGA->setEnabled(false);
    ui->in_p->setEnabled(false);
    ui->in_n->setEnabled(false);
    ui->analogbuffer->setEnabled(false);
    ui->sync->setEnabled(false);
    ui->mode->setEnabled(false);
    ui->set_t0->setEnabled(false);
    ui->run->setText("Stop");
    mutex.lock();
    buffer.clear();
    if (measure_mode == 1) {
      ui->qwtPlot->setAxisScale(QwtPlot::yLeft, 0, ui->volt_range->value());
    }
    for (int i = 0; i < _plotDataSize; i++) {
      xData[i] = 0.0;
      xData_buf[i] = 0.0;
      yData[i] = 0.0;
    }
    writepoint = _plotDataSize - 1;
    curve->setSamples(&xData[writepoint], &yData[writepoint], _plotDataSize - writepoint);
    ui->qwtPlot->replot();
    mutex.unlock();
    command.kill = 0;
    command.run = 1;
    this->sock->write((char *)(&command), sizeof(command));
    this->sock->flush();

    timerID = this->startTimer(15);
  } else {
    ui->save->setEnabled(true);
    ui->saveall->setEnabled(true);
    ui->config->setEnabled(true);
    ui->Connect->setEnabled(true);
    ui->Rate->setEnabled(true);
    ui->PGA->setEnabled(true);
    ui->in_p->setEnabled(true);
    ui->in_n->setEnabled(true);
    ui->analogbuffer->setEnabled(true);
    ui->sync->setEnabled(true);
    ui->mode->setEnabled(true);
    ui->set_t0->setEnabled(true);

    command.run = 0;
    this->sock->write((char *)(&command), sizeof(command));
    this->sock->flush();

    this->killTimer(timerID);
    ui->run->setText("Run");
    l_sum = 0;
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
  ui->time_range->setValue(50000);
  ui->time_center->setValue(0);
}

void MainWindow::range_fine_func(int checked) {
  if (checked) {
    ui->volt_range->setSingleStep(0.0001);
    ui->volt_range->setMinimum(0.000001);

  } else {
    ui->volt_range->setSingleStep(0.1);
    ui->volt_range->setMinimum(0.1);
  }
}

void MainWindow::center_fine_func(int checked) {
  if (checked) {
    ui->center->setSingleStep(0.0001);
  } else {
    ui->center->setSingleStep(0.1);
  }
}

void MainWindow::time_fine_func(int checked) {
  if (checked) {
    ui->time_range->setSingleStep(10);
    ui->time_range->setMinimum(100);
  } else {
    ui->time_range->setSingleStep(1000);
    ui->time_range->setMinimum(1000);
  }
}

void MainWindow::time_center_fine_func(int checked) {
  if (checked) {
    ui->time_center->setSingleStep(1);
  } else {
    ui->time_center->setSingleStep(100);
  }
}

void MainWindow::save_as() {
  QString filters("CSV files (*.csv);;All files(*.*)");
  QString defaultFilter("CSV files (*.csv)");
  QString filepath;
  QDateTime date = QDateTime::currentDateTime();
  QFileDialog dialog;
  QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

  filepath = dialog.getSaveFileName(0, tr("名前を付けて保存"), path + "/" + "plot-" + date.toString("yyyy-MM-dd-HH-mm") + ".csv", filters, &defaultFilter);
  if (filepath.size() > 0) {
    QFile file(filepath);
    if (file.open(QFile::ReadWrite)) {
      QTextStream filestream(&file);
      filestream.setCodec("UTF-8");
      filestream.setRealNumberNotation(QTextStream::ScientificNotation);
      if (measure_mode == 0) {
        filestream << "Time[s],Volt[v]" << Qt::endl;
        mutex.lock();
        for (int i = t_range_n; i < t_range_p; i++) {
          filestream << QString::number(xData[i], 'g', 9) << "," << QString::number(yData[i], 'g', 9) << Qt::endl;
        }
        mutex.unlock();
        filestream.flush();
      } else if (measure_mode == 1) {
        filestream << "freq,value" << Qt::endl;
        mutex.lock();
        for (int i = 0; i < N; i++) {
          filestream << QString::number(xData[i], 'g', 9);
          filestream << "," << QString::number(fft_read.F[i].real(), 'g', 9);
          if (fft_read.F[i].imag() >= 0) {
            filestream << "+" << QString::number(fft_read.F[i].imag(), 'g', 9) << "j" << Qt::endl;
          } else {
            filestream << QString::number(fft_read.F[i].imag(), 'g', 9) << "j" << Qt::endl;
          }
        }
        mutex.unlock();
        filestream.flush();
      }

      file.close();
    }
  }
}

void MainWindow::save_all() {
  QString filters("CSV files (*.csv);;All files(*.*)");
  QString defaultFilter("CSV files (*.csv)");
  QString filepath;
  QDateTime date = QDateTime::currentDateTime();
  QFileDialog dialog;
  QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

  filepath = dialog.getSaveFileName(0, tr("名前を付けて保存"), path + "/" + "plot-" + date.toString("yyyy-MM-dd-HH-mm") + ".csv", filters, &defaultFilter);
  if (filepath.size() > 0) {
    QFile file(filepath);
    if (file.open(QFile::ReadWrite)) {
      QTextStream filestream(&file);
      filestream.setCodec("UTF-8");
      filestream.setRealNumberNotation(QTextStream::ScientificNotation);
      if (measure_mode == 0) {
        filestream << "Time[s],Volt[v]" << Qt::endl;
        mutex.lock();
        for (int i = 0; i < _plotDataSize; i++) {
          filestream << QString::number(xData[i], 'g', 9) << "," << QString::number(yData[i], 'g', 9) << Qt::endl;
        }
        mutex.unlock();
        filestream.flush();
      } else if (measure_mode == 1) {
        filestream << "freq,value" << Qt::endl;
        mutex.lock();
        for (int i = 0; i < N; i++) {
          filestream << QString::number(xData[i], 'g', 9);
          filestream << "," << QString::number(fft_read.F[i].real(), 'g', 9);
          if (fft_read.F[i].imag() >= 0) {
            filestream << "+" << QString::number(fft_read.F[i].imag(), 'g', 9) << "j" << Qt::endl;
          } else {
            filestream << QString::number(fft_read.F[i].imag(), 'g', 9) << "j" << Qt::endl;
          }
        }
        mutex.unlock();
        filestream.flush();
      }

      file.close();
    }
  }
}

void MainWindow::connect_socket() {
  if (ui->Connect->text().compare("Connect", Qt::CaseSensitive) == 0) {
    hostname = ui->Host->text();
    Port = ui->Port->text();
    this->sock->connectToHost(hostname, Port.toUShort(), QIODevice::ReadWrite);
    if (sock->waitForConnected(5000)) {
      ui->statusBar->showMessage(hostname + ":" + Port + " is Connected.", 5000);
      ui->Connect->setText("Disconnect");
      ui->config->setEnabled(true);
    } else {
      ui->statusBar->showMessage(hostname + " is not Connected.");
      return;
    }
  } else {
    command.kill = 1;
    this->sock->write((char *)(&command), sizeof(command));
    this->sock->flush();

    this->sock->disconnectFromHost();
    if (sock->state() == QAbstractSocket::UnconnectedState || sock->waitForDisconnected(5000)) {
      ui->Connect->setText("Connect");
      ui->statusBar->showMessage(hostname + ":" + Port + " is Disconnected.", 5000);
      ui->config->setEnabled(false);
      ui->run->setEnabled(false);
      ui->save->setEnabled(false);
      ui->saveall->setEnabled(false);
    }
  }
}

void MainWindow::config() {
  int mode = ui->mode->currentIndex();

  if (mode == 1) {
    ui->Rate->setCurrentIndex(0);
    ui->Rate->setEnabled(false);
    ui->center_range->setEnabled(false);
    ui->center->setEnabled(false);
    ui->reset_range->setEnabled(false);
    ui->sync->setChecked(false);

    ui->time_fine->setEnabled(false);
    ui->time_range->setEnabled(false);
    ui->time_center_fine->setEnabled(false);
    ui->time_center->setEnabled(false);
    ui->samplereset->setEnabled(false);
    ui->set_t0->setEnabled(false);
  } else {
    ui->Rate->setEnabled(true);
    ui->range_fine->setEnabled(true);
    ui->volt_range->setEnabled(true);
    ui->center_range->setEnabled(true);
    ui->center->setEnabled(true);
    ui->reset_range->setEnabled(true);

    ui->time_fine->setEnabled(true);
    ui->time_range->setEnabled(true);
    ui->time_center_fine->setEnabled(true);
    ui->time_center->setEnabled(true);
    ui->samplereset->setEnabled(true);
    ui->set_t0->setEnabled(true);
  }
  int rate = ui->Rate->currentIndex();
  int pga = ui->PGA->currentIndex();
  int pin = ui->in_p->currentIndex();
  int nin = ui->in_n->currentIndex();

  command.rate = rate_list[rate];
  command.gain = gain_list[pga];
  command.positive = ain_list[pin];
  command.negative = ain_list[nin];
  command.mode = mode_list[mode];
  if (ui->analogbuffer->isChecked()) {
    command.buf = 1;
  } else {
    command.buf = 0;
  }

  if (ui->sync->isChecked()) {
    command.sync = 1;
  } else {
    command.sync = 0;
  }

  command.run = 0;
  command.kill = 0;
  ui->run->setEnabled(true);

  this->sock->write((char *)(&command), sizeof(command));
  this->sock->flush();
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (this->sock->state() != QAbstractSocket::UnconnectedState) {
    command.kill = 1;
    this->sock->write((char *)(&command), sizeof(command));
    this->sock->flush();
    this->sock->close();
  }
  event->accept();
}

void MainWindow::read_task() {
  if (this->sock->bytesAvailable() > 0) {
    this->mutex.lock();
    buffer.append(this->sock->readAll());
    this->mutex.unlock();
  }
}

void MainWindow::change_t0() {
  t_0 = xData_buf[t_center];
  for (int i = writepoint; i < _plotDataSize; i++) {
    xData[i] = (xData_buf[i] - t_0) / 1000000.0;
  }
  curve->setSamples(&xData[writepoint], &yData[writepoint], _plotDataSize - writepoint);
  if (writepoint > t_range_n) {
    ui->qwtPlot->setAxisScale(QwtPlot::xBottom, xData[writepoint], xData[t_range_p]);
  } else {
    ui->qwtPlot->setAxisScale(QwtPlot::xBottom, xData[t_range_n], xData[t_range_p]);
  }
  ui->qwtPlot->replot();
}