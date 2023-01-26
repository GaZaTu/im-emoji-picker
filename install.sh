#!/bin/sh

PROJECT_OWNER="GaZaTu"
PROJECT_REPO="im-emoji-picker"
PROJECT_NAME="I'm Emoji Picker"

RELEASE="latest"
FRAMEWORK=""

while getopts ":r:f:" flag
do
  case $flag in
    r) RELEASE="$OPTARG";;
    f) FRAMEWORK="$OPTARG";;
  esac
done

if [ -z "$FRAMEWORK" ];
then
  if command -v ibus > /dev/null;
  then
    FRAMEWORK="ibus"
  fi
  if command -v fcitx5 > /dev/null;
  then
    FRAMEWORK="fcitx5"
  fi
fi

if [ -z "$FRAMEWORK" ];
then
  echo "no input method framework found/specified (example: `sh install.sh -f fcitx5`)"
  exit 1
fi

echo "Installing `$PROJECT_NAME` release: `$RELEASE`..."

DISTRO_ID=$(grep "^ID=" "/etc/os-release" | sed "s/ID=//" | sed "s/\"//g")
DISTRO_VERSION_ID=$(grep "^VERSION_ID=" "/etc/os-release" | sed "s/VERSION_ID=//" | sed "s/\"//g")

PACKAGE_MANAGER=$(
  case "$DISTRO_ID" in
    "ubuntu"* | "debian"*) echo "apt";;
    "opensuse"*) echo "zypper";;
    "fedora"*) echo "dnf";;
  esac
)
PACKAGE_MANAGER_INSTALL_COMMAND=$(
  case "$PACKAGE_MANAGER" in
    "apt") echo "install -y";;
    "zypper") echo "install -y --allow-unsigned-rpm";;
    "dnf") echo "install -y --nogpgcheck";;
  esac
)

if ! command -v jq > /dev/null;
then
  if [ -z "$PACKAGE_MANAGER" ];
  then
    echo "jq not found, and no known package manager found to install it either"
    exit 1
  fi

  echo ""
  echo "jq not found, installing through package manager..."
  echo "> sudo $PACKAGE_MANAGER $PACKAGE_MANAGER_INSTALL_COMMAND jq"
  sudo $PACKAGE_MANAGER $PACKAGE_MANAGER_INSTALL_COMMAND jq
fi

echo ""
echo "Check github for release artifacts..."
RELEASE_ID=$(echo $RELEASE | sed "s/\//-/")
RELEASE_JSON="/tmp/$PROJECT_REPO-$RELEASE_ID.json"
wget -nv "https://api.github.com/repos/$PROJECT_OWNER/$PROJECT_REPO/releases/$RELEASE" -O "$RELEASE_JSON"

echo ""
echo "Find artifact for $DISTRO_ID $DISTRO_VERSION_ID"
URL=$(jq -r ".assets | .[] | .browser_download_url | select(. | (test(\"$DISTRO_ID\") and test(\"$DISTRO_VERSION_ID\") and test(\"$FRAMEWORK\")))" "$RELEASE_JSON")
ARTIFACT_IS_APPIMAGE=false

# if [ -z "$URL" ];
# then
#   echo "Matching artifact not found; Find .AppImage instead"
#   URL=$(jq -r ".assets | .[] | .browser_download_url | select(. | (test(\".AppImage\")))" "$RELEASE_JSON")
#   ARTIFACT_IS_APPIMAGE=true
# fi

if [ -z "$URL" ];
then
  echo "No artifact found; either something went wrong or your system is not supported"
  exit 1
fi

ARTIFACT_NAME=$(basename "$URL")
ARTIFACT_PATH="/tmp/$ARTIFACT_NAME"

# EXECUTABLE_NAME="emoji-picker"

# APPIMAGE_NAME="$EXECUTABLE_NAME.AppImage"
# APPIMAGE_INSTALL_PATH="/usr/bin/$APPIMAGE_NAME"

echo ""
echo "Downloading artifact..."
wget -q --show-progress "$URL" -O "$ARTIFACT_PATH"

if $ARTIFACT_IS_APPIMAGE
then
  # if command -v $EXECUTABLE_NAME > /dev/null;
  # then
  #   echo ""
  #   echo "Abort: do not override $EXECUTABLE_NAME with .AppImage"
  #   exit 2
  # fi

  # EXECUTABLE_NAME="$APPIMAGE_NAME"

  # echo ""
  # echo "Move artifact and make it executable"
  # echo "> sudo cp $ARTIFACT_PATH $APPIMAGE_INSTALL_PATH"
  # sudo cp "$ARTIFACT_PATH" "$APPIMAGE_INSTALL_PATH"
  # echo "> sudo chmod +x $APPIMAGE_INSTALL_PATH"
  # sudo chmod +x "$APPIMAGE_INSTALL_PATH"
else
  # if command -v $APPIMAGE_NAME > /dev/null;
  # then
  #   echo ""
  #   echo "Remove existing .AppImage"
  #   echo "> sudo rm $APPIMAGE_INSTALL_PATH"
  #   sudo rm "$APPIMAGE_INSTALL_PATH"
  # fi

  echo ""
  echo "Install artifact using package manager"
  echo "> sudo $PACKAGE_MANAGER $PACKAGE_MANAGER_INSTALL_COMMAND $ARTIFACT_PATH"
  sudo $PACKAGE_MANAGER $PACKAGE_MANAGER_INSTALL_COMMAND "$ARTIFACT_PATH"
fi

echo ""
echo "`$PROJECT_NAME` installed! 🎉"
