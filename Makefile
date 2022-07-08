.PHONY: all
.PHONY: clean

TARGET=filter_roms
VERSION=1.2
RELEASE_NAME=FilterROMs-$(VERSION)
	
###########################################################

ifneq (,$(UNION_PLATFORM))
PLATFORM=$(UNION_PLATFORM)
endif

ifeq (,$(PLATFORM))
PLATFORM=linux
endif

###########################################################

CC = $(CROSS_COMPILE)g++
CFLAGS = -Isrc -Iinclude -DVERSION=\"$(VERSION)\" -DPLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z)
LDFLAGS = -s -lSDL -lSDL_image -lSDL_ttf -lrt -lz -lm -lsqlite3
OPTM=-O3

all: setup main zip

setup:
	mkdir -p ./build
	mkdir -p ./release

main:
	cp -R ./skeleton/. ./build
	$(CC) -o build/$(TARGET) src/main.cpp $(CFLAGS) $(LDFLAGS) $(OPTM) -ldl -rdynamic
	cd ./src/kbinput && make

zip:
	mkdir -p ./release/PAYLOAD/App/FilterROMs
	cp -R ./build/. ./release/PAYLOAD/App/FilterROMs
	cd ./release/PAYLOAD && zip -r ../$(RELEASE_NAME).zip App
	rm -rf ./release/PAYLOAD

clean:
	rm -rf ./build
	rm -rf ./release
