#include <QThread>
#include <QMutex>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "ADS1256.h"

#pragma once

class Worker : public QThread {
  void run() override;

 private:
  bool* stopped;
  int sock;
  int len;
  int* writepoint;
  uint64_t* xData_buf;
  double* yData;
  QMutex* mutex;

 public:
  Worker(QMutex* mutex, bool* stopped, int sock, int len, uint64_t* xData_buf, double* yData, int* writepoint);
};