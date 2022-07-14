.PHONY: all
.PHONY: clean

TARGET=SearchFilter
VERSION=1.1
	
###########################################################

RELEASE_NAME=$(TARGET)-$(VERSION)
BUILD_DIR := $(shell pwd -P)/build/App/$(TARGET)

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
	# sed -i "s/{VERSION}/$(VERSION)/g" $(BUILD_DIR)/config.json
	cd ./src/filter && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	cd ./src/search && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	cd ./src/tools && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	# cd ./src/kbinput && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make

zip:
	mkdir -p ./release
	cd ./build && zip -r ../release/$(RELEASE_NAME).zip App Emu

clean:
	rm -rf ./build
	rm -f ./release/$(RELEASE_NAME).zip
