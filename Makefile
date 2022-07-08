.PHONY: build
.PHONY: clean

TARGET=filter_roms
	
###########################################################

ifneq (,$(UNION_PLATFORM))
PLATFORM=$(UNION_PLATFORM)
endif

ifeq (,$(PLATFORM))
PLATFORM=linux
endif

###########################################################

CC = $(CROSS_COMPILE)g++
CFLAGS = -Isrc -Iinclude -DPLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z)
LDFLAGS = -s -lSDL -lSDL_image -lSDL_ttf -lrt -lz -lm -lsqlite3
OPTM=-O3

build:
	mkdir -p build
	cp -R ./skeleton/. ./build
	$(CC) -o build/$(TARGET) src/main.cpp $(CFLAGS) $(LDFLAGS) $(OPTM) -ldl -rdynamic
	cd ./src/kbinput && make
clean:
	rm -rf ./build
