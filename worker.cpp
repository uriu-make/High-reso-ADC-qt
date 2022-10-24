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
  int l_sum = 0;
  setTerminationEnabled(false);
  while (!this->isInterruptionRequested()) {
    struct read_data buf = {0};
    while (!*stopped) {
      recv(sock, &buf, sizeof(read_data), MSG_WAITALL);
      if (buf.len < 0) {
        break;
      }
      l_sum = std::clamp(l_sum + buf.len, 0, len - 1);
      mutex->lock();
      *writepoint = len - l_sum;
      memcpy(yData, &yData[buf.len], sizeof(double) * (len - buf.len));
      memcpy(xData_buf, &xData_buf[buf.len], sizeof(int64_t) * (len - buf.len));
      memcpy(&yData[len - buf.len], buf.volt, sizeof(double) * buf.len);
      memcpy(&xData_buf[len - buf.len], buf.t, sizeof(int64_t) * buf.len);
      mutex->unlock();
    }
    l_sum = 0;
  }
  setTerminationEnabled(true);
}