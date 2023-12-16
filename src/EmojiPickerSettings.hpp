#pragma once

#include "emojis.hpp"
#include <QFontMetrics>
#include <QSettings>
#include <unordered_map>
#include <utility>
#include <vector>

class EmojiPickerSettings : public QSettings {
  Q_OBJECT

public:
  static void writeDefaultsToDisk();

  explicit EmojiPickerSettings();

  bool skinTonesDisabled() const;
  void skinTonesDisabled(bool skinTonesDisabled);

  bool gendersDisabled() const;
  void gendersDisabled(bool gendersDisabled);

  bool useSystemQtTheme() const;
  void useSystemQtTheme(bool useSystemQtTheme);

  int maxEmojiVersion() const;
  void maxEmojiVersion(int maxEmojiVersion);

  bool isDisabledEmoji(const Emoji& emoji, const QFontMetrics& fontMetrics);

  std::vector<std::string> emojiAliasFiles();
  void emojiAliasFiles(const std::vector<std::string>& emojiAliasFiles);

  std::unordered_map<std::string, std::vector<QString>> emojiAliases();

  // std::string customQssFilePath() const;
  // void customQssFilePath(const std::string& customQssFilePath);

  double windowOpacity() const;
  void windowOpacity(double windowOpacity);

  bool closeAfterFirstInput() const;
  void closeAfterFirstInput(bool closeAfterFirstInput);

  bool useSystemEmojiFont() const;
  void useSystemEmojiFont(bool useSystemEmojiFont);

  bool useSystemEmojiFontWidthHeuristics() const;
  void useSystemEmojiFontWidthHeuristics(bool useSystemEmojiFontWidthHeuristics);

  std::string systemEmojiFontOverride() const;
  void systemEmojiFontOverride(const std::string& systemEmojiFontOverride);

  std::string scaleFactor() const;
  void scaleFactor(const std::string& scaleFactor);

  bool saveKaomojiInMRU() const;
  void saveKaomojiInMRU(bool saveKaomojiInMRU);

  std::unordered_map<char, QKeySequence> customHotKeys();
  void customHotKeys(const std::unordered_map<char, QKeySequence>& customHotKeys);
};

class EmojiPickerCache : public QSettings {
  Q_OBJECT

public:
  explicit EmojiPickerCache();

  ~EmojiPickerCache();

  std::vector<Emoji> emojiMRU();
  void emojiMRU(const std::vector<Emoji>& emojiMRU);

private:
  static QString path();
};
