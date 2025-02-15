.PHONY: all build run

all: build run

build:
	gcc main.c src/*.c src/**/*.c -o sea_battle -Iinclude -ggdb3

run:
	./sea_battle 2> err.log

clear:
	rm sea_battle err.log
