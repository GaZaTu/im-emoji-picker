#include "EmojiCache.hpp"

template <typename T>
std::vector<T> readQSettingsArrayToStdVector(QSettings& settings, const QString& prefix, std::function<T(QSettings&)> readValue, const std::vector<T>& defaultValue = {}) {
  std::vector<T> data;

  const int size = settings.beginReadArray(prefix);
  for (int i = 0; i < size; i++) {
    settings.setArrayIndex(i);

    data.push_back(readValue(settings));
  }
  settings.endArray();

  if (data.size() == 0) {
    return defaultValue;
  }

  return data;
}

template <typename T>
void writeQSettingsArrayFromStdVector(QSettings& settings, const QString& prefix, const std::vector<T>& data, std::function<void(QSettings&, const T&)> writeValue) {
  const int size = data.size();
  settings.beginWriteArray(prefix, size);
  for (int i = 0; i < size; i++) {
    settings.setArrayIndex(i);

    writeValue(settings, data[i]);
  }
  settings.endArray();
}

std::vector<Emoji> EmojiCache::getEmojiMRU() {
  auto prefix = "emojiMRU";
  auto handler = [](QSettings& settings) -> Emoji {
    return {
      settings.value("emojiKey").toString().toStdString(),
      settings.value("emojiStr").toString().toStdString(),
    };
  };

  auto mru = readQSettingsArrayToStdVector<Emoji>(*this, prefix, handler);
  return mru;
}

void EmojiCache::setEmojiMRU(const std::vector<Emoji>& mru) {
  auto prefix = "emojiMRU";
  auto handler = [](QSettings& settings, const Emoji& emoji) -> void {
    settings.setValue("emojiKey", QString::fromStdString(emoji.name));
    settings.setValue("emojiStr", QString::fromStdString(emoji.code));
  };

  writeQSettingsArrayFromStdVector<Emoji>(*this, prefix, mru, handler);
}
