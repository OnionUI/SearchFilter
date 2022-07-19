.PHONY: all setup build package release clean

TARGET=SearchFilter
VERSION=1.3
	
###########################################################

RELEASE_NAME=$(TARGET)-$(VERSION)
BUILD_DIR := $(shell pwd -P)/build/App/$(TARGET)

###########################################################

all: clean setup build package

setup:
	@mkdir -p $(BUILD_DIR)
	@cp -R ./src/static/build/. ./build

build:
	@echo :: building $(BUILD_DIR)
	cd ./src/filter && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	cd ./src/search && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	cd ./src/tools && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	# cd ./src/kbinput && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make

package:
	@echo :: package
	@mkdir -p ./package
	@cp -R ./src/static/package/. ./package
	@cd ./build && zip -rq ../package/App/$(TARGET)/PACKAGE.zip App Emu
	@sed -i "s/{VERSION}/$(VERSION)/g" ./package/App/$(TARGET)/config.json

release: all
	@echo :: release
	@mkdir -p ./release
	@cd ./package && zip -rq ../release/$(RELEASE_NAME).zip App

clean:
	@rm -rf ./build
	@rm -rf ./package
	@rm -f ./release/$(RELEASE_NAME).zip
	@echo :: cleaned
