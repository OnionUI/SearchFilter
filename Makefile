TARGET=SearchFilter
VERSION=1.4
	
###########################################################

RELEASE_NAME=$(TARGET)-$(VERSION)
ROOT_DIR := $(shell pwd -P)
BUILD_DIR := $(ROOT_DIR)/build/.tmp_update/bin
LIB_DIR := $(ROOT_DIR)/build/.tmp_update/lib
TOOLCHAIN := aemiii91/miyoomini-toolchain:latest

ifdef OS
	current_dir := $(shell cd)
	ROOT_DIR := $(subst \,/,$(current_dir))
	makedir := mkdir
	createfile := echo.>
else
	ROOT_DIR := $(shell pwd)
	makedir := mkdir -p
	createfile := touch
endif

###########################################################

.PHONY: all release clean git-clean with-toolchain

all: build
dev: clean build

.setup:
	@mkdir -p $(BUILD_DIR)
	@cp -R ./src/static/build/. ./build
	@cp -R ./lib/. $(LIB_DIR)
	@rm -f $(LIB_DIR)/libsqlite3.so
	@touch .setup

build: .setup
	@echo :: $(TARGET) - building $(BUILD_DIR)
	cd ./src/filter && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	cd ./src/search && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	cd ./src/tools && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make
	cd ./src/kbinput && BUILD_DIR=$(BUILD_DIR) VERSION=$(VERSION) make

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

cache/.docker:
	docker pull $(TOOLCHAIN)
	$(makedir) cache
	$(createfile) cache/.docker

toolchain: cache/.docker
	docker run -it --rm -v "$(ROOT_DIR)":/root/workspace $(TOOLCHAIN) /bin/bash

with-toolchain: cache/.docker
	docker run --rm -v "$(ROOT_DIR)":/root/workspace $(TOOLCHAIN) /bin/bash -c "source /root/.bashrc; make $(CMD)"
