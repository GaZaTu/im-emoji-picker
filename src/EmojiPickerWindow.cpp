#include "EmojiPickerWindow.hpp"
#include "EmojiLabel.hpp"
#include "emojis.hpp"
#include "kaomojis.hpp"
#include "logging.hpp"
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QScreen>
#include <QStyle>
#include <QTimer>
#include <algorithm>
#include <exception>
#include <memory>
#include <vector>

Emoji convertKaomojiToEmoji(const Kaomoji& kaomoji) {
  return Emoji{kaomoji.name, kaomoji.text, -1};
}

void moveQWidgetToCenter(QWidget* window) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
  auto rect = QApplication::screenAt(QCursor::pos())->geometry();
#else
  auto rect = qApp->desktop()->availableGeometry(QCursor::pos());
#endif

  window->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, window->size(), rect));
}

std::function<void()> resetInputMethodEngine = []() {};

ThreadsafeQueue<std::shared_ptr<EmojiCommand>> emojiCommandQueue;

EmojiPickerWindow::EmojiPickerWindow() : QMainWindow() {
  setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
  setWindowIcon(QIcon(":/res/im-emoji-picker_72x72.png"));
  setWindowOpacity(_settings.windowOpacity());
  setFocusPolicy(Qt::NoFocus);
  setAttribute(Qt::WA_ShowWithoutActivating);
  setFixedSize(340, 180 + (_settings.hideStatusBar() ? 0 : 20));

  for (const auto& emoji : emojis) {
    auto emojiLayoutWidget = new EmojiLabel(_emojiListWidget, _settings, emoji);
    auto emojiLayoutItem = new QWidgetItemV2(emojiLayoutWidget);

    _emojiLayoutItems.emplace_back(emojiLayoutItem);
    emojiLayoutWidget->hide();
  }

  for (const auto& kaomoji : kaomojis) {
    auto emojiLayoutWidget = new EmojiLabel(_emojiListWidget, _settings, convertKaomojiToEmoji(kaomoji));
    auto emojiLayoutItem = new QWidgetItemV2(emojiLayoutWidget);

    _kaomojiLayoutItems.emplace_back(emojiLayoutItem);
    emojiLayoutWidget->hide();
  }

  _searchContainerWidget->setLayout(_searchContainerLayout);
  _searchContainerLayout->setStackingMode(QStackedLayout::StackAll);

  _searchCompletion->setStyleSheet(_searchCompletion->styleSheet() + QString("background: #00000000;"));
  QColor _searchCompletionTextColor = _searchEdit->palette().text().color();
  _searchCompletionTextColor.setAlphaF(0.6);
  _searchCompletion->setStyleSheet(_searchCompletion->styleSheet() + QString("color: #%1;").arg(_searchCompletionTextColor.rgba(), 0, 16));

  _searchContainerLayout->addWidget(_searchCompletion);
  _searchContainerLayout->addWidget(_searchEdit);

  _emojiListWidget->setLayout(_emojiListLayout);
  _emojiListLayout->setContentsMargins(4, 4, 4, 4);
  _emojiListLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

  _emojiListScroll->setWidgetResizable(true);
  _emojiListScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  _emojiListScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  _emojiListScroll->setWidget(_emojiListWidget);

  _centralWidget->setLayout(_centralLayout);
  _centralLayout->setContentsMargins(0, 0, 0, 0);
  _centralLayout->setSpacing(0);
  _centralLayout->addWidget(_searchContainerWidget);
  _centralLayout->addWidget(_emojiListScroll, 1);

  setCentralWidget(_centralWidget);

  if (!_settings.hideStatusBar()) {
    _mruModeLabel->setEmoji({"", u8"⭐"}, 14, 14);
    _mruModeLabel->setHighlighted(_mode == ViewMode::MRU);
    _listModeLabel->setEmoji({"", u8"🗃"}, 14, 14);
    _listModeLabel->setHighlighted(_mode == ViewMode::LIST);
    _kaomojiModeLabel->setEmoji({"", u8"ヽ(o^ ^o)ﾉ"}, 18, 18);
    _kaomojiModeLabel->setHighlighted(_mode == ViewMode::KAOMOJI);

    _statusBar->setFixedHeight(20);
    _statusBar->addPermanentWidget(_mruModeLabel);
    _statusBar->addPermanentWidget(_listModeLabel);
    _statusBar->addPermanentWidget(_kaomojiModeLabel);

    setStatusBar(_statusBar);
  }

  EmojiPickerSettings::writeDefaultsToDisk();
}

