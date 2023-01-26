#if __has_include("fcitx/inputmethodengine.h")

#include "Fcitx5ImEmojiPickerEngine.hpp"
#include "EmojiPickerWindow.hpp"
#include "fcitx/inputmethodentry.h"
#include "logging.hpp"
#include <QKeyEvent>
#include <QProcess>
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

void Fcitx5ImEmojiPickerEngine::keyEvent(const fcitx::InputMethodEntry& entry, fcitx::KeyEvent& keyEvent) {
  log_printf("[debug] Fcitx5ImEmojiPickerEngine::keyEvent key:%s code:%d isRelease:%d\n", keyEvent.key().toString().data(), keyEvent.key().code(), keyEvent.isRelease());

  if (keyEvent.key().isModifier()) {
    return;
  }

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
  QString _text = QString::fromStdString(keyEvent.key().toString());
  _text = _text.right(1);

  switch (keyEvent.key().code()) {
  case KEYCODE_SPACE: // FcitxKey_Space:
    _text = " ";
    break;
  case KEYCODE_UNDERSCORE: // FcitxKey_Underscore:
    _text = "_";
    break;
  }

  QKeyEvent* qevent = new QKeyEvent(_type, _key, _modifiers, _text);
  EmojiAction action = getEmojiActionForQKeyEvent(qevent);

  if (action != EmojiAction::INVALID) {
    emojiCommandQueue.push(std::make_shared<EmojiCommandProcessKeyEvent>(qevent));

    keyEvent.filterAndAccept();
  } else {
    delete qevent;
  }

  if (_key == Qt::Key_Return && _type == QKeyEvent::KeyRelease) {
    sendCursorLocation(keyEvent.inputContext());
  }
}

void Fcitx5ImEmojiPickerEngine::activate(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) {
  log_printf("[debug] Fcitx5ImEmojiPickerEngine::activate\n");

  emojiCommandQueue.push(std::make_shared<EmojiCommandEnable>([this, inputContext{event.inputContext()}](const std::string& text) {
    inputContext->commitString(text);

    // usleep(10000);
    // sendCursorLocation(inputContext);
  }));

  sendCursorLocation(event.inputContext());
}

void Fcitx5ImEmojiPickerEngine::deactivate(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) {
  log_printf("[debug] Fcitx5ImEmojiPickerEngine::deactivate\n");

  emojiCommandQueue.push(std::make_shared<EmojiCommandDisable>());
}

void Fcitx5ImEmojiPickerEngine::reset(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) {
  log_printf("[debug] Fcitx5ImEmojiPickerEngine::reset\n");

  emojiCommandQueue.push(std::make_shared<EmojiCommandReset>());
}

void Fcitx5ImEmojiPickerEngine::sendCursorLocation(fcitx::InputContext* inputContext) {
  const fcitx::Rect& r = inputContext->cursorRect();

  log_printf("[debug] Fcitx5ImEmojiPickerEngine::sendCursorLocation x:%d y:%d w:%d h:%d\n", r.left(), r.top(), r.width(), r.height());

  emojiCommandQueue.push(std::make_shared<EmojiCommandSetCursorLocation>(new QRect(r.left(), r.top(), r.width(), r.height())));
}

fcitx::AddonInstance* Fcitx5ImEmojiPickerEngineFactory::create(fcitx::AddonManager* manager) {
  log_printf("[debug] Fcitx5ImEmojiPickerEngineFactory::create\n");

  std::thread{[]() {
    QProcess fcitx5remote;
    fcitx5remote.start("fcitx5-remote", {"-n"});
    fcitx5remote.waitForFinished();

    QString defaultInputMethod{fcitx5remote.readAllStandardOutput()};
    resetInputMethodEngine = [defaultInputMethod{defaultInputMethod.trimmed()}]() {
      QProcess fcitx5remote;
      fcitx5remote.start("fcitx5-remote", {"-s", defaultInputMethod});
      fcitx5remote.waitForFinished();
    };
  }}.detach();

  static bool startedGUIThread = false;
  if (!startedGUIThread) {
    std::thread{gui_main, 0, nullptr}.detach();
  }

  return new Fcitx5ImEmojiPickerEngine();
}

FCITX_ADDON_FACTORY(Fcitx5ImEmojiPickerEngineFactory);

#endif
