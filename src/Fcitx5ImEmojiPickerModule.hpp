#pragma once

#if __has_include("fcitx/addoninstance.h")

#include <fcitx/addonfactory.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/instance.h>

FCITX_CONFIGURATION(Fcitx5ImEmojiPickerModuleConfig, fcitx::KeyListOption triggerKey{this, "TriggerKey", "Trigger Key", {fcitx::Key("Control+Alt+Shift+period")}, fcitx::KeyListConstrain()};);

class Fcitx5ImEmojiPickerModule : public fcitx::AddonInstance {
public:
  Fcitx5ImEmojiPickerModule(fcitx::Instance* instance);

  bool trigger(fcitx::InputContext* inputContext);

  void keyEvent(fcitx::KeyEvent& keyEvent);

  void activate(fcitx::InputContextEvent& event);

  void deactivate(fcitx::InputContextEvent& event);

  void reset(fcitx::InputContextEvent& event);

private:
  static constexpr char CONFIG_FILE[] = "conf/im-emoji-picker.conf";

  fcitx::Instance* _instance;
  std::vector<std::unique_ptr<fcitx::HandlerTableEntry<fcitx::EventHandler>>> _eventHandlers;

  Fcitx5ImEmojiPickerModuleConfig _config;

  bool _active = false;

  void sendCursorLocation(fcitx::InputContextEvent& event);

  void reloadConfig() override;

  const fcitx::Configuration* getConfig() const override;

  void setConfig(const fcitx::RawConfig& config) override;
};

class Fcitx5ImEmojiPickerModuleFactory : public fcitx::AddonFactory {
public:
  fcitx::AddonInstance* create(fcitx::AddonManager* manager) override;
};

#endif
