# SPDX-License-Identifier: GPL-2.0-only

CC = gcc
CXX = g++
LD = $(CXX)
CFLAGS := -Wall -Wextra -Os -ggdb3 -std=gnu11 $(CFLAGS) -I. -Wno-deprecated
CXXFLAGS := -Wall -Wextra -Os -ggdb -std=gnu++14 $(CXXFLAGS) -Ijson/include -I. -Wno-deprecated
LDFLAGS := -Os -ggdb3 $(LDFLAGS)
LIBS := -lcurl -lpthread -lTgBot -lcrypto -lssl

MUIKA := muika.bin
MUIKA_SOURCES_CPP := \
	muika/main.cpp \
	muika/Muika.cpp \
	muika/ModuleManager.cpp \
	muika/modules/jqftu/entry.cpp \
	muika/modules/jqftu/Deck.cpp \
	muika/modules/jqftu/Card.cpp \
	muika/modules/jqftu/Session.cpp \
	muika/modules/jqftu/Command.cpp \
	muika/modules/jqftu/decks/tozai_line/Deck.cpp \
	muika/modules/jqftu/decks/tozai_line/Card.cpp \
	muika/modules/jqftu/decks/jlpt_n5/Deck.cpp \
	muika/modules/jqftu/decks/jlpt_n5/Card.cpp

MUIKA_OBJECTS := $(MUIKA_SOURCES_CPP:.cpp=.o)
MUIKA_DEPENDS := $(MUIKA_SOURCES_CPP:.cpp=.d)

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
