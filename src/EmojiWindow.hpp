#pragma once

#include "EmojiLabel.hpp"
#include "ThreadsafeQueue.hpp"
#include <QGridLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>
#include <functional>
#include <memory>
#include <qevent.h>
#include <string>

extern std::function<void()> reset_input_method_engine;

struct EmojiCommand {
public:
  virtual ~EmojiCommand() {
  }
};

struct EmojiCommandEnable : public EmojiCommand {
public:
  std::function<void(const std::string&)> commit_text;

  EmojiCommandEnable(std::function<void(const std::string&)> commit_text) : EmojiCommand(), commit_text{std::move(commit_text)} {
  }
};

struct EmojiCommandDisable : public EmojiCommand {
public:
  EmojiCommandDisable() : EmojiCommand() {
  }
};

struct EmojiCommandReset : public EmojiCommand {
public:
  EmojiCommandReset() : EmojiCommand() {
  }
};

struct EmojiCommandSetCursorLocation : public EmojiCommand {
public:
  std::shared_ptr<QRect> rect;

  EmojiCommandSetCursorLocation(QRect* rect) : EmojiCommand(), rect{rect} {
  }
};

struct EmojiCommandProcessKeyEvent : public EmojiCommand {
public:
  std::shared_ptr<QKeyEvent> key_event;

  EmojiCommandProcessKeyEvent(QKeyEvent* key_event) : EmojiCommand(), key_event{key_event} {
  }
};

extern ThreadsafeQueue<std::shared_ptr<EmojiCommand>> emoji_command_queue;

struct EmojiWindow : public QMainWindow {
  Q_OBJECT

public:
  std::function<void(const std::string&)> commit_text;

  explicit EmojiWindow();

public Q_SLOTS:
  void reset();
  void enable();
  void disable();
  void set_cursor_location(const QRect& rect);
  void process_key_event(const QKeyEvent& event);

private:
  bool standalone = false;

  int selectedRow = 0;
  int selectedCol = 0;
  EmojiLabel* selectedEmojiLabel();
  void moveSelectedEmojiLabel(int row, int col);

  QWidget* central_widget = new QWidget(this);
  QVBoxLayout* central_layout = new QVBoxLayout(central_widget);

  QLineEdit* search_edit = new QLineEdit(central_widget);

  QScrollArea* emoji_list_scroll = new QScrollArea(central_widget);
  QWidget* emoji_list_widget = new QWidget(emoji_list_scroll);
  QGridLayout* emoji_list_layout = new QGridLayout(emoji_list_widget);
};

void gui_main(int argc, char** argv);
