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
  int sock;
  int len;
  double* xData;
  double* yData;
  QMutex* mutex;

 public:
  Worker(QMutex* mutex, int sock, int len, double* xData, double* yData);
};