SIM_PATH ?= $(abspath ../../simulator)
# 获取所有的程序名，假设你的程序名存储在src目录下，以.c为后缀
NAMES ?= $(notdir $(basename $(wildcard src/*.c)))
BASE_PORT ?= $(abspath ../base-port)
ARGS ?= 
result = 0

.PHONY: all clean clean-all run

all: $(NAMES)

%: src/%.c
	@NAMES=$@ SRCS=src/$@ BASE_PORT=$(BASE_PORT) SIM_PATH=$(SIM_PATH) ARGS=$(ARGS) make $(MAKECMDGOALS) -s -f $(BASE_PORT)/Makefile
	$(eval result := $?)

ifeq ($(result),0)
	@echo $(result) >> result.txt
else
	@echo false
	@echo $(result)
endif

run: all
	@cat result.txt
	@rm -f result.txt

clean:
	@rm -rf ./build/*

clean-all:
	@rm -rf ./build/*
	@rm -rf ../base-port/build/*


## Lab1 TODO: create Makefile for functest here
# .PHONY: clean clean-all
# NAMES ?= $(notdir $(basename $(wildcard src/*.c)))
# SRCS = src/$(NAMES)
# BASE_PORT = $(abspath ../base-port)
# SIM_PATH = $(abspath "../../simulator")
# %:
# 	@echo $(NAMES)
# -include $(BASE_PORT)/Makefile

