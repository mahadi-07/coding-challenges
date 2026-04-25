#!/usr/bin/env bash

INSTALL_PATH="/usr/local/bin/ccwc"
SCRIPT_PATH="$(cd "$(dirname "$0")" && pwd)/ccwc.sh"

case "$1" in
  up|install)
    if [ ! -f "$SCRIPT_PATH" ]; then
      echo "ccwc.sh not found in current directory"
      exit 1
    fi

    chmod +x "$SCRIPT_PATH"
    sudo cp "$SCRIPT_PATH" "$INSTALL_PATH"
    sudo chmod +x "$INSTALL_PATH"

    echo "ccwc installed successfully at $INSTALL_PATH"
    ;;

  down|uninstall)
    if [ -f "$INSTALL_PATH" ]; then
      sudo rm -f "$INSTALL_PATH"
      echo "ccwc removed from system"
    else
      echo "ccwc is not installed"
    fi
    ;;

  *)
    echo "Usage:"
    echo "  ./prepare.sh up      # install ccwc"
    echo "  ./prepare.sh down    # uninstall ccwc"
    ;;
esac