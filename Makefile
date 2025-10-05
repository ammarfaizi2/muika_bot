# SPDX-License-Identifier: GPL-2.0-only

CC = gcc
CXX = g++
LD = $(CXX)
CFLAGS := -Wall -Wextra -Os -ggdb3 -std=gnu11 $(CFLAGS) -I. -Wno-deprecated -fvisibility=hidden -DHAVE_CURL
CXXFLAGS := -Wall -Wextra -Os -ggdb -std=gnu++14 $(CXXFLAGS) -Ijson/include -I. -Wno-deprecated -fvisibility=hidden -DHAVE_CURL
LDFLAGS := -Os -ggdb3 $(LDFLAGS)
LIBS := -lpthread -lTgBot -lcrypto -lssl -lcurl

MUIKA := muika.bin
MUIKA_SOURCES_CPP := \
	muika/helpers.cpp \
	muika/main.cpp \
	muika/Muika/Modules/Jqftu/Worker.cpp \
	muika/Muika/Modules/Jqftu/ModJqftu.cpp \
	muika/Muika/Module.cpp \
	muika/Muika/ModuleManager.cpp \
	muika/Muika/MuikaBot.cpp

MUIKA_OBJECTS := $(MUIKA_SOURCES_CPP:.cpp=.o)
MUIKA_DEPENDS := $(MUIKA_SOURCES_CPP:.cpp=.d)

ifeq ($(ENABLE_SANITIZER),1)
	CFLAGS += -fsanitize=address
	CXXFLAGS += -fsanitize=address
	LDFLAGS += -fsanitize=address
endif

ifeq ($(ENABLE_LTO),1)
	CFLAGS += -flto
	CXXFLAGS += -flto
	LDFLAGS += -flto
endif

all: $(MUIKA)

$(MUIKA): $(MUIKA_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

clean:
	rm -f $(MUIKA) $(MUIKA_OBJECTS) $(MUIKA_DEPENDS)

-include $(MUIKA_DEPENDS)

.PHONY: all clean
