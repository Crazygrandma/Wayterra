.PHONY: all setup build clean run install uninstall

PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
WAYLAND_SESSIONS_DIR = $(PREFIX)/share/wayland-sessions

all: build

setup:
	meson setup build

build:
	meson setup build --reconfigure
	ninja -C build

run: build
	./build/wayterra

install: build
	install -Dm755 build/wayterra $(DESTDIR)$(BINDIR)/wayterra
	install -Dm644 wayterra.desktop $(DESTDIR)$(WAYLAND_SESSIONS_DIR)/wayterra.desktop

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/wayterra
	rm -f $(DESTDIR)$(WAYLAND_SESSIONS_DIR)/wayterra.desktop

clean:
	rm -rf build
