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
#include <qlineedit.h>
#include <qnamespace.h>
#include <qscrollarea.h>
#include <qwindowdefs.h>
#include <vector>
#include "EmojiCache.hpp"
#include "EmojiLabel.hpp"
#include "emojis.hpp"
#include "logging.hpp"
#include <QClipboard>

std::function<void()> resetInputMethodEngine = []() {};

ThreadsafeQueue<std::shared_ptr<EmojiCommand>> emojiCommandQueue;

EmojiWindow::EmojiWindow() : QMainWindow() {
  setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
  setWindowIcon(QIcon(":/res/dank-emoji-picker.png"));
  setWindowOpacity(0.95);
  setFocusPolicy(Qt::NoFocus);
  setAttribute(Qt::WA_ShowWithoutActivating);

  resize(360, 200);

  for (const auto& emoji : emojis) {
    auto emojiLayoutWidget = new EmojiLabel(_emojiListWidget, emoji);
    auto emojiLayoutItem = new QWidgetItemV2(emojiLayoutWidget);

    _emojiLayoutItems.emplace_back(emojiLayoutItem);
    emojiLayoutWidget->hide();
  }

  _searchContainerWidget->setLayout(_searchContainerLayout);
  _searchContainerLayout->setStackingMode(QStackedLayout::StackAll);

  _searchEdit->setTextMargins(1, 0, 0, 0);

  _searchCompletion->setIndent(_searchEdit->fontMetrics().averageCharWidth());
  QColor _searchCompletionTextColor = _searchEdit->palette().text().color();
  _searchCompletionTextColor.setAlphaF(0.6);
  _searchCompletion->setStyleSheet(QString("color: #%1;").arg(_searchCompletionTextColor.rgba(), 0, 16));

  _searchContainerLayout->addWidget(_searchCompletion);
  _searchContainerLayout->addWidget(_searchEdit);

  _emojiListWidget->setLayout(_emojiListLayout);
  _emojiListLayout->setContentsMargins(4, 4, 0, 4);

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

  // setStatusBar(new QStatusBar(this));
  // statusBar()->addPermanentWidget(new EmojiLabel(this, {"", u8"⭐"}));
  // statusBar()->addPermanentWidget(new EmojiLabel(this, {"", u8"🗃"}));
}

