#if __has_include("fcitx/addoninstance.h")

#include "Fcitx5ImEmojiPickerModule.hpp"
#include "EmojiPickerWindow.hpp"
#include "logging.hpp"
#include <QKeyEvent>
#include <QProcess>
#include <fcitx-config/configuration.h>
#include <fcitx-config/enum.h>
#include <fcitx-config/iniparser.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/keysymgen.h>
#include <fcitx/event.h>
#include <fcitx/inputcontext.h>
#include <thread>

#define KEYCODE_ESCAPE 9
#define KEYCODE_RETURN 36
#define KEYCODE_BACKSPACE 22
#define KEYCODE_TAB 23
#define KEYCODE_ARROW_UP 111
#define KEYCODE_ARROW_DOWN 116
#define KEYCODE_ARROW_LEFT 113
#define KEYCODE_ARROW_RIGHT 114
#define KEYCODE_PAGE_UP 112
#define KEYCODE_PAGE_DOWN 117
#define KEYCODE_F4 70

#define KEYCODE_SPACE 65
#define KEYCODE_UNDERSCORE 61

Fcitx5ImEmojiPickerModule::Fcitx5ImEmojiPickerModule(fcitx::Instance* instance) : _instance(instance) {
  resetInputMethodEngine = [this]() {
    fcitx::InputContextEvent dummy{nullptr, (fcitx::EventType)0};
    deactivate(dummy);
  };

  _eventHandlers.emplace_back(_instance->watchEvent(fcitx::EventType::InputContextKeyEvent, fcitx::EventWatcherPhase::Default, [this](fcitx::Event& _event) {
    auto& event = static_cast<fcitx::KeyEvent&>(_event);

    if (event.isRelease()) {
      return;
    }

    if (!event.key().checkKeyList(*_config.triggerKey)) {
      return;
    }

    event.filterAndAccept();
    activate(event);
  }));

  _eventHandlers.emplace_back(_instance->watchEvent(fcitx::EventType::InputContextFocusOut, fcitx::EventWatcherPhase::Default, [this](fcitx::Event& _event) {
    auto& event = static_cast<fcitx::InputContextEvent&>(_event);

    log_printf("[debug] Fcitx5ImEmojiPickerModule::_eventHandlers[InputContextFocusOut]\n");

    deactivate(event);
  }));

  _eventHandlers.emplace_back(_instance->watchEvent(fcitx::EventType::InputContextReset, fcitx::EventWatcherPhase::Default, [this](fcitx::Event& _event) {
    auto& event = static_cast<fcitx::InputContextEvent&>(_event);

    log_printf("[debug] Fcitx5ImEmojiPickerModule::_eventHandlers[InputContextReset]\n");

    deactivate(event);
  }));

  _eventHandlers.emplace_back(_instance->watchEvent(fcitx::EventType::InputContextSwitchInputMethod, fcitx::EventWatcherPhase::Default, [this](fcitx::Event& _event) {
    auto& event = static_cast<fcitx::InputContextEvent&>(_event);

    log_printf("[debug] Fcitx5ImEmojiPickerModule::_eventHandlers[InputContextSwitchInputMethod]\n");

    deactivate(event);
  }));

  _eventHandlers.emplace_back(_instance->watchEvent(fcitx::EventType::InputContextKeyEvent, fcitx::EventWatcherPhase::PreInputMethod, [this](fcitx::Event& _event) {
    if (!_active) {
      return;
    }

    auto& event = static_cast<fcitx::KeyEvent&>(_event);

    event.filter();
    keyEvent(event);
  }));

  reloadConfig();
}

