.PHONY: all
.PHONY: clean

TARGET=SearchFilter
VERSION=1.2.1
	
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

release: clean setup main package

setup:
	mkdir -p ./build

main:
	@echo BUILD_DIR $(BUILD_DIR)
	cp -R ./skeleton/. ./build
	rm -f $(BUILD_DIR)/config.json
	rm -f $(BUILD_DIR)/install.sh
	cd ./src/filter && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	cd ./src/search && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	cd ./src/tools && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	# cd ./src/kbinput && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make

package:
	rm -rf ./package
	mkdir -p ./package/App/$(TARGET)
	cd ./build && zip -r ../package/App/$(TARGET)/PACKAGE.zip App Emu
	cp ./skeleton/App/$(TARGET)/install.sh ./package/App/$(TARGET)
	cp ./skeleton/App/$(TARGET)/config.json ./package/App/$(TARGET)
	cp ./skeleton/App/$(TARGET)/res/icon_installer.png ./package/App/$(TARGET)/icon.png
	sed -i "s/{VERSION}/$(VERSION)/g" ./package/App/$(TARGET)/config.json
	mkdir -p ./release
	cd ./package && zip -r ../release/$(RELEASE_NAME).zip App

clean:
	rm -rf ./build
	rm -rf ./package
	rm -f ./release/$(RELEASE_NAME).zip
