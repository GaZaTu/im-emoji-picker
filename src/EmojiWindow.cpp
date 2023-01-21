#include "EmojiWindow.hpp"
#include <QCoreApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QStatusBar>
#include <QTimer>
#include <QApplication>
#include <exception>
#include <memory>
#include <qnamespace.h>
#include <qscrollarea.h>
#include <vector>
#include "EmojiLabel.hpp"
#include "emojis.hpp"
#include "logging.hpp"

std::function<void()> reset_input_method_engine = []() {};

ThreadsafeQueue<std::shared_ptr<EmojiCommand>> emoji_command_queue;

EmojiWindow::EmojiWindow() : QMainWindow() {
  setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
  setWindowIcon(QIcon(":/res/dank-emoji-picker.png"));
  setWindowOpacity(0.95);
  setFocusPolicy(Qt::NoFocus);
  setAttribute(Qt::WA_ShowWithoutActivating);

  resize(360, 200);

  for (const auto& emoji : emojis) {
    auto emoji_layout_widget = new EmojiLabel(_emoji_list_widget, emoji);
    auto emoji_layout_item = new QWidgetItemV2(emoji_layout_widget);

    _allocated_emoji_layout_items.emplace_back(emoji_layout_item);
  }

  _emoji_list_widget->setLayout(_emoji_list_layout);
  _emoji_list_layout->setContentsMargins(4, 4, 0, 4);

  updateEmojiList();

  _emoji_list_scroll->setWidgetResizable(true);
  _emoji_list_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  _emoji_list_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  // _emoji_list_scroll->setSizeAdjustPolicy(QScrollArea::AdjustToContents);
  _emoji_list_scroll->setWidget(_emoji_list_widget);

  _central_widget->setLayout(_central_layout);
  _central_layout->setContentsMargins(0, 0, 0, 0);
  _central_layout->setSpacing(0);
  _central_layout->addWidget(_search_edit);
  _central_layout->addWidget(_emoji_list_scroll);

  setCentralWidget(_central_widget);

  // setStatusBar(new QStatusBar(this));
  // statusBar()->showMessage("TEST");
}

EmojiLabel* EmojiWindow::selectedEmojiLabel() {
  QLayoutItem* item = _emoji_list_layout->itemAtPosition(_selected_row, _selected_col);
  if (!item) {
    return nullptr;
  }

  EmojiLabel* widget = static_cast<EmojiLabel*>(item->widget());
  return widget;
}

void showEmojiRow(QGridLayout* _emoji_list_layout, int row) {
  for (int col = 0; col < _emoji_list_layout->columnCount(); col++) {
    QLayoutItem* emoji_layout_item = _emoji_list_layout->itemAtPosition(row, col);
    if (!emoji_layout_item) {
      continue;
    }

    auto label = static_cast<EmojiLabel*>(emoji_layout_item->widget());
    label->show();
  }
}

void EmojiWindow::moveSelectedEmojiLabel(int row, int col) {
  if (selectedEmojiLabel()) {
    selectedEmojiLabel()->setHighlighted(false);
  }

  _selected_row += row;
  _selected_col += col;

  if (!selectedEmojiLabel()) {
    _selected_row -= row;
    _selected_col -= col;
  }

  if (selectedEmojiLabel()) {
    selectedEmojiLabel()->setHighlighted(true);

    if (row != 0) {
      for (int x = _selected_row; x < std::min(_selected_row + 5, _emoji_list_layout->rowCount()); x++) {
        showEmojiRow(_emoji_list_layout, x);
      }
      for (int x = _selected_row; x >= std::max(_selected_row - 5, 0); x--) {
        showEmojiRow(_emoji_list_layout, x);
      }
    }

    _emoji_list_scroll->ensureWidgetVisible(selectedEmojiLabel());
  }
}

bool stringIncludes(const std::string& text, const std::string& search) {
  auto found = std::search(text.begin(), text.end(), search.begin(), search.end(), [](char c1, char c2) {
    return std::tolower(c1) == std::tolower(c2);
  });

  if (search.length() >= 3) {
    return found != text.end();
  } else {
    return found == text.begin();
  }
}

void EmojiWindow::updateEmojiList() {
  QLayoutItem* item_to_remove;
  while ((item_to_remove = _emoji_list_layout->itemAt(0))) {
    auto label = static_cast<EmojiLabel*>(item_to_remove->widget());
    const auto& emoji = label->emoji();

    _emoji_list_layout->removeItem(item_to_remove);

    label->hide();
  }

  std::string search = _search_edit->text().toStdString();

  int row = 0;
  int col = 0;
  for (auto emoji_layout_item : _allocated_emoji_layout_items) {
    auto label = static_cast<EmojiLabel*>(emoji_layout_item->widget());
    const auto& emoji = label->emoji();

    if (search != "" && !stringIncludes(emoji.name, search)) {
      continue;
    }

    if (row <= 5) {
      label->show();
    }

    _emoji_list_layout->addItem(&*emoji_layout_item, row, col);

    col += 1;
    if (col >= 10) {
      col = 0;
      row += 1;
    }

    if (search != "" && row >= 5) {
      break;
    }
  }

  _selected_row = 0;
  _selected_col = 0;
  if (selectedEmojiLabel()) {
    selectedEmojiLabel()->setHighlighted(true);
  }

  _emoji_list_scroll->setWidget(_emoji_list_widget);
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

  _search_edit->setText("");

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
    _search_edit->selectAll();
    return;
  }

  if (event.key() == Qt::Key_Escape) {
    disable();
    return;
  }

  if (event.key() == Qt::Key_Return) {
    commit_text(selectedEmojiLabel()->emoji().code);
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

  if (_search_edit->hasSelectedText()) {
    _search_edit->setText("");
  }

  if (event.key() == Qt::Key_Backspace) {
    _search_edit->setText(_search_edit->text().left(_search_edit->text().length() - 1));
    updateEmojiList();
    return;
  }

  _search_edit->setText(_search_edit->text() + event.text());
  updateEmojiList();
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
