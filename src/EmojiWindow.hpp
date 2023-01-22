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
#include <qstatusbar.h>
#include <qwidget.h>
#include <string>
#include <QStackedLayout>
#include <vector>

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

enum class EmojiAction {
  INVALID,
  SELECT_ALL_IN_SEARCH,
  COPY_SELECTED_EMOJI,
  DISABLE,
  COMMIT_EMOJI,
  SWITCH_VIEW_MODE,
  CHANGE_SELECTED_EMOJI,
  CUT_SELECTION_IN_SEARCH,
  REMOVE_CHAR_IN_SEARCH,
  INSERT_CHAR_IN_SEARCH,
};

EmojiAction getEmojiActionForQKeyEvent(const QKeyEvent* event);

struct EmojiWindow : public QMainWindow {
  Q_OBJECT

public:
  std::function<void(const std::string&)> commitText;

  explicit EmojiWindow();

public Q_SLOTS:
  void reset();
  void enable();
  void disable();
  void setCursorLocation(const QRect* rect);
  void processKeyEvent(const QKeyEvent* event);

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

  short _maxEmojiVersion = -1;
  bool _skinTonesDisabled = true;
  bool _gendersDisabled = true;
  bool _useSystemEmojiFont = false;
  bool _useSystemEmojiFontWidthHeuristics = true;

  bool isDisabledEmoji(const Emoji& emoji);

  std::vector<Emoji> _emojiMRU;

  enum class ViewMode {
    MRU,
    LIST,
  };

  ViewMode _mode = ViewMode::MRU;

  QStatusBar* _statusBar = new QStatusBar(this);
  EmojiLabel* _mruModeLabel = new EmojiLabel(_statusBar);
  EmojiLabel* _listModeLabel = new EmojiLabel(_statusBar);
};

void gui_main(int argc, char** argv);
