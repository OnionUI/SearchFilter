.PHONY: build
.PHONY: clean

TARGET=tools
	
###########################################################

ifeq (,$(BUILD_DIR))
BUILD_DIR=.
endif

ifneq (,$(UNION_PLATFORM))
PLATFORM=$(UNION_PLATFORM)
endif

ifeq (,$(PLATFORM))
PLATFORM=linux
endif

###########################################################

CC = $(CROSS_COMPILE)g++
CFLAGS = -I../../include -I../common -DVERSION=\"$(VERSION)\" -DPLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z)
LDFLAGS = -s -lSDL -lSDL_image -lSDL_ttf -lrt -lz -lm
OPTM=-O3

build:
	$(CC) -o $(BUILD_DIR)/$(TARGET) main.cpp ../../include/jsoncpp.cpp $(CFLAGS) $(LDFLAGS) $(OPTM) -ldl -rdynamic -Wno-psabi
clean:
	rm -f $(BUILD_DIR)/$(TARGET)
