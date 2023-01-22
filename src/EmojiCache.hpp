#pragma once

#include <QSettings>
#include <QCoreApplication>
#include <QStandardPaths>
#include "emojis.hpp"

class EmojiCache : public QSettings {
public:
  explicit EmojiCache() : QSettings(path(), QSettings::IniFormat) {
  }

  ~EmojiCache() {
    setValue("version", QCoreApplication::applicationVersion());
  }

  std::vector<Emoji> getEmojiMRU();
  void setEmojiMRU(const std::vector<Emoji>& emojiMRU);

private:
  static QString path() {
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/cache.ini";
  }
};