EmojiLabel* EmojiWindow::selectedEmojiLabel() {
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

void EmojiWindow::moveSelectedEmojiLabel(int row, int column) {
  if (selectedEmojiLabel()) {
    selectedEmojiLabel()->setHighlighted(false);
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

void EmojiWindow::updateSearchCompletion() {
  QString completion = "";
  if (selectedEmojiLabel()) {
    completion = QString::fromStdString(selectedEmojiLabel()->emoji().name);
  }

  int indexOfText = std::max(completion.indexOf(_searchEdit->text(), 0, Qt::CaseInsensitive), 0);
  int textWidth = _searchEdit->fontMetrics().horizontalAdvance(completion.left(indexOfText));

  completion.replace(indexOfText, _searchEdit->text().length(), _searchEdit->text());
  if (completion.length() > 36) {
    completion = completion.left(36) + "...";
  }

  _searchEdit->setTextMargins(textWidth, 0, 0, 0);

  _searchCompletion->setText(completion);
}

bool EmojiWindow::isDisabledEmoji(const Emoji& emoji) {
  if (_maxEmojiVersion != -1 && (emoji.version > _maxEmojiVersion)) {
    return true;
  }

  if (_skinTonesDisabled && emoji.isSkinToneVariation()) {
    return true;
  }

  if (_gendersDisabled && emoji.isGenderVariation()) {
    return true;
  }

  if (_useSystemEmojiFont && _useSystemEmojiFontWidthHeuristics) {
    if (!fontSupportsEmoji(fontMetrics(), QString::fromStdString(emoji.code))) {
      return true;
    }
  }

  return false;
}

void EmojiWindow::updateEmojiList() {
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

  int row = 0;
  int column = 0;
  for (auto emojiLayoutItem : _emojiLayoutItems) {
    auto label = static_cast<EmojiLabel*>(emojiLayoutItem->widget());
    const auto& emoji = label->emoji();

    if (_mode == ViewMode::MRU && search == "") {
      if (std::find(_emojiMRU.begin(), _emojiMRU.end(), emoji) == _emojiMRU.end()) {
        continue;
      }
    }

    if (isDisabledEmoji(emoji)) {
      continue;
    }

    if (search != "" && !stringIncludes(emoji.name, search)) {
      continue;
    }

    if (row <= 5) {
      label->show();
    }

    _emojiListLayout->addItem(&*emojiLayoutItem, row, column);

    column += 1;
    if (column >= 10) {
      column = 0;
      row += 1;
    }

    if (search != "" && row >= 5) {
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

void EmojiWindow::reset() {
  disable();
}

void EmojiWindow::enable() {
  show();

  _emojiMRU = EmojiCache{}.getEmojiMRU();

  _searchEdit->setText("");
  _searchCompletion->setText("");
  updateEmojiList();
}

void EmojiWindow::disable() {
  if (!isVisible()) {
    return;
  }

  hide();

  // TODO: move to center

  resetInputMethodEngine(); // TODO: configurable

  EmojiCache{}.setEmojiMRU(_emojiMRU);
}

void EmojiWindow::setCursorLocation(const QRect* rect) {
  if (rect->x() == 0 && rect->y() == 0) {
    return;
  }

  move(rect->x(), rect->y() + rect->height());
}

#if __cplusplus <= 202002L
namespace std {
template <typename T>
void erase(std::vector<T>& vector, const T& value) {
  vector.erase(std::remove(vector.begin(), vector.end(), value), vector.end());
}
} // namespace std
#endif

EmojiAction getEmojiActionForQKeyEvent(const QKeyEvent* event) {
  if (event->key() == 0 && !isascii(event->text().at(0).toLatin1())) {
    return EmojiAction::INVALID;
  }

  if (event->type() == QKeyEvent::KeyRelease) {
    return EmojiAction::INVALID;
  }

  if (event->modifiers() & Qt::MetaModifier) {
    return EmojiAction::INVALID;
  }

  if (event->modifiers() & Qt::ControlModifier && event->text().toUpper() == "A") {
    return EmojiAction::SELECT_ALL_IN_SEARCH;
  }

  if (event->modifiers() & Qt::ControlModifier && event->text().toUpper() == "C") {
    return EmojiAction::COPY_SELECTED_EMOJI;
  }

  if (event->modifiers() & Qt::ControlModifier && event->text().toUpper() == "X") {
    return EmojiAction::CUT_SELECTION_IN_SEARCH;
  }

  if (event->modifiers() & Qt::ControlModifier) {
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
    return EmojiAction::CHANGE_SELECTED_EMOJI;
  }

  if (event->key() == Qt::Key_Down) {
    return EmojiAction::CHANGE_SELECTED_EMOJI;
  }

  if (event->key() == Qt::Key_Left) {
    return EmojiAction::CHANGE_SELECTED_EMOJI;
  }

  if (event->key() == Qt::Key_Right) {
    return EmojiAction::CHANGE_SELECTED_EMOJI;
  }

  if (event->key() == Qt::Key_Backspace) {
    return EmojiAction::REMOVE_CHAR_IN_SEARCH;
  }

  return EmojiAction::INSERT_CHAR_IN_SEARCH;
}

void EmojiWindow::processKeyEvent(const QKeyEvent* event) {
  EmojiAction action = getEmojiActionForQKeyEvent(event);

  switch (action) {
  case EmojiAction::INVALID:
    break;

  case EmojiAction::SELECT_ALL_IN_SEARCH:
    _searchEdit->selectAll();
    break;

  case EmojiAction::COPY_SELECTED_EMOJI:
    // TODO: copy selected emoji
    break;

  case EmojiAction::DISABLE:
    disable();
    break;

  case EmojiAction::COMMIT_EMOJI:
    if (selectedEmojiLabel()) {
      const Emoji& emoji = selectedEmojiLabel()->emoji();
      commitText(emoji.code);

      std::erase(_emojiMRU, emoji);
      _emojiMRU.insert(_emojiMRU.begin(), emoji);
    }
    if (event->modifiers() & Qt::ShiftModifier) {
      disable();
    }
    break;

  case EmojiAction::SWITCH_VIEW_MODE:
    switch (_mode) {
    case ViewMode::MRU:
      _mode = ViewMode::LIST;
      break;
    case ViewMode::LIST:
      _mode = ViewMode::MRU;
      break;
    }
    _searchEdit->setText("");
    _searchCompletion->setText("");
    updateEmojiList();
    break;

  case EmojiAction::CHANGE_SELECTED_EMOJI:
    if (event->key() == Qt::Key_Up) {
      moveSelectedEmojiLabel(-1, 0);
    }
    if (event->key() == Qt::Key_Down) {
      moveSelectedEmojiLabel(1, 0);
    }
    if (event->key() == Qt::Key_Left) {
      moveSelectedEmojiLabel(0, -1);
    }
    if (event->key() == Qt::Key_Right) {
      moveSelectedEmojiLabel(0, 1);
    }
    break;

  case EmojiAction::CUT_SELECTION_IN_SEARCH:
    if (_searchEdit->hasSelectedText()) {
      _searchEdit->setText("");
    }
    // ctrl+a -> ctrl+x = empty text field
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

  EmojiWindow window;

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
