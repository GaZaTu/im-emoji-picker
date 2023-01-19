#pragma once

#include "ThreadsafeQueue.hpp"
#include <QGridLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QWidget>
#include <functional>

extern std::function<void()> reset_ibus_engine_to_original;

#define KEYCODE_ESCAPE 1
#define KEYCODE_RETURN 28
#define KEYCODE_BACKSPACE 14
#define KEYCODE_ARROW_UP 103
#define KEYCODE_ARROW_DOWN 108
#define KEYCODE_ARROW_LEFT 103
#define KEYCODE_ARROW_RIGHT 108

struct RimeCommand {
public:
  virtual ~RimeCommand() {
  }
};

struct RimeCommandInit : public RimeCommand {
public:
  std::function<void(const std::string&)> commit_text;

  RimeCommandInit(std::function<void(const std::string&)> commit_text) : RimeCommand() {
    this->commit_text = commit_text;
  }
};

struct RimeCommandReset : public RimeCommand {
public:
  RimeCommandReset() : RimeCommand() {
  }
};

struct RimeCommandEnable : public RimeCommand {
public:
  RimeCommandEnable() : RimeCommand() {
  }
};

struct RimeCommandDisable : public RimeCommand {
public:
  RimeCommandDisable() : RimeCommand() {
  }
};

struct RimeCommandSetCursorLocation : public RimeCommand {
public:
  int x;
  int y;
  int w;
  int h;

  RimeCommandSetCursorLocation(int x, int y, int w, int h) : RimeCommand() {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
  }
};

struct RimeCommandProcessKeyEvent : public RimeCommand {
public:
  uint keyval;
  uint keycode;
  uint modifiers;

  RimeCommandProcessKeyEvent(uint keyval, uint keycode, uint modifiers) : RimeCommand() {
    this->keyval = keyval;
    this->keycode = keycode;
    this->modifiers = modifiers;
  }
};

extern ThreadsafeQueue<std::shared_ptr<RimeCommand>> rime_command_queue;

struct RimeWindow : public QMainWindow {
  Q_OBJECT

public:
  std::function<void(const std::string&)> commit_text;

  explicit RimeWindow();

public Q_SLOTS:
  void reset();
  void enable();
  void disable();
  void set_cursor_location(int x, int y, int w, int h);
  void process_key_event(uint keyval, uint keycode, uint modifiers);

private:
  QWidget* central_widget = new QWidget(this);
  QVBoxLayout* central_layout = new QVBoxLayout(central_widget);

  QLineEdit* search_edit = new QLineEdit(central_widget);

  QWidget* emoji_list = new QWidget(central_widget);
  QGridLayout* emoji_list_layout = new QGridLayout(emoji_list);
};
