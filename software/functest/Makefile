# 获取所有的程序名，假设你的程序名存储在src目录下，以.c为后缀
NAMES ?= $(notdir $(basename $(wildcard src/*.c)))
BASE_PORT = $(abspath ../base-port)

.PHONY: all clean clean-all

all: $(NAMES)

%: src/%.c
	@NAMES=$@ SRCS=src/$@ BASE_PORT=$(BASE_PORT) make -s -f $(BASE_PORT)/Makefile

clean:
	@rm -rf ./build/*

clean-all:
	@rm -rf ./build/*
	@rm -rf ../base-port/build/*