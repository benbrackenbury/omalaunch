# Omalaunch

SpaceX launches on the Linux desktop. Omalaunch is a Qt Quick client for [Omarchy](https://omarchy.org/) that reads the same snapshot as the iOS Redstone app.

It follows the current Omarchy theme (including live switches) and is meant to be driven from the keyboard, in the same spirit as omacalc and omamail.

## What it shows

Upcoming, in-progress, and past SpaceX missions from redstone-server. Follow a launch, open the webcast or the SpaceX page, and (when SpaceX is flying Starship) poll the public tracker for vehicle telemetry. Stats and Starbase road closures live on their own pages. Non-SpaceX launches can be turned on in settings.

Data comes from one unauthenticated GET: `/v1/snapshot`. The default server URL is the same one the iOS app uses. You can point it at another host in settings.

## Keyboard

Hold Ctrl to see numbered sidebar slots, then `Ctrl+1`… to jump. `?` opens the full list.

| Key | Action |
| --- | --- |
| `j` / `k` | Move |
| `g` / `G` | First / last |
| `o` / Enter | Open |
| `u` / Esc | Back |
| `f` | Follow |
| `w` | Webcast |
| `s` | SpaceX page |
| `/` | Search |
| `1`–`5` | Mission filters |
| `r` | Refresh |
| `?` | Shortcuts |

While a mission is open, `j`/`k` walk the list and open the next one. `Shift+J` / `Shift+K` scroll the page. Search and settings keep letters as typing. Esc leaves the field.

## Build

Needs Qt 6 (`qt6-base`, `qt6-declarative`, `qt6-svg`) and `xdg-desktop-portal`.

```bash
./bin/build
./build/omalaunch
```

## Install

```bash
./bin/build
install -Dm755 build/omalaunch ~/.local/bin/omalaunch
install -Dm644 packaging/omalaunch.desktop ~/.local/share/applications/omalaunch.desktop
install -Dm644 packaging/omalaunch.svg ~/.local/share/icons/hicolor/scalable/apps/omalaunch.svg
install -Dm644 packaging/omalaunch.png ~/.local/share/icons/hicolor/256x256/apps/omalaunch.png
update-desktop-database ~/.local/share/applications
```

There is a `PKGBUILD` if you would rather make a package.

## Theme

Colors come from `~/.local/state/omarchy/current/theme/colors.toml`. The file is watched, so changing themes in Omarchy updates the window without a restart. Dark mode and text scale follow the desktop portal, same as omacalc.

## Icon

International Orange (`#FF4F00`) circle on white, matching the iOS app icon.
