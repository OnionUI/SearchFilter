TARGET=SearchFilter
VERSION=1.2.3
	
###########################################################

RELEASE_NAME=$(TARGET)-$(VERSION)
ROOT_DIR := $(shell pwd -P)
BUILD_DIR := $(ROOT_DIR)/build/App/$(TARGET)
TOOLCHAIN := ghcr.io/onionui/miyoomini-toolchain

###########################################################

.PHONY: all release clean git-clean with-toolchain

all: clean build

.setup:
	@mkdir -p $(BUILD_DIR)
	@cp -R ./src/static/build/. ./build
	@cp -R ./lib/. $(BUILD_DIR)/lib
	@rm -f $(BUILD_DIR)/lib/libsqlite3.so
	@touch .setup

build: .setup
	@echo :: $(TARGET) - building $(BUILD_DIR)
#	cd ./src/filter && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	cd ./src/search && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	cd ./src/tools && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
#   cd ./src/kbinput && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make

release: build
	@echo :: $(TARGET) - release
	@mkdir -p ./release
	@rm -f ./release/$(RELEASE_NAME).zip
	@cd ./build && zip -rq ../release/$(RELEASE_NAME).zip App

clean:
	@rm -rf .setup
	@rm -rf ./build
	@echo :: $(TARGET) - cleaned

git-clean:
	@git clean -xfd -e .vscode

with-toolchain:
	docker pull $(TOOLCHAIN)
	docker run --rm -v "$(ROOT_DIR)":/root/workspace $(TOOLCHAIN) /bin/bash -c "source /root/.bashrc; make $(CMD)"
