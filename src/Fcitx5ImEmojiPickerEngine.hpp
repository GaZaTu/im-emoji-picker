#pragma once

#if __has_include("fcitx/inputmethodengine.h")

#include <fcitx/addonfactory.h>
#include <fcitx/inputmethodengine.h>

class Fcitx5ImEmojiPickerEngine : public fcitx::InputMethodEngineV2 {
public:
  void keyEvent(const fcitx::InputMethodEntry& entry, fcitx::KeyEvent& keyEvent) override;

  void activate(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) override;

  void deactivate(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) override;

  void reset(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) override;

private:
  void sendCursorLocation(fcitx::InputContext* inputContext);
};

class Fcitx5ImEmojiPickerEngineFactory : public fcitx::AddonFactory {
public:
  fcitx::AddonInstance* create(fcitx::AddonManager* manager) override;
};

#endif
