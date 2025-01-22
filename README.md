# Pwrctl

## Description

This is a simple GUI application to control the power management of the system.

## Requirements

```terminal
sudo apt install -y \
    qt6-base \
    qt6-wayland \
    libxcb1 \
    libx11-6 \
    libgl1
```

## Building

```terminal
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    git

# Qt6 development packages
sudo apt install -y \
    qt6-base-dev \
    qt6-base-private-dev \
    qt6-tools-dev \
    qt6-wayland \
    libgl1-mesa-dev

# X11/Wayland dependencies
sudo apt install -y \
    libx11-dev \
    libxcb1-dev \
    libxkbcommon-dev \
    libwayland-dev
```
