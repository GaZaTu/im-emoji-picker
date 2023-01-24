#pragma once

#include "emojis.hpp"
#include <QSettings>
#include <utility>
#include <vector>
#include <QFontMetrics>

class EmojiSettings : public QSettings {
  Q_OBJECT

public:
  static void writeDefaultsToDisk();

  explicit EmojiSettings();

  bool skinTonesDisabled() const;
  void skinTonesDisabled(bool skinTonesDisabled);

  bool gendersDisabled() const;
  void gendersDisabled(bool gendersDisabled);

  // bool useSystemQtTheme() const;
  // void useSystemQtTheme(bool useSystemQtTheme);

  int maxEmojiVersion() const;
  void maxEmojiVersion(int maxEmojiVersion);

  bool isDisabledEmoji(const Emoji& emoji, const QFontMetrics& fontMetrics);

  std::vector<std::string> emojiAliasFiles();
  void emojiAliasFiles(const std::vector<std::string>& emojiAliasFiles);

  std::unordered_map<std::string, std::vector<std::string>> emojiAliases();

  // std::string customQssFilePath() const;
  // void customQssFilePath(const std::string& customQssFilePath);

  double windowOpacity() const;
  void windowOpacity(double windowOpacity);

  bool closeAfterFirstInput() const;
  void closeAfterFirstInput(bool closeAfterFirstInput);

  bool hideStatusBar() const;
  void hideStatusBar(bool hideStatusBar);

  bool useSystemEmojiFont() const;
  void useSystemEmojiFont(bool useSystemEmojiFont);

  bool useSystemEmojiFontWidthHeuristics() const;
  void useSystemEmojiFontWidthHeuristics(bool useSystemEmojiFontWidthHeuristics);

  int searchEditTextOffset() const;
  void searchEditTextOffset(int searchEditTextOffset);

private:
  static EmojiSettings* _snapshot;
};

class EmojiCache : public QSettings {
  Q_OBJECT

public:
  explicit EmojiCache();

  ~EmojiCache();

  std::vector<Emoji> emojiMRU();
  void emojiMRU(const std::vector<Emoji>& emojiMRU);

private:
  static QString path();
};
