#include "EmojiWindow.hpp"
#include <QCoreApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QStatusBar>
#include <QTimer>
#include <QApplication>
#include <exception>
#include <qnamespace.h>
#include <vector>
#include "EmojiLabel.hpp"
#include "emojis.hpp"
#include "logging.hpp"

std::function<void()> reset_input_method_engine = []() {};

ThreadsafeQueue<std::shared_ptr<EmojiCommand>> emoji_command_queue;

static const int COLS = 10;

EmojiWindow::EmojiWindow() : QMainWindow() {
  setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  // setWindowIcon(QIcon(":/res/x11-emoji-picker.png"));
  setWindowOpacity(0.95);
  setFocusPolicy(Qt::NoFocus);
  setAttribute(Qt::WA_ShowWithoutActivating);

  resize(360, 200);

  emoji_list_widget->setLayout(emoji_list_layout);
  emoji_list_layout->setContentsMargins(4, 4, 0, 4);

  int row = 0;
  int col = 0;
  for (const auto& emoji : emojis) {
    auto emoji_layout_widget = new EmojiLabel(emoji_list_widget, emoji);
    auto emoji_layout_item = new QWidgetItemV2(emoji_layout_widget);
    emoji_list_layout->addItem(emoji_layout_item, row, col);

    col += 1;
    if (col >= COLS) {
      col = 0;
      row += 1;
    }
  }

  selectedEmojiLabel()->setHighlighted(true);
  // selectedEmojiLabel()->repaint(true);

  emoji_list_scroll->setWidgetResizable(true);
  emoji_list_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  emoji_list_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  emoji_list_scroll->setWidget(emoji_list_widget);

  central_widget->setLayout(central_layout);
  central_layout->setContentsMargins(0, 0, 0, 0);
  central_layout->setSpacing(0);
  central_layout->addWidget(search_edit);
  central_layout->addWidget(emoji_list_scroll);

  setCentralWidget(central_widget);

  // setStatusBar(new QStatusBar(this));
  // statusBar()->showMessage("TEST");
}

EmojiLabel* EmojiWindow::selectedEmojiLabel() {
  QLayoutItem* item = emoji_list_layout->itemAtPosition(selectedRow, selectedCol);
  if (!item) {
    return nullptr;
  }

  EmojiLabel* widget = static_cast<EmojiLabel*>(item->widget());
  return widget;
}

void EmojiWindow::moveSelectedEmojiLabel(int row, int col) {
  selectedEmojiLabel()->setHighlighted(false);
  // selectedEmojiLabel()->repaint();

  selectedRow += row;
  if (selectedRow < 0 || selectedRow >= emoji_list_layout->rowCount()) {
    selectedRow -= row;
  }

  selectedCol += col;
  if (selectedCol < 0 || selectedCol >= emoji_list_layout->columnCount()) {
    selectedCol -= col;
  }

  selectedEmojiLabel()->setHighlighted(true);
  // selectedEmojiLabel()->repaint();

  emoji_list_scroll->ensureWidgetVisible(selectedEmojiLabel());
  // emoji_list_scroll->repaint();
}

void EmojiWindow::reset() {
  disable();
}

void EmojiWindow::enable() {
  show();
}

void EmojiWindow::disable() {
  hide();

  reset_input_method_engine(); // TODO: configurable

  search_edit->setText("");
  // search_edit->repaint();

  // TODO: move to center
}

void EmojiWindow::set_cursor_location(const QRect& rect) {
  if (rect.x() == 0 && rect.y() == 0) {
    return;
  }

  move(rect.x(), rect.y() + rect.height());
}

void EmojiWindow::process_key_event(const QKeyEvent& event) {
  if (event.type() == QKeyEvent::KeyRelease) {
    return;
  }

  if (event.modifiers() & Qt::ControlModifier && event.text() == "a") {
    search_edit->selectAll();
    // search_edit->repaint();
    return;
  }

  if (event.key() == Qt::Key_Escape) {
    disable();
    return;
  }

  if (event.key() == Qt::Key_Return) {
    commit_text(u8"👌");
    return;
  }

  if (event.key() == Qt::Key_Up) {
    moveSelectedEmojiLabel(-1, 0);
    return;
  }

  if (event.key() == Qt::Key_Down) {
    moveSelectedEmojiLabel(1, 0);
    return;
  }

  if (event.key() == Qt::Key_Left) {
    moveSelectedEmojiLabel(0, -1);
    return;
  }

  if (event.key() == Qt::Key_Right) {
    moveSelectedEmojiLabel(0, 1);
    return;
  }

  if (search_edit->hasSelectedText()) {
    search_edit->setText("");
  }

  if (event.key() == Qt::Key_Backspace) {
    search_edit->setText(search_edit->text().left(search_edit->text().length() - 1));
    // search_edit->repaint();
    return;
  }

  search_edit->setText(search_edit->text() + event.text());
  // search_edit->repaint();

  // QKeyEvent::Type event_type = (modifiers & IBUS_RELEASE_MASK) ? QKeyEvent::KeyRelease : QKeyEvent::KeyPress;
  // int event_key = keyval;
  // Qt::KeyboardModifiers event_modifiers = Qt::NoModifier;
  // QString event_text = QString((char)keyval);

  // QKeyEvent* event = new QKeyEvent(event_type, event_key, event_modifiers, event_text);
  // QCoreApplication::sendEvent(this, event);
}

void gui_main(int argc, char** argv) {
  QApplication::setOrganizationName(PROJECT_ORGANIZATION);
  QApplication::setOrganizationDomain(PROJECT_ORGANIZATION);
  QApplication::setApplicationName(PROJECT_NAME);
  QApplication::setApplicationVersion(PROJECT_VERSION);

  QApplication app{argc, argv};

  EmojiWindow emoji_window;

  // TODO: improve the following
  QTimer emoji_command_processor;
  QObject::connect(&emoji_command_processor, &QTimer::timeout, [&emoji_window]() {
    std::shared_ptr<EmojiCommand> _command;
    if (emoji_command_queue.pop(_command)) {
      if (auto command = std::dynamic_pointer_cast<EmojiCommandEnable>(_command)) {
        emoji_window.commit_text = command->commit_text;
        emoji_window.enable();
      }
      if (auto command = std::dynamic_pointer_cast<EmojiCommandDisable>(_command)) {
        emoji_window.disable();
      }
      if (auto command = std::dynamic_pointer_cast<EmojiCommandReset>(_command)) {
        emoji_window.reset();
      }
      if (auto command = std::dynamic_pointer_cast<EmojiCommandSetCursorLocation>(_command)) {
        emoji_window.set_cursor_location(*command->rect);
      }
      if (auto command = std::dynamic_pointer_cast<EmojiCommandProcessKeyEvent>(_command)) {
        emoji_window.process_key_event(*command->key_event);
      }
    }
  });
  emoji_command_processor.start(5);

  app.exec();
}
