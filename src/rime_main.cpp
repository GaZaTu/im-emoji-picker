#include <ibus.h>
#include <signal.h>

#include "rime_engine.hpp"
#include "rime_main.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <thread>

extern "C" {
FILE* log_file = NULL;

void log_printf(const char* format, ...) {
  fprintf(log_file, "(%ld) ", time(NULL));

  va_list args;
  va_start(args, format);
  vfprintf(log_file, format, args);
  va_end(args);

  fflush(log_file);
}

static void ibus_disconnect_cb(IBusBus* bus, gpointer user_data) {
  log_printf("[debug] bus disconnected\n");

  ibus_quit();
}

static void rime_with_ibus() {
  ibus_init();

  IBusBus* bus = ibus_bus_new();
  g_object_ref_sink(bus);

  if (!ibus_bus_is_connected(bus)) {
    log_printf("[error] not connected to ibus\n");
    exit(EXIT_FAILURE);
  }

  g_signal_connect(bus, "disconnected", G_CALLBACK(ibus_disconnect_cb), NULL);

  IBusFactory* factory = ibus_factory_new(ibus_bus_get_connection(bus));
  g_object_ref_sink(factory);

  ibus_factory_add_engine(factory, "rime", IBUS_TYPE_RIME_ENGINE);
  if (!ibus_bus_request_name(bus, "im.rime.Rime", 0)) {
    log_printf("[error] could not requst bus name");
    exit(EXIT_FAILURE);
  }

  log_printf("[debug] start ibus_main\n");
  ibus_main();
  log_printf("[debug] end ibus_main\n");

  g_object_unref(factory);
  g_object_unref(bus);
}

static void sigterm_cb(int sig) {
  exit(EXIT_FAILURE);
}

void gui_main(int argc, char** argv) {
  QApplication::setOrganizationName(PROJECT_ORGANIZATION);
  QApplication::setOrganizationDomain(PROJECT_ORGANIZATION);
  QApplication::setApplicationName(PROJECT_NAME);
  QApplication::setApplicationVersion(PROJECT_VERSION);

  QApplication app{argc, argv};

  log_printf("[debug] start gui_main\n");
  app.exec();
  log_printf("[debug] end gui_main\n");
}

int main(int argc, char** argv) {
  log_file = fopen("/tmp/ibus-test.log", "a");
  fprintf(log_file, "--------------------------------------------------\n");

  signal(SIGTERM, sigterm_cb);
  signal(SIGINT, sigterm_cb);

  std::thread gui_thread{&gui_main, argc, argv};

  rime_with_ibus();

  fclose(log_file);

  return 0;
}
}

void runInGUIThread(std::function<void()> callback) {
  WorkerThread* thread = new WorkerThread();
  thread->callback = std::move(callback);

  thread->start();
}

void WorkerThread::run() {
  QTimer* timer = new QTimer();
  log_printf("runinguithread %ld -> %ld\n", timer->thread(), qApp->thread());

  timer->moveToThread(qApp->thread());
  timer->setSingleShot(true);

  QObject::connect(timer, &QTimer::timeout, [timer, callback{std::move(callback)}]() {
    log_printf("deleteLater 1\n");
    timer->deleteLater();
    log_printf("deleteLater 2\n");

    callback();
  });

  QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection, Q_ARG(int, 0));

  deleteLater();
}
