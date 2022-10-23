#include "worker.h"

Worker::Worker(QMutex* mutex, bool* stopped, int sock, int len, int64_t* xData_buf, double* yData, int* writepoint) {
  this->stopped = stopped;
  this->mutex = mutex;
  this->sock = sock;
  this->len = len;
  this->xData_buf = xData_buf;
  this->yData = yData;
  this->writepoint = writepoint;
}

void Worker::run() {
  // struct read_data buf[20000];
  int l = 0;
  int l_sum = 0;
  setTerminationEnabled(false);
  while (!this->isInterruptionRequested()) {
    double volt[len] = {0.0};
    int64_t t[len] = {0};
    while (!*stopped) {
      read(sock, &l, sizeof(int));
      read(sock, volt, sizeof(double) * l);
      read(sock, t, sizeof(int64_t) * l);

      // read(sock, buf, sizeof(struct read_data) * (l));
      l_sum = std::clamp(l_sum + l, 0, len);
      mutex->lock();
      *writepoint = len - l_sum;

      memcpy(yData, &yData[l], sizeof(double) * (len - l));
      memcpy(xData_buf, &xData_buf[l], sizeof(int64_t) * (len - l));
      memcpy(&yData[len - l], volt, sizeof(double) * l);
      memcpy(&xData_buf[len - l], t, sizeof(int64_t) * l);
      mutex->unlock();
    }
    l_sum = 0;
  }
  setTerminationEnabled(true);
}