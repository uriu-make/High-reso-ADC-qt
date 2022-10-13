#include "worker.h"

Worker::Worker(QMutex* mutex, bool* stopped, int sock, int len, double* xData, double* yData, int* writepoint) {
  this->stopped = stopped;
  this->mutex = mutex;
  this->sock = sock;
  this->len = len;
  this->xData = xData;
  this->yData = yData;
  this->writepoint = writepoint;
}

void Worker::run() {
  struct read_data buf[20000];
  int64_t xData_buf[len] = {0};
  int64_t t_0;
  int l = 0;
  int l_sum = 0;
  setTerminationEnabled(false);
  while (!this->isInterruptionRequested()) {
    while (!*stopped) {
      read(sock, &l, sizeof(int));
      read(sock, buf, sizeof(struct read_data) * (l));
      l_sum = std::clamp(l_sum + l, 0, len);
      mutex->lock();
      *writepoint = len - l_sum;
      // for (int i = l; i < len; i++) {
      //   yData[i - l] = yData[i];
      //   xData_buf[i - l] = xData_buf[i];
      // }
      memcpy(yData, &yData[l], sizeof(double) * (len - l));

      memcpy(xData_buf, &xData_buf[l], sizeof(double) * (len - l));

      for (int i = 0; i < l; i++) {
        yData[len - l + i] = buf[i].volt;
        xData_buf[len - l + i] = buf[i].t;
      }
      t_0 = xData_buf[len / 2];
      for (int i = 0; i < len; i++) {
        xData[i] = (double)(xData_buf[i] - t_0) / 1000000;
      }
      mutex->unlock();
      usleep(500);
    }
    l_sum = 0;
  }
  setTerminationEnabled(true);
}