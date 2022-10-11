#include "worker.h"

Worker::Worker(QMutex* mutex, int sock, int len, double* xData, double* yData) {
  this->mutex = mutex;
  this->sock = sock;
  this->len = len;
  this->xData = xData;
  this->yData = yData;
}

void Worker::run() {
  struct read_data buf[20000];
  int64_t xData_buf[len] = {0};
  int64_t t_0;
  int l;
  while (!this->isInterruptionRequested()) {
    read(sock, &l, sizeof(l));
    read(sock, buf, sizeof(struct read_data) * (l));
    mutex->lock();
    for (int i = l; i < len; i++) {
      yData[i - l] = yData[i];
      xData_buf[i - l] = xData_buf[i];
    }

    for (int i = 0; i < l; i++) {
      yData[len - l + i] = buf[i].volt;
      xData_buf[len - l + i] = buf[i].t;
    }
    t_0 = xData_buf[len / 2];
    for (int i = 0; i < len; i++) {
      xData[i] = (double)(xData_buf[i] - t_0) / 1000000;
    }
    mutex->unlock();
  }
}