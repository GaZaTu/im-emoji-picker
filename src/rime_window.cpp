#include "rime_window.hpp"
#include <QCoreApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QStatusBar>

ThreadsafeQueue<std::shared_ptr<RimeCommand>> rime_command_queue;

// copied from ibustypes.h
typedef enum {
  IBUS_SHIFT_MASK = 1 << 0,
  IBUS_LOCK_MASK = 1 << 1,
  IBUS_CONTROL_MASK = 1 << 2,
  IBUS_MOD1_MASK = 1 << 3,
  IBUS_MOD2_MASK = 1 << 4,
  IBUS_MOD3_MASK = 1 << 5,
  IBUS_MOD4_MASK = 1 << 6,
  IBUS_MOD5_MASK = 1 << 7,
  IBUS_BUTTON1_MASK = 1 << 8,
  IBUS_BUTTON2_MASK = 1 << 9,
  IBUS_BUTTON3_MASK = 1 << 10,
  IBUS_BUTTON4_MASK = 1 << 11,
  IBUS_BUTTON5_MASK = 1 << 12,

  /* The next few modifiers are used by XKB, so we skip to the end.
   * Bits 15 - 23 are currently unused. Bit 29 is used internally.
   */

  /* ibus mask */
  IBUS_HANDLED_MASK = 1 << 24,
  IBUS_FORWARD_MASK = 1 << 25,
  IBUS_IGNORED_MASK = IBUS_FORWARD_MASK,

  IBUS_SUPER_MASK = 1 << 26,
  IBUS_HYPER_MASK = 1 << 27,
  IBUS_META_MASK = 1 << 28,

  IBUS_RELEASE_MASK = 1 << 30,

  IBUS_MODIFIER_MASK = 0x5f001fff
} IBusModifierType;

RimeWindow::RimeWindow() : QMainWindow() {
  setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  // setWindowIcon(QIcon(":/res/x11-emoji-picker.png"));
  setWindowOpacity(0.95);

  resize(360, 200);

  central_widget->setLayout(central_layout);
  central_layout->setContentsMargins(0, 0, 0, 0);
  central_layout->addWidget(search_edit);
  central_layout->addWidget(emoji_list);

  setCentralWidget(central_widget);

  if (true) {
    setStatusBar(new QStatusBar(this));
    statusBar()->showMessage("TEST");
  }
}

void RimeWindow::reset() {
  search_edit->setText("");
  search_edit->repaint();

  hide();
}

void RimeWindow::set_cursor_location(int x, int y, int w, int h) {
  if (x == 0 && y == 0) {
    return;
  }

  move(x, y + h);
}

void RimeWindow::process_key_event(uint keyval, uint keycode, uint modifiers) {
  if (modifiers & IBUS_RELEASE_MASK) {
    return;
  }

  if (keycode == KEYCODE_ESCAPE) {
    reset();
    return;
  }

  if (keycode == KEYCODE_RETURN) {
    commit_text(u8"👌");
    return;
  }

  // QKeyEvent::Type event_type = (modifiers & IBUS_RELEASE_MASK) ? QKeyEvent::KeyRelease : QKeyEvent::KeyPress;
  // int event_key = keyval;
  // Qt::KeyboardModifiers event_modifiers = Qt::NoModifier;
  // QString event_text = QString((char)keyval);

  // QKeyEvent* event = new QKeyEvent(event_type, event_key, event_modifiers, event_text);
  // QCoreApplication::sendEvent(this, event);

  if (keycode == KEYCODE_BACKSPACE) {
    if (!search_edit->text().isEmpty()) {
      search_edit->setText(search_edit->text().chopped(1));
      search_edit->repaint();
    }

    return;
  }

  search_edit->setText(search_edit->text() + (char)keyval);
  search_edit->repaint();

  show();
}
