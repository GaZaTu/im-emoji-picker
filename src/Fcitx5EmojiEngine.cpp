#if __has_include("fcitx/inputmethodengine.h")

#include "Fcitx5EmojiEngine.hpp"
#include "EmojiWindow.hpp"
#include "fcitx/inputmethodentry.h"
#include "logging.hpp"
#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/keysymgen.h>
#include <fcitx/event.h>
#include <fcitx/inputcontext.h>
#include <thread>

#define KEYCODE_ESCAPE 9
#define KEYCODE_RETURN 36
#define KEYCODE_BACKSPACE 22
#define KEYCODE_ARROW_UP 111
#define KEYCODE_ARROW_DOWN 116
#define KEYCODE_ARROW_LEFT 113
#define KEYCODE_ARROW_RIGHT 114

void Fcitx5EmojiEngine::keyEvent(const fcitx::InputMethodEntry& entry, fcitx::KeyEvent& keyEvent) {
  log_printf("[debug] Fcitx5EmojiEngine::keyEvent code: %d key:%s isRelease:%d\n", keyEvent.key().code(), keyEvent.key().toString().data(), keyEvent.isRelease());

  QKeyEvent::Type _type = keyEvent.isRelease() ? QKeyEvent::KeyRelease : QKeyEvent::KeyPress;
  int _key = 0;
  switch (keyEvent.key().code()) {
  case KEYCODE_ESCAPE: // FcitxKey_Escape:
    _key = Qt::Key_Escape;
    break;
  case KEYCODE_RETURN: // FcitxKey_Return:
    _key = Qt::Key_Return;
    break;
  case KEYCODE_BACKSPACE: // FcitxKey_BackSpace:
    _key = Qt::Key_Backspace;
    break;
  case KEYCODE_ARROW_UP: // FcitxKey_uparrow:
    _key = Qt::Key_Up;
    break;
  case KEYCODE_ARROW_DOWN: // FcitxKey_downarrow:
    _key = Qt::Key_Down;
    break;
  case KEYCODE_ARROW_LEFT: // FcitxKey_leftarrow:
    _key = Qt::Key_Left;
    break;
  case KEYCODE_ARROW_RIGHT: // FcitxKey_rightarrow:
    _key = Qt::Key_Right;
    break;
  }
  Qt::KeyboardModifiers _modifiers = Qt::NoModifier;
  if (keyEvent.key().states() & fcitx::KeyState::Super) {
    return;
  }
  if (keyEvent.key().states() & fcitx::KeyState::Ctrl) {
    _modifiers = _modifiers | Qt::ControlModifier;
  }
  if (keyEvent.key().states() & fcitx::KeyState::Shift) {
    _modifiers = _modifiers | Qt::ShiftModifier;
  }
  QString _text = QString::fromStdString(keyEvent.key().toString());

  if (_key != 0 || isascii(_text.at(0).toLatin1())) {
    emoji_command_queue.push(std::make_shared<EmojiCommandProcessKeyEvent>(new QKeyEvent(_type, _key, _modifiers, _text)));

    keyEvent.filterAndAccept();
  }
}

void Fcitx5EmojiEngine::activate(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) {
  log_printf("[debug] Fcitx5EmojiEngine::activate\n");

  emoji_command_queue.push(std::make_shared<EmojiCommandEnable>([inputContext{event.inputContext()}](const std::string& text) {
    inputContext->commitString(text);
  }));
}

void Fcitx5EmojiEngine::deactivate(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) {
  log_printf("[debug] Fcitx5EmojiEngine::deactivate\n");

  emoji_command_queue.push(std::make_shared<EmojiCommandDisable>());
}

void Fcitx5EmojiEngine::reset(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) {
  log_printf("[debug] Fcitx5EmojiEngine::reset\n");

  emoji_command_queue.push(std::make_shared<EmojiCommandReset>());
}

fcitx::AddonInstance* Fcitx5EmojiEngineFactory::create(fcitx::AddonManager* manager) {
  log_printf("[debug] Fcitx5EmojiEngineFactory::create\n");

  static bool started_gui_thread = false;
  if (!started_gui_thread) {
    std::thread{gui_main, 0, nullptr}.detach();
  }

  return new Fcitx5EmojiEngine();
}

FCITX_ADDON_FACTORY(Fcitx5EmojiEngineFactory);

#endif
