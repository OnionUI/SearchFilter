.PHONY: build
.PHONY: clean

TARGET=kbinput
	
###########################################################

ifneq (,$(UNION_PLATFORM))
PLATFORM=$(UNION_PLATFORM)
endif

ifeq (,$(PLATFORM))
PLATFORM=linux
endif

###########################################################

CC = $(CROSS_COMPILE)g++
CFLAGS = -I../common -DPLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z)
LDFLAGS = -s -lSDL -lSDL_image -lSDL_ttf -lrt
OPTM=-O3

build:
	$(CC) -o ../../build/$(TARGET) main.cpp $(CFLAGS) $(LDFLAGS) $(OPTM) -ldl -rdynamic
clean:
	rm -f ../../build/$(TARGET)