void Fcitx5ImEmojiPickerModule::keyEvent(fcitx::KeyEvent& keyEvent) {
  log_printf("[debug] Fcitx5ImEmojiPickerModule::keyEvent key:%s code:%d isRelease:%d\n", keyEvent.key().toString().data(), keyEvent.key().code(), keyEvent.isRelease());

  if (keyEvent.key().isModifier()) {
    return;
  }

  QString _text = QString::fromStdString(keyEvent.key().toString());
  _text = _text.right(1);
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
  case KEYCODE_TAB: // FcitxKey_Tab:
    _key = Qt::Key_Tab;
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
  case KEYCODE_PAGE_UP: // FcitxKey_pageup:
    _key = Qt::Key_PageUp;
    break;
  case KEYCODE_PAGE_DOWN: // FcitxKey_pagedown:
    _key = Qt::Key_PageDown;
    break;
  case KEYCODE_F4: // FcitxKey_f4:
    _key = Qt::Key_F4;
    break;
  default:
    _key = QKeySequence{_text, QKeySequence::PortableText}[0];
    break;
  }
  Qt::KeyboardModifiers _modifiers = Qt::NoModifier;
  if (keyEvent.key().states() & fcitx::KeyState::Super) {
    _modifiers |= Qt::MetaModifier;
  }
  if (keyEvent.key().states() & fcitx::KeyState::Ctrl) {
    _modifiers |= Qt::ControlModifier;
  }
  if (keyEvent.key().states() & fcitx::KeyState::Shift) {
    _modifiers |= Qt::ShiftModifier;
  }

  switch (keyEvent.key().code()) {
  case KEYCODE_SPACE: // FcitxKey_Space:
    _text = " ";
    break;
  case KEYCODE_UNDERSCORE: // FcitxKey_Underscore:
    _text = "_";
    break;
  }

  QKeyEvent* qevent = createKeyEventWithUserPreferences(_type, _key, _modifiers, _text);
  EmojiAction action = getEmojiActionForQKeyEvent(qevent);

  if (action != EmojiAction::INVALID) {
    emojiCommandQueue.push(std::make_shared<EmojiCommandProcessKeyEvent>(qevent, action));

    keyEvent.accept();
  } else {
    delete qevent;
  }

  if (_key == Qt::Key_Return && _type == QKeyEvent::KeyRelease) {
    sendCursorLocation(keyEvent);
  }
}

void Fcitx5ImEmojiPickerModule::activate(fcitx::InputContextEvent& event) {
  if (_active) {
    return;
  }

  log_printf("[debug] Fcitx5ImEmojiPickerModule::activate\n");

  _active = true;

  gui_set_active(true);

  sendCursorLocation(event);

  emojiCommandQueue.push(std::make_shared<EmojiCommandEnable>([this, inputContext{event.inputContext()}](const std::string& text) {
    log_printf("[debug] Fcitx5ImEmojiPickerModule::(lambda) commitString:%s program:%s\n", text.data(), inputContext->program().data());

    inputContext->commitString(text);
  }, false));
}

void Fcitx5ImEmojiPickerModule::deactivate(fcitx::InputContextEvent& event) {
  if (!_active) {
    return;
  }

  log_printf("[debug] Fcitx5ImEmojiPickerModule::deactivate\n");

  _active = false;

  emojiCommandQueue.push(std::make_shared<EmojiCommandDisable>());

  gui_set_active(false);
}

void Fcitx5ImEmojiPickerModule::reset(fcitx::InputContextEvent& event) {
  log_printf("[debug] Fcitx5ImEmojiPickerModule::reset\n");

  emojiCommandQueue.push(std::make_shared<EmojiCommandReset>());
}

void Fcitx5ImEmojiPickerModule::sendCursorLocation(fcitx::InputContextEvent& event) {
  const fcitx::Rect& r = event.inputContext()->cursorRect();

  log_printf("[debug] Fcitx5ImEmojiPickerModule::sendCursorLocation x:%d y:%d w:%d h:%d\n", r.left(), r.top(), r.width(), r.height());

  emojiCommandQueue.push(std::make_shared<EmojiCommandSetCursorLocation>(new QRect(r.left(), r.top(), r.width(), r.height())));
}

void Fcitx5ImEmojiPickerModule::reloadConfig() {
  fcitx::readAsIni(_config, CONFIG_FILE);
}

const fcitx::Configuration* Fcitx5ImEmojiPickerModule::getConfig() const {
  return &_config;
}

void Fcitx5ImEmojiPickerModule::setConfig(const fcitx::RawConfig& config) {
  _config.load(config, true);
  safeSaveAsIni(_config, CONFIG_FILE);
}

fcitx::AddonInstance* Fcitx5ImEmojiPickerModuleFactory::create(fcitx::AddonManager* manager) {
  log_printf("[debug] Fcitx5ImEmojiPickerModuleFactory::create\n");

  static bool gui_main_started = false;
  if (!gui_main_started) {
    gui_main_started = true;
    std::thread{gui_main, 0, nullptr}.detach();
  }

  return new Fcitx5ImEmojiPickerModule(manager->instance());
}

FCITX_ADDON_FACTORY(Fcitx5ImEmojiPickerModuleFactory);

#endif
