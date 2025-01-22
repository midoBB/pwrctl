# Makefile for Pwrctl

# Configuration
BUILD_DIR   ?= build
DIST_DIR    ?= dist
EXEC_NAME   := pwrctl
DESTDIR     ?= $(HOME)/.local

# Default target
all: build

# Configure project
configure:
	mkdir -p $(BUILD_DIR)
	cmake -B $(BUILD_DIR) -DCMAKE_INSTALL_PREFIX=$(DESTDIR) -DCMAKE_BUILD_TYPE=Release
	cmake -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=1

# Build project
build: configure
	cmake --build $(BUILD_DIR)
	mkdir -p $(DIST_DIR)
	cp $(BUILD_DIR)/$(EXEC_NAME) $(DIST_DIR)/

# Install to system
install: build
	mkdir -p $(DESTDIR)/bin
	cmake --install $(BUILD_DIR) --prefix $(DESTDIR)

uninstall:
	rm -rf $(DESTDIR)/bin/$(EXEC_NAME)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR)

.PHONY: all configure build install clean
