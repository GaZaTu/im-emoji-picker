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
#include <qlayoutitem.h>
#include <qwidget.h>
#include <string>
#include <QStackedLayout>

extern std::function<void()> resetInputMethodEngine;

struct EmojiCommand {
public:
  virtual ~EmojiCommand() {
  }
};

struct EmojiCommandEnable : public EmojiCommand {
public:
  std::function<void(const std::string&)> commitText;

  EmojiCommandEnable(std::function<void(const std::string&)> commitText) : EmojiCommand(), commitText{std::move(commitText)} {
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
  std::shared_ptr<QKeyEvent> keyEvent;

  EmojiCommandProcessKeyEvent(QKeyEvent* keyEvent) : EmojiCommand(), keyEvent{keyEvent} {
  }
};

extern ThreadsafeQueue<std::shared_ptr<EmojiCommand>> emojiCommandQueue;

struct EmojiWindow : public QMainWindow {
  Q_OBJECT

public:
  std::function<void(const std::string&)> commitText;

  explicit EmojiWindow();

public Q_SLOTS:
  void reset();
  void enable();
  void disable();
  void setCursorLocation(const QRect& rect);
  void processKeyEvent(const QKeyEvent& event);

private:
  std::vector<std::shared_ptr<QWidgetItem>> _emojiLayoutItems;

  int _selectedRow = 0;
  int _selectedColumn = 0;
  EmojiLabel* selectedEmojiLabel();
  void moveSelectedEmojiLabel(int row, int col);

  QWidget* _centralWidget = new QWidget(this);
  QVBoxLayout* _centralLayout = new QVBoxLayout(_centralWidget);

  QWidget* _searchContainerWidget = new QWidget(_centralWidget);
  QStackedLayout* _searchContainerLayout = new QStackedLayout(_searchContainerWidget);
  QLineEdit* _searchEdit = new QLineEdit(_searchContainerWidget);
  QLabel* _searchCompletion = new QLabel(_searchEdit);

  QScrollArea* _emojiListScroll = new QScrollArea(_centralWidget);
  QWidget* _emojiListWidget = new QWidget(_emojiListScroll);
  QGridLayout* _emojiListLayout = new QGridLayout(_emojiListWidget);

  void updateSearchCompletion();
  void updateEmojiList();
};

void gui_main(int argc, char** argv);
