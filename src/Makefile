.PHONY: all setup build clean run

all: build

setup:
	meson setup build

build:
	meson setup build --reconfigure
	ninja -C build

run: build
	./build/wayterra

clean:
	rm -rf build
