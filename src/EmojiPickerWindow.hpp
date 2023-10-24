#pragma once

#include "EmojiLabel.hpp"
#include "EmojiPickerSettings.hpp"
#include "ThreadsafeQueue.hpp"
#include "kaomojis.hpp"
#include <QGridLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMainWindow>
#include <QScrollArea>
#include <QStackedLayout>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

extern std::function<void()> resetInputMethodEngine;

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
  OPEN_SETTINGS,
  CUT_SELECTION_IN_SEARCH,
  CLEAR_SEARCH,
  REMOVE_CHAR_IN_SEARCH,
  INSERT_CHAR_IN_SEARCH,
};

struct EmojiCommand {
public:
  virtual ~EmojiCommand() {
  }
};

struct EmojiCommandEnable : public EmojiCommand {
public:
  std::function<void(const std::string&)> commitText;

  bool resetPosition;

  EmojiCommandEnable(std::function<void(const std::string&)>&& commitText, bool resetPosition = true) : EmojiCommand(), commitText{std::move(commitText)}, resetPosition{resetPosition} {
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

  EmojiAction action;

  EmojiCommandProcessKeyEvent(QKeyEvent* keyEvent, EmojiAction action = EmojiAction::INVALID) : EmojiCommand(), keyEvent{keyEvent}, action{action} {
  }
};

extern ThreadsafeQueue<std::shared_ptr<EmojiCommand>> emojiCommandQueue;

QKeyEvent* createKeyEventWithUserPreferences(QEvent::Type _type, int _key, Qt::KeyboardModifiers _modifiers, const QString& _text);

EmojiAction getEmojiActionForQKeyEvent(const QKeyEvent* event);

void moveQWidgetToCenter(QWidget* window);

void moveQWidgetToPoint(QWidget* window, QPoint windowPoint);

struct EmojiPickerWindow : public QMainWindow {
  Q_OBJECT

public:
  std::function<void(const std::string&)> commitText;

  explicit EmojiPickerWindow();

  void updateSearchCompletion();
  void updateEmojiList();

public Q_SLOTS:
  void reset();
  void enable(bool resetPosition = true);
  void disable();
  void setCursorLocation(const QRect* rect);
  void processKeyEvent(const QKeyEvent* event, EmojiAction action = EmojiAction::INVALID);

protected:
  void changeEvent(QEvent* event) override;

  void wheelEvent(QWheelEvent* event) override;

private:
  int _rowSize = 10;

  EmojiPickerSettings _settings;

  std::unordered_map<std::string, QWidgetItem*> _emojiLayoutItems;
  QWidgetItem* createEmojiLabel(const Emoji& emoji);

  QWidgetItem* getEmojiLayoutItem(const Emoji& emoji);
  QWidgetItem* getKaomojiLayoutItem(const Kaomoji& kaomoji);

  int _selectedRow = 0;
  int _selectedColumn = 0;
  EmojiLabel* selectedEmojiLabel();
  void moveSelectedEmojiLabel(int row, int col);

  QWidget* _centralWidget = new QWidget(this);
  QVBoxLayout* _centralLayout = new QVBoxLayout(_centralWidget);

  QWidget* _searchContainerWidget = new QWidget(_centralWidget);
  QStackedLayout* _searchContainerLayout = new QStackedLayout(_searchContainerWidget);
  QLineEdit* _searchEdit = new QLineEdit(_searchContainerWidget);
  QLineEdit* _searchCompletion = new QLineEdit(_searchContainerWidget);

  QScrollArea* _emojiListScroll = new QScrollArea(_centralWidget);
  QWidget* _emojiListWidget = new QWidget(_emojiListScroll);
  QGridLayout* _emojiListLayout = new QGridLayout(_emojiListWidget);

  QStatusBar* _statusBar = new QStatusBar(this);
  EmojiLabel* _mruModeLabel = new EmojiLabel(_statusBar, _settings);
  EmojiLabel* _listModeLabel = new EmojiLabel(_statusBar, _settings);
  EmojiLabel* _kaomojiModeLabel = new EmojiLabel(_statusBar, _settings);

  void addItemToEmojiList(QLayoutItem* emojiLayoutItem, EmojiLabel* label, int colspan, int& row, int& column);

  enum class SearchMode {
    AUTO,
    CONTAINS,
    STARTS_WITH,
    EQUALS,
  };

  static bool stringMatches(const QString& target, const QString& search, SearchMode mode);

  bool emojiMatchesSearch(const Emoji& emoji, const QString& search, SearchMode mode, QString& found);
  bool emojiMatchesSearch(const Emoji& emoji, const QString& search, SearchMode mode);

  std::unordered_set<std::string> _disabledEmojis;

  std::unordered_map<std::string, std::vector<QString>> _emojiAliases;

  std::vector<Emoji> _emojiMRU;

  enum class ViewMode {
    MRU,
    LIST,
    KAOMOJI,
  };

  ViewMode _mode = ViewMode::MRU;

  void commitEmoji(const Emoji& emoji, bool isRealEmoji, bool closeAfter);

  bool _closing = false;
};

void gui_set_active(bool active);

void gui_main(int argc, char** argv);
