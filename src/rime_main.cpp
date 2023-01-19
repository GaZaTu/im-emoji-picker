#include <ibus.h>
#include <memory>
#include <qnamespace.h>
#include <qtimer.h>
#include <signal.h>

#include "rime_engine.hpp"
#include "rime_main.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <QProcess>
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

  IBusEngineDesc* original_engine_info = ibus_bus_get_global_engine(bus);
  g_object_ref_sink(factory);

  const char* original_engine_name = ibus_engine_desc_get_name(original_engine_info);
  reset_ibus_engine_to_original = [bus, original_engine_name{std::string{original_engine_name}}]() {
    ibus_bus_set_global_engine(bus, original_engine_name.data());
  };

  ibus_factory_add_engine(factory, "rime", IBUS_TYPE_RIME_ENGINE);
  if (!ibus_bus_request_name(bus, "im.rime.Rime", 0)) {
    log_printf("[error] could not requst bus name");
    exit(EXIT_FAILURE);
  }

  log_printf("[debug] start ibus_main\n");
  ibus_main();
  log_printf("[debug] end ibus_main\n");

  g_object_unref(original_engine_info);
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

  RimeWindow rime_window;

  QTimer rime_command_processor;
  QObject::connect(&rime_command_processor, &QTimer::timeout, [&rime_window]() {
    std::shared_ptr<RimeCommand> _command;
    if (rime_command_queue.pop(_command)) {
      if (auto command = std::dynamic_pointer_cast<RimeCommandInit>(_command)) {
        rime_window.commit_text = command->commit_text;
      }
      if (auto command = std::dynamic_pointer_cast<RimeCommandReset>(_command)) {
        rime_window.reset();
      }
      if (auto command = std::dynamic_pointer_cast<RimeCommandEnable>(_command)) {
        rime_window.enable();
      }
      if (auto command = std::dynamic_pointer_cast<RimeCommandDisable>(_command)) {
        rime_window.disable();
      }
      if (auto command = std::dynamic_pointer_cast<RimeCommandSetCursorLocation>(_command)) {
        rime_window.set_cursor_location(command->x, command->y, command->w, command->h);
      }
      if (auto command = std::dynamic_pointer_cast<RimeCommandProcessKeyEvent>(_command)) {
        rime_window.process_key_event(command->keyval, command->keycode, command->modifiers);
      }
    }
  });
  rime_command_processor.start(5);

  if (argc >= 2 && std::string{argv[1]} == "test") {
    rime_window.show();
  }

  log_printf("[debug] start gui_main\n");
  app.exec();
  log_printf("[debug] end gui_main\n");
}

int main(int argc, char** argv) {
  log_file = fopen("/tmp/ibus-test.log", "a");
  fprintf(log_file, "--------------------------------------------------\n");

  signal(SIGTERM, sigterm_cb);
  signal(SIGINT, sigterm_cb);

  if (argc >= 2 && std::string{argv[1]} == "test") {
    gui_main(argc, argv);
  } else {
    std::thread gui_thread{&gui_main, argc, argv};

    rime_with_ibus();
  }

  fclose(log_file);

  return 0;
}
}
