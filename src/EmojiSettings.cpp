#include "EmojiSettings.hpp"
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

void EmojiSettings::writeDefaultsToDisk() {
  EmojiSettings s;

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
  s.searchEditTextOffset(s.searchEditTextOffset());
}

EmojiSettings::EmojiSettings() : QSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName(), nullptr) {
}

bool EmojiSettings::skinTonesDisabled() const {
  return value("skinTonesDisabled", false).toBool();
}

void EmojiSettings::skinTonesDisabled(bool skinTonesDisabled) {
  setValue("skinTonesDisabled", skinTonesDisabled);
}

bool EmojiSettings::gendersDisabled() const {
  return value("gendersDisabled", false).toBool();
}

void EmojiSettings::gendersDisabled(bool gendersDisabled) {
  setValue("gendersDisabled", gendersDisabled);
}

// bool EmojiSettings::useSystemQtTheme() const {
//   return value("useSystemQtTheme", false).toBool();
// }

// void EmojiSettings::useSystemQtTheme(bool useSystemQtTheme) {
//   setValue("useSystemQtTheme", useSystemQtTheme);
// }

int EmojiSettings::maxEmojiVersion() const {
  return value("maxEmojiVersion", -1).toInt();
}

void EmojiSettings::maxEmojiVersion(int maxEmojiVersion) {
  setValue("maxEmojiVersion", maxEmojiVersion);
}

bool EmojiSettings::isDisabledEmoji(const Emoji& emoji, const QFontMetrics& fontMetrics) {
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

std::vector<std::string> EmojiSettings::emojiAliasFiles() {
  return readQSettingsArrayToStdVector<std::string>(*this, "emojiAliasFiles", [](QSettings& settings) -> std::string {
    return settings.value("path").toString().toStdString();
  }, defaultEmojiAliasFiles);
}

void EmojiSettings::emojiAliasFiles(const std::vector<std::string>& emojiAliasFiles) {
  writeQSettingsArrayFromStdVector<std::string>(*this, "emojiAliasFiles", emojiAliasFiles, [](QSettings& settings, const std::string& exception) -> void {
    settings.setValue("path", QString::fromStdString(exception));
  });
}

std::unordered_map<std::string, std::vector<std::string>> EmojiSettings::emojiAliases() {
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

double EmojiSettings::windowOpacity() const {
  return value("windowOpacity", 0.90).toDouble();
}

void EmojiSettings::windowOpacity(double windowOpacity) {
  setValue("windowOpacity", windowOpacity);
}

bool EmojiSettings::closeAfterFirstInput() const {
  return value("closeAfterFirstInput", false).toBool();
}

void EmojiSettings::closeAfterFirstInput(bool closeAfterFirstInput) {
  setValue("closeAfterFirstInput", closeAfterFirstInput);
}

bool EmojiSettings::hideStatusBar() const {
  return value("hideStatusBar", false).toBool();
}

void EmojiSettings::hideStatusBar(bool hideStatusBar) {
  setValue("hideStatusBar", hideStatusBar);
}

bool EmojiSettings::useSystemEmojiFont() const {
  return value("useSystemEmojiFont", false).toBool();
}

void EmojiSettings::useSystemEmojiFont(bool useSystemEmojiFont) {
  setValue("useSystemEmojiFont", useSystemEmojiFont);
}

bool EmojiSettings::useSystemEmojiFontWidthHeuristics() const {
  return value("useSystemEmojiFontWidthHeuristics", true).toBool();
}

void EmojiSettings::useSystemEmojiFontWidthHeuristics(bool useSystemEmojiFontWidthHeuristics) {
  setValue("useSystemEmojiFontWidthHeuristics", useSystemEmojiFontWidthHeuristics);
}

int EmojiSettings::searchEditTextOffset() const {
  return value("searchEditTextOffset", 0).toInt();
}

void EmojiSettings::searchEditTextOffset(int searchEditTextOffset) {
  setValue("searchEditTextOffset", searchEditTextOffset);
}

EmojiCache::EmojiCache() : QSettings(path(), QSettings::IniFormat) {
}

EmojiCache::~EmojiCache() {
  setValue("version", QCoreApplication::applicationVersion());
}

QString EmojiCache::path() {
  return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/cache.ini";
}

std::vector<Emoji> EmojiCache::emojiMRU() {
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

void EmojiCache::emojiMRU(const std::vector<Emoji>& mru) {
  auto prefix = "emojiMRU";
  auto handler = [](QSettings& settings, const Emoji& emoji) -> void {
    settings.setValue("emojiKey", QString::fromStdString(emoji.name));
    settings.setValue("emojiStr", QString::fromStdString(emoji.code));
  };

  writeQSettingsArrayFromStdVector<Emoji>(*this, prefix, mru, handler);
}
