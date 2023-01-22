#if __has_include("ibus.h")

#include "IBusEmojiEngine.hpp"
#include "logging.hpp"
#include "EmojiWindow.hpp"
#include <signal.h>
#include <thread>

extern "C" {
static void sigterm_cb(int sig) {
  exit(EXIT_FAILURE);
}

static void ibus_disconnect_cb(IBusBus* bus, gpointer user_data) {
  log_printf("[debug] ibus_disconnect_cb\n");

  ibus_quit();
}

int main(int argc, char** argv) {
  signal(SIGTERM, sigterm_cb);
  signal(SIGINT, sigterm_cb);

  ibus_init();

  IBusBus* bus = ibus_bus_new();
  g_object_ref_sink(bus);

  if (!ibus_bus_is_connected(bus)) {
    log_printf("[error] not connected to ibus\n");
    exit(EXIT_FAILURE);
  }

  g_signal_connect(bus, "disconnected", G_CALLBACK(ibus_disconnect_cb), nullptr);

  IBusFactory* factory = ibus_factory_new(ibus_bus_get_connection(bus));
  g_object_ref_sink(factory);

  IBusEngineDesc* original_engine_info = ibus_bus_get_global_engine(bus);
  g_object_ref_sink(factory);

  const char* original_engine_name = ibus_engine_desc_get_name(original_engine_info);
  resetInputMethodEngine = [bus, original_engine_name{std::string{original_engine_name}}]() {
    ibus_bus_set_global_engine(bus, original_engine_name.data());
  };

  ibus_factory_add_engine(factory, "dank-emoji-picker", IBUS_TYPE_EMOJI_ENGINE);
  if (!ibus_bus_request_name(bus, "xyz.gazatu.DankEmojiPicker", 0)) {
    log_printf("[error] could not requst bus name");
    exit(EXIT_FAILURE);
  }

  gui_main(argc, argv);

  g_object_unref(original_engine_info);
  g_object_unref(factory);
  g_object_unref(bus);

  return 0;
}
}

#endif
