.PHONY: all build run

all: build run

build:
	gcc test.c src/*.c -o asd -Iinclude -ggdb3

run:
	./asd 2> err.log

clear:
	rm asd
