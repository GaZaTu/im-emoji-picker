#pragma once

#include <QThread>
#include <functional>
#include <stdio.h>

extern "C" {
extern FILE* log_file;

void log_printf(const char* format, ...);
}

void runInGUIThread(std::function<void()> callback);

class WorkerThread : public QThread {
  Q_OBJECT

public:
  std::function<void()> callback;

protected:
  void run() override;
};
