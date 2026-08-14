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

# Create installation directories
mkdir -p "$INSTALL_DIR" "$BIN_DIR"

echo "Downloading $ASSET_URL..."
curl -fsSL "$ASSET_URL" -o /tmp/dashtype.tar.gz

echo "Extracting..."

# Clean previous installation
rm -rf "$INSTALL_DIR"/*

tar -xzf /tmp/dashtype.tar.gz \
  -C "$INSTALL_DIR" \
  --strip-components=1

rm /tmp/dashtype.tar.gz

# Verify executable exists
if [ ! -f "$INSTALL_DIR/dashtype" ]; then
  echo "Installation failed: dashtype executable was not found." >&2
  exit 1
fi

chmod +x "$INSTALL_DIR/dashtype"

# Create/update symlink
ln -sf "$INSTALL_DIR/dashtype" "$BIN_DIR/dashtype"

# Add ~/.local/bin to PATH permanently
PATH_LINE='export PATH="$HOME/.local/bin:$PATH"'

add_to_shell_config() {
  local config_file="$1"

  if [ -f "$config_file" ]; then
    if ! grep -Fxq "$PATH_LINE" "$config_file"; then
      echo "" >> "$config_file"
      echo "# dashtype" >> "$config_file"
      echo "$PATH_LINE" >> "$config_file"

      echo "Added ~/.local/bin to $config_file"
    fi
  fi
}

add_to_shell_config "$HOME/.bashrc"
add_to_shell_config "$HOME/.zshrc"

# Make dashtype available in the current shell immediately
export PATH="$BIN_DIR:$PATH"

echo ""
echo "Dashtype installed successfully!"
echo "Executable: $BIN_DIR/dashtype"
echo ""

# Verify command works
if command -v dashtype >/dev/null 2>&1; then
  echo "You can now run:"
  echo "  dashtype"
else
  echo "Installation completed, but dashtype was not found in PATH."
  echo "Restart your terminal and run:"
  echo "  dashtype"
fi