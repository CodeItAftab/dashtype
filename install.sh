#!/usr/bin/env bash
set -euo pipefail

REPO="CodeItAftab/dashtype"
INSTALL_DIR="$HOME/.local/share/dashtype"
BIN_DIR="$HOME/.local/bin"

echo "Fetching latest dashtype release..."
ASSET_URL=$(curl -fsSL "https://api.github.com/repos/$REPO/releases/latest" \
  | grep "browser_download_url.*dashtype-linux.tar.gz" \
  | cut -d '"' -f 4)

if [ -z "$ASSET_URL" ]; then
  echo "Could not find dashtype-linux.tar.gz in the latest release." >&2
  exit 1
fi

mkdir -p "$INSTALL_DIR" "$BIN_DIR"

echo "Downloading $ASSET_URL..."
curl -fsSL "$ASSET_URL" -o /tmp/dashtype.tar.gz

echo "Extracting..."
tar -xzf /tmp/dashtype.tar.gz -C "$INSTALL_DIR" --strip-components=1
rm /tmp/dashtype.tar.gz

ln -sf "$INSTALL_DIR/dashtype" "$BIN_DIR/dashtype"

if [[ ":$PATH:" != *":$BIN_DIR:"* ]]; then
  echo "Add this to your ~/.bashrc or ~/.zshrc:"
  echo "  export PATH=\"\$HOME/.local/bin:\$PATH\""
fi

echo "Installed! Run 'dashtype' to get started."
