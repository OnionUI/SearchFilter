.PHONY: all
.PHONY: clean

TARGET=filter_roms
VERSION=1.0.1
RELEASE_NAME=SearchFilter-$(VERSION)
BUILD_DIR := $(shell pwd -P)/build/App/SearchFilter
	
###########################################################

ifneq (,$(UNION_PLATFORM))
PLATFORM=$(UNION_PLATFORM)
endif

ifeq (,$(PLATFORM))
PLATFORM=linux
endif

###########################################################

all: setup main

release: clean setup main zip

setup:
	mkdir -p ./build

main:
	@echo BUILD_DIR $(BUILD_DIR)
	cp -R ./skeleton/. ./build
	cd ./src/filter && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	cd ./src/search && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	# cd ./src/kbinput && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make

zip:
	mkdir -p ./release
	cd ./build && zip -r ../release/$(RELEASE_NAME).zip App Emu

clean:
	rm -rf ./build
	rm -rf ./release
