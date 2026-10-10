# Redstone for Omarchy

Qt Quick desktop client for [redstone-server](https://github.com). Same snapshot the iOS app uses.

## Build

```bash
./bin/build
./build/redstone
```

Needs Qt 6 (`qt6-base`, `qt6-declarative`) and `xdg-desktop-portal`.

## Install (user)

```bash
./bin/build
install -Dm755 build/redstone ~/.local/bin/redstone
install -Dm644 packaging/redstone.desktop ~/.local/share/applications/redstone.desktop
install -Dm644 packaging/redstone.svg ~/.local/share/icons/hicolor/scalable/apps/redstone.svg
update-desktop-database ~/.local/share/applications >/dev/null 2>&1 || true
```

## Theme

Reads `~/.local/state/omarchy/current/theme/colors.toml` and reloads when you switch Omarchy themes, same pattern as omacalc.