EmojiLabel* EmojiPickerWindow::selectedEmojiLabel() {
  QLayoutItem* item = _emojiListLayout->itemAtPosition(_selectedRow, _selectedColumn);
  if (!item) {
    return nullptr;
  }

  EmojiLabel* widget = static_cast<EmojiLabel*>(item->widget());
  return widget;
}

void showEmojiRow(QGridLayout* _emojiListLayout, int row) {
  for (int column = 0; column < _emojiListLayout->columnCount(); column++) {
    QLayoutItem* emojiLayoutItem = _emojiListLayout->itemAtPosition(row, column);
    if (!emojiLayoutItem) {
      continue;
    }

    auto label = static_cast<EmojiLabel*>(emojiLayoutItem->widget());
    label->show();
  }
}

void EmojiPickerWindow::moveSelectedEmojiLabel(int row, int column) {
  if (selectedEmojiLabel()) {
    selectedEmojiLabel()->setHighlighted(false);

    // the following is a piece of shit
    // basically kaomoji have a colspan of 2 which means:
    // moving left or right requires a 2 instead of 1
    if (!selectedEmojiLabel()->hasRealEmoji()) {
      column *= 2;
    }
  }

  _selectedRow += row;
  _selectedColumn += column;

  if (!selectedEmojiLabel()) {
    _selectedRow -= row;
    _selectedColumn -= column;
  }

  if (selectedEmojiLabel()) {
    selectedEmojiLabel()->setHighlighted(true);

    if (row != 0) {
      for (int x = _selectedRow; x < std::min(_selectedRow + 5, _emojiListLayout->rowCount()); x++) {
        showEmojiRow(_emojiListLayout, x);
      }
      for (int x = _selectedRow; x >= std::max(_selectedRow - 5, 0); x--) {
        showEmojiRow(_emojiListLayout, x);
      }
    }

    _emojiListScroll->ensureWidgetVisible(selectedEmojiLabel());

    updateSearchCompletion();
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

bool EmojiPickerWindow::emojiMatchesSearch(const Emoji& emoji, const std::string& search, std::string& found) {
  if (stringIncludes(emoji.name, search)) {
    found = emoji.name;
    return true;
  }

  for (const std::string& alias : _emojiAliases[emoji.code]) {
    if (stringIncludes(alias, search)) {
      found = alias;
      return true;
    }
  }

  return false;
}

bool EmojiPickerWindow::emojiMatchesSearch(const Emoji& emoji, const std::string& search) {
  std::string found;

  return emojiMatchesSearch(emoji, search, found);
}

void EmojiPickerWindow::updateSearchCompletion() {
  QString search = _searchEdit->text();

  QString completion = "";
  if (selectedEmojiLabel()) {
    const Emoji& emoji = selectedEmojiLabel()->emoji();

    std::string found;
    if (emojiMatchesSearch(emoji, search.toStdString(), found)) {
      completion = QString::fromStdString(found);
    }
  }

  int indexOfSearch = std::max(completion.indexOf(search, 0, Qt::CaseInsensitive), 0);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
  int offsetOfSearch = _searchEdit->fontMetrics().horizontalAdvance(completion.left(indexOfSearch));
#else
  int offsetOfSearch = _searchEdit->fontMetrics().width(completion.left(indexOfSearch));
#endif

  _searchEdit->setTextMargins(offsetOfSearch, 0, 0, 0);

  _searchCompletion->setText(completion);
}

void EmojiPickerWindow::addItemToEmojiList(QLayoutItem* emojiLayoutItem, EmojiLabel* label, int& row, int& column) {
  int colspan = 1;
  if (!label->hasRealEmoji()) {
    colspan = 2;
  }

  if ((column + colspan) > 10) {
    column = 0;
    row += 1;
  }

  _emojiListLayout->addItem(emojiLayoutItem, row, column, 1, colspan, Qt::AlignHCenter | Qt::AlignBaseline);

  column += colspan;
  if (column >= 10) {
    column = 0;
    row += 1;
  }
}

void EmojiPickerWindow::updateEmojiList() {
  if (selectedEmojiLabel()) {
    selectedEmojiLabel()->setHighlighted(false);
  }

  QLayoutItem* itemToRemove;
  while ((itemToRemove = _emojiListLayout->itemAt(0))) {
    auto label = static_cast<EmojiLabel*>(itemToRemove->widget());
    const auto& emoji = label->emoji();

    _emojiListLayout->removeItem(itemToRemove);

    label->hide();
  }

  std::string search = _searchEdit->text().toStdString();

  switch (_mode) {
  case ViewMode::MRU: {
    if (search == "") {
      int row = 0;
      int column = 0;
      for (const auto& emoji : _emojiMRU) {
        auto emojiLayoutItemIterator = std::find_if(_emojiLayoutItems.begin(), _emojiLayoutItems.end(), [&emoji](std::shared_ptr<QWidgetItem> emojiLayoutItem) {
          auto label = static_cast<EmojiLabel*>(emojiLayoutItem->widget());
          auto matches = (label->emoji() == emoji);

          return matches;
        });
        if (emojiLayoutItemIterator == _emojiLayoutItems.end()) {
          continue;
        }

        auto emojiLayoutItem = *emojiLayoutItemIterator;

        auto label = static_cast<EmojiLabel*>(emojiLayoutItem->widget());
        label->show();

        addItemToEmojiList(&*emojiLayoutItem, label, row, column);
      }
      break;
    }
    // fallthrough to ViewMode::LIST if search != ""
  }

  case ViewMode::LIST: {
    int row = 0;
    int column = 0;
    for (auto emojiLayoutItem : _emojiLayoutItems) {
      auto label = static_cast<EmojiLabel*>(emojiLayoutItem->widget());
      const auto& emoji = label->emoji();

      if (_disabledEmojis.count(emoji.code) != 0) {
        continue;
      }

      if (search != "" && !emojiMatchesSearch(emoji, search)) {
        continue;
      }

      if (row <= 5) {
        label->show();
      }

      addItemToEmojiList(&*emojiLayoutItem, label, row, column);

      if (search != "" && row >= 5) {
        break;
      }
    }
    break;
  }

  case ViewMode::KAOMOJI: {
    int row = 0;
    int column = 0;
    for (auto emojiLayoutItem : _kaomojiLayoutItems) {
      auto label = static_cast<EmojiLabel*>(emojiLayoutItem->widget());
      const auto& emoji = label->emoji();

      if (_disabledEmojis.count(emoji.code) != 0) {
        continue;
      }

      if (search != "" && !stringIncludes(emoji.name, search)) {
        continue;
      }

      if (row <= 6) {
        label->show();
      }

      addItemToEmojiList(&*emojiLayoutItem, label, row, column);

      if (search != "" && row >= 5) {
        break;
      }
    }
    break;
  }
  }

  _selectedRow = 0;
  _selectedColumn = 0;
  if (selectedEmojiLabel()) {
    selectedEmojiLabel()->setHighlighted(true);
  }

  updateSearchCompletion();
}

void EmojiPickerWindow::reset() {
  disable();
}

void EmojiPickerWindow::enable() {
  moveQWidgetToCenter(this);

  show();

  _settings.sync();

  _disabledEmojis.clear();
  for (const Emoji& emoji : emojis) {
    if (_settings.isDisabledEmoji(emoji, fontMetrics())) {
      _disabledEmojis.insert(emoji.code);
    }
  }

  _emojiAliases = _settings.emojiAliases();

  _emojiMRU = EmojiPickerCache{}.emojiMRU();

  _searchEdit->setText("");
  _searchCompletion->setText("");
  updateEmojiList();
}

void EmojiPickerWindow::disable() {
  if (!isVisible()) {
    return;
  }

  hide();

  resetInputMethodEngine();

  EmojiPickerCache{}.emojiMRU(_emojiMRU);
}

void EmojiPickerWindow::setCursorLocation(const QRect* rect) {
  if (rect->x() == 0 && rect->y() == 0) {
    return;
  }

  move(rect->x(), rect->y() + rect->height());
}

EmojiAction getEmojiActionForQKeyEvent(const QKeyEvent* event) {
  // TODO: ctrl+w = delete word
  // TODO: ctrl+d = select word
  // TODO: key values instead of key codes

  if (event->key() == 0 && !isascii(event->text().at(0).toLatin1())) {
    return EmojiAction::INVALID;
  }

  if (event->type() == QKeyEvent::KeyRelease) {
    return EmojiAction::INVALID;
  }

  if (event->modifiers() & Qt::MetaModifier) {
    return EmojiAction::INVALID;
  }

  if (event->modifiers() & Qt::ControlModifier) {
    if (event->text().toUpper() == "A") {
      return EmojiAction::SELECT_ALL_IN_SEARCH;
    }

    if (event->text().toUpper() == "C") {
      return EmojiAction::COPY_SELECTED_EMOJI;
    }

    if (event->text().toUpper() == "X") {
      return EmojiAction::CUT_SELECTION_IN_SEARCH;
    }

    if (event->key() == Qt::Key_Up) {
      return EmojiAction::PAGE_UP;
    }

    if (event->key() == Qt::Key_Down) {
      return EmojiAction::PAGE_DOWN;
    }

    if (event->key() == Qt::Key_Backspace) {
      return EmojiAction::CLEAR_SEARCH;
    }

    return EmojiAction::INVALID;
  }

  if (event->key() == Qt::Key_Escape) {
    return EmojiAction::DISABLE;
  }

  if (event->key() == Qt::Key_Return) {
    return EmojiAction::COMMIT_EMOJI;
  }

  if (event->key() == Qt::Key_Tab) {
    return EmojiAction::SWITCH_VIEW_MODE;
  }

  if (event->key() == Qt::Key_Up) {
    return EmojiAction::UP;
  }

  if (event->key() == Qt::Key_Down) {
    return EmojiAction::DOWN;
  }

  if (event->key() == Qt::Key_Left) {
    return EmojiAction::LEFT;
  }

  if (event->key() == Qt::Key_Right) {
    return EmojiAction::RIGHT;
  }

  if (event->key() == Qt::Key_PageUp) {
    return EmojiAction::PAGE_UP;
  }

  if (event->key() == Qt::Key_PageDown) {
    return EmojiAction::PAGE_DOWN;
  }

  if (event->key() == Qt::Key_F4) {
    return EmojiAction::OPEN_SETTINGS;
  }

  if (event->key() == Qt::Key_Backspace) {
    return EmojiAction::REMOVE_CHAR_IN_SEARCH;
  }

  return EmojiAction::INSERT_CHAR_IN_SEARCH;
}

void EmojiPickerWindow::processKeyEvent(const QKeyEvent* event) {
  EmojiAction action = getEmojiActionForQKeyEvent(event);

  switch (action) {
  case EmojiAction::INVALID:
    break;

  case EmojiAction::SELECT_ALL_IN_SEARCH:
    _searchEdit->selectAll();
    break;

  case EmojiAction::COPY_SELECTED_EMOJI:
    if (selectedEmojiLabel()) {
      const EmojiLabel* label = selectedEmojiLabel();
      const Emoji& emoji = label->emoji();

      QApplication::clipboard()->setText(QString::fromStdString(emoji.code));
    }
    break;

  case EmojiAction::DISABLE:
    disable();
    break;

  case EmojiAction::COMMIT_EMOJI:
    if (selectedEmojiLabel()) {
      const EmojiLabel* label = selectedEmojiLabel();
      const Emoji& emoji = label->emoji();

      commitText(emoji.code);

      if (label->hasRealEmoji()) {
        _emojiMRU.erase(std::remove(_emojiMRU.begin(), _emojiMRU.end(), emoji), _emojiMRU.end());
        _emojiMRU.insert(_emojiMRU.begin(), emoji);
        while (_emojiMRU.size() > 40) {
          _emojiMRU.pop_back();
        }
      }
    }
    if (((event->modifiers() & Qt::ShiftModifier) && !_settings.closeAfterFirstInput()) || (!(event->modifiers() & Qt::ShiftModifier) && _settings.closeAfterFirstInput())) {
      disable();
    }
    break;

  case EmojiAction::SWITCH_VIEW_MODE:
    switch (_mode) {
    case ViewMode::MRU:
      _mode = ViewMode::LIST;
      break;
    case ViewMode::LIST:
      _mode = ViewMode::KAOMOJI;
      break;
    case ViewMode::KAOMOJI:
      _mode = ViewMode::MRU;
      break;
    }
    _mruModeLabel->setHighlighted(_mode == ViewMode::MRU);
    _listModeLabel->setHighlighted(_mode == ViewMode::LIST);
    _kaomojiModeLabel->setHighlighted(_mode == ViewMode::KAOMOJI);
    _searchEdit->setText("");
    _searchCompletion->setText("");
    updateEmojiList();
    break;

  case EmojiAction::UP:
    moveSelectedEmojiLabel(-1, 0);
    break;

  case EmojiAction::DOWN:
    moveSelectedEmojiLabel(1, 0);
    break;

  case EmojiAction::LEFT:
    moveSelectedEmojiLabel(0, -1);
    break;

  case EmojiAction::RIGHT:
    moveSelectedEmojiLabel(0, 1);
    break;

  case EmojiAction::PAGE_UP:
    moveSelectedEmojiLabel(-4, 0);
    break;

  case EmojiAction::PAGE_DOWN:
    moveSelectedEmojiLabel(4, 0);
    break;

  case EmojiAction::OPEN_SETTINGS:
    QDesktopServices::openUrl(QUrl::fromLocalFile(_settings.fileName()));
    disable();
    break;

  case EmojiAction::CUT_SELECTION_IN_SEARCH:
    if (_searchEdit->hasSelectedText()) {
      _searchEdit->setText("");
    }
    // ctrl+a -> ctrl+x = empty text field
    updateEmojiList();
    break;

  case EmojiAction::CLEAR_SEARCH:
    _searchEdit->setText("");
    updateEmojiList();
    break;

  case EmojiAction::REMOVE_CHAR_IN_SEARCH:
    if (_searchEdit->hasSelectedText()) {
      _searchEdit->setText("");
    }
    _searchEdit->setText(_searchEdit->text().left(_searchEdit->text().length() - 1));
    updateEmojiList();
    break;

  case EmojiAction::INSERT_CHAR_IN_SEARCH:
    if (_searchEdit->hasSelectedText()) {
      _searchEdit->setText("");
    }
    _searchEdit->setText(_searchEdit->text() + event->text());
    updateEmojiList();
    break;
  }
}

void gui_main(int argc, char** argv) {
  QApplication::setOrganizationName(PROJECT_ORGANIZATION);
  QApplication::setOrganizationDomain(PROJECT_ORGANIZATION);
  QApplication::setApplicationName(PROJECT_NAME);
  QApplication::setApplicationVersion(PROJECT_VERSION);

  QApplication app{argc, argv};

  // TODO maybe: emoji translations

  // TODO maybe: if Qt system theme is not desired: app.setStyle("fusion")

  EmojiPickerWindow window;

  // TODO: improve the following
  QTimer emojiCommandProcessor;
  QObject::connect(&emojiCommandProcessor, &QTimer::timeout, [&window]() {
    std::shared_ptr<EmojiCommand> _command;
    if (emojiCommandQueue.pop(_command)) {
      if (auto command = std::dynamic_pointer_cast<EmojiCommandEnable>(_command)) {
        window.commitText = command->commitText;
        window.enable();
      }
      if (auto command = std::dynamic_pointer_cast<EmojiCommandDisable>(_command)) {
        window.disable();
      }
      if (auto command = std::dynamic_pointer_cast<EmojiCommandReset>(_command)) {
        window.reset();
      }
      if (auto command = std::dynamic_pointer_cast<EmojiCommandSetCursorLocation>(_command)) {
        window.setCursorLocation(&*command->rect);
      }
      if (auto command = std::dynamic_pointer_cast<EmojiCommandProcessKeyEvent>(_command)) {
        window.processKeyEvent(&*command->keyEvent);
      }
    }
  });
  emojiCommandProcessor.start(5);

  app.exec();
}
