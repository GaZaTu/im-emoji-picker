#include "EmojiPickerSettings.hpp"
#include "EmojiLabel.hpp"
#include <QCoreApplication>
#include <QStandardPaths>

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

void EmojiPickerSettings::writeDefaultsToDisk() {
  EmojiPickerSettings s;

  s.skinTonesDisabled(s.skinTonesDisabled());
  s.gendersDisabled(s.gendersDisabled());
  // s.useSystemQtTheme(s.useSystemQtTheme());
  s.maxEmojiVersion(s.maxEmojiVersion());
  s.emojiAliasFiles(s.emojiAliasFiles());
  // s.customQssFilePath(s.customQssFilePath());
  s.windowOpacity(s.windowOpacity());
  s.closeAfterFirstInput(s.closeAfterFirstInput());
  s.hideStatusBar(s.hideStatusBar());
  s.useSystemEmojiFont(s.useSystemEmojiFont());
  s.useSystemEmojiFontWidthHeuristics(s.useSystemEmojiFontWidthHeuristics());
}

EmojiPickerSettings::EmojiPickerSettings() : QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), nullptr) {
}

bool EmojiPickerSettings::skinTonesDisabled() const {
  return value("skinTonesDisabled", false).toBool();
}

void EmojiPickerSettings::skinTonesDisabled(bool skinTonesDisabled) {
  setValue("skinTonesDisabled", skinTonesDisabled);
}

bool EmojiPickerSettings::gendersDisabled() const {
  return value("gendersDisabled", false).toBool();
}

void EmojiPickerSettings::gendersDisabled(bool gendersDisabled) {
  setValue("gendersDisabled", gendersDisabled);
}

// bool EmojiSettings::useSystemQtTheme() const {
//   return value("useSystemQtTheme", false).toBool();
// }

// void EmojiSettings::useSystemQtTheme(bool useSystemQtTheme) {
//   setValue("useSystemQtTheme", useSystemQtTheme);
// }

int EmojiPickerSettings::maxEmojiVersion() const {
  return value("maxEmojiVersion", -1).toInt();
}

void EmojiPickerSettings::maxEmojiVersion(int maxEmojiVersion) {
  setValue("maxEmojiVersion", maxEmojiVersion);
}

bool EmojiPickerSettings::isDisabledEmoji(const Emoji& emoji, const QFontMetrics& fontMetrics) {
  if (maxEmojiVersion() != -1 && (emoji.version > maxEmojiVersion())) {
    return true;
  }

  if (skinTonesDisabled() && emoji.isSkinToneVariation()) {
    return true;
  }

  if (gendersDisabled() && emoji.isGenderVariation()) {
    return true;
  }

  if (useSystemEmojiFont() && useSystemEmojiFontWidthHeuristics()) {
    if (!fontSupportsEmoji(fontMetrics, QString::fromStdString(emoji.code))) {
      return true;
    }
  }

  return false;
}

std::vector<std::string> defaultEmojiAliasFiles = {
  ":/res/aliases/github-emojis.ini",
  ":/res/aliases/gitmoji-emojis.ini",
};

std::vector<std::string> EmojiPickerSettings::emojiAliasFiles() {
  return readQSettingsArrayToStdVector<std::string>(*this, "emojiAliasFiles", [](QSettings& settings) -> std::string {
    return settings.value("path").toString().toStdString();
  }, defaultEmojiAliasFiles);
}

void EmojiPickerSettings::emojiAliasFiles(const std::vector<std::string>& emojiAliasFiles) {
  writeQSettingsArrayFromStdVector<std::string>(*this, "emojiAliasFiles", emojiAliasFiles, [](QSettings& settings, const std::string& exception) -> void {
    settings.setValue("path", QString::fromStdString(exception));
  });
}

std::unordered_map<std::string, std::vector<std::string>> EmojiPickerSettings::emojiAliases() {
  std::unordered_map<std::string, std::vector<std::string>> result;

  for (const std::string& path : emojiAliasFiles()) {
    QSettings emojiAliasesIni{QString::fromStdString(path), QSettings::IniFormat};

    int arraySize = emojiAliasesIni.beginReadArray("AliasesList");
    for (int i = 0; i < arraySize; i++) {
      emojiAliasesIni.setArrayIndex(i);

      std::string alias = emojiAliasesIni.value("alias").toString().toStdString();
      std::string value = emojiAliasesIni.value("value").toString().toStdString();

      result[value].push_back(alias);
    }
    emojiAliasesIni.endArray();
  }

  return result;
}

// std::string EmojiSettings::customQssFilePath() const {
//   return value("customQssFilePath", "").toString().toStdString();
// }

// void EmojiSettings::customQssFilePath(const std::string& customQssFilePath) {
//   setValue("customQssFilePath", QString::fromStdString(customQssFilePath));
// }

double EmojiPickerSettings::windowOpacity() const {
  return value("windowOpacity", 0.90).toDouble();
}

void EmojiPickerSettings::windowOpacity(double windowOpacity) {
  setValue("windowOpacity", windowOpacity);
}

bool EmojiPickerSettings::closeAfterFirstInput() const {
  return value("closeAfterFirstInput", false).toBool();
}

void EmojiPickerSettings::closeAfterFirstInput(bool closeAfterFirstInput) {
  setValue("closeAfterFirstInput", closeAfterFirstInput);
}

bool EmojiPickerSettings::hideStatusBar() const {
  return value("hideStatusBar", false).toBool();
}

void EmojiPickerSettings::hideStatusBar(bool hideStatusBar) {
  setValue("hideStatusBar", hideStatusBar);
}

bool EmojiPickerSettings::useSystemEmojiFont() const {
  return value("useSystemEmojiFont", false).toBool();
}

void EmojiPickerSettings::useSystemEmojiFont(bool useSystemEmojiFont) {
  setValue("useSystemEmojiFont", useSystemEmojiFont);
}

bool EmojiPickerSettings::useSystemEmojiFontWidthHeuristics() const {
  return value("useSystemEmojiFontWidthHeuristics", true).toBool();
}

void EmojiPickerSettings::useSystemEmojiFontWidthHeuristics(bool useSystemEmojiFontWidthHeuristics) {
  setValue("useSystemEmojiFontWidthHeuristics", useSystemEmojiFontWidthHeuristics);
}

EmojiPickerCache::EmojiPickerCache() : QSettings(path(), QSettings::IniFormat) {
}

EmojiPickerCache::~EmojiPickerCache() {
  setValue("version", QCoreApplication::applicationVersion());
}

QString EmojiPickerCache::path() {
  return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/cache.ini";
}

std::vector<Emoji> EmojiPickerCache::emojiMRU() {
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

void EmojiPickerCache::emojiMRU(const std::vector<Emoji>& mru) {
  auto prefix = "emojiMRU";
  auto handler = [](QSettings& settings, const Emoji& emoji) -> void {
    settings.setValue("emojiKey", QString::fromStdString(emoji.name));
    settings.setValue("emojiStr", QString::fromStdString(emoji.code));
  };

  writeQSettingsArrayFromStdVector<Emoji>(*this, prefix, mru, handler);
}
