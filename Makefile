TARGET=SearchFilter
VERSION=1.2.3
	
###########################################################

RELEASE_NAME=$(TARGET)-$(VERSION)
ROOT_DIR := $(shell pwd -P)
BUILD_DIR := $(ROOT_DIR)/build/App/$(TARGET)
TOOLCHAIN := ghcr.io/onionui/miyoomini-toolchain

###########################################################

.PHONY: all setup build package release clean git-clean with-toolchain

all: clean setup build package

setup:
	@mkdir -p $(BUILD_DIR)
	@cp -R ./src/static/build/. ./build
	@cp -R ./lib/. $(BUILD_DIR)/lib
	@rm -f $(BUILD_DIR)/lib/libsqlite3.so

build:
	@echo :: $(TARGET) - building $(BUILD_DIR)
	cd ./src/filter && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	cd ./src/search && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	cd ./src/tools && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
#   cd ./src/kbinput && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make

package:
	@echo :: $(TARGET) - package
	@mkdir -p ./package
	@cp -R ./src/static/package/. ./package
	@cd ./build && zip -rq ../package/App/$(TARGET)/PACKAGE.zip App Emu
	@sed -i "s/{VERSION}/$(VERSION)/g" ./package/App/$(TARGET)/config.json

release: all
	@echo :: $(TARGET) - release
	@mkdir -p ./release
	@cd ./package && zip -rq ../release/$(RELEASE_NAME).zip App

clean:
	@rm -rf ./build
	@rm -rf ./package
	@rm -f ./release/$(RELEASE_NAME).zip
	@echo :: $(TARGET) - cleaned

git-clean:
	@git clean -xfd -e .vscode

with-toolchain:
	docker pull $(TOOLCHAIN)
	docker run --rm -v "$(ROOT_DIR)":/root/workspace $(TOOLCHAIN) /bin/bash -c "source /root/.bashrc; make $(CMD)"
