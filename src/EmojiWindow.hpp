#pragma once

#include "EmojiLabel.hpp"
#include "EmojiSettings.hpp"
#include "ThreadsafeQueue.hpp"
#include <QGridLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QScrollArea>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <functional>
#include <memory>
#include <qevent.h>
#include <qlayoutitem.h>
#include <qstatusbar.h>
#include <qwidget.h>
#include <string>
#include <unordered_set>
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
  UP,
  DOWN,
  LEFT,
  RIGHT,
  PAGE_UP,
  PAGE_DOWN,
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
  EmojiSettings _settings;

  std::vector<std::shared_ptr<QWidgetItem>> _emojiLayoutItems;
  std::vector<std::shared_ptr<QWidgetItem>> _kaomojiLayoutItems;

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

  QStatusBar* _statusBar = new QStatusBar(this);
  EmojiLabel* _mruModeLabel = new EmojiLabel(_statusBar, _settings);
  EmojiLabel* _listModeLabel = new EmojiLabel(_statusBar, _settings);
  EmojiLabel* _kaomojiModeLabel = new EmojiLabel(_statusBar, _settings);

  void addItemToEmojiList(QLayoutItem* emojiLayoutItem, EmojiLabel* label, int& row, int& column);

  void updateSearchCompletion();
  void updateEmojiList();

  bool emojiMatchesSearch(const Emoji& emoji, const std::string& search, std::string& found);
  bool emojiMatchesSearch(const Emoji& emoji, const std::string& search);

  std::unordered_set<std::string> _disabledEmojis;

  std::unordered_map<std::string, std::vector<std::string>> _emojiAliases;

  std::vector<Emoji> _emojiMRU;

  enum class ViewMode {
    MRU,
    LIST,
    KAOMOJI,
  };

  ViewMode _mode = ViewMode::MRU;
};

void gui_main(int argc, char** argv);
