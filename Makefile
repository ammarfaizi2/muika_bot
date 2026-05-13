# SPDX-License-Identifier: GPL-2.0-only

CC = gcc
CXX = g++
LD = $(CXX)
CFLAGS := -Wall -Wextra -Os -ggdb3 -std=gnu11 -I. -Wno-deprecated
CXXFLAGS := -Wall -Wextra -Os -ggdb3 -std=gnu++17 -I./muika/json/include -I. -Wno-deprecated
LDFLAGS := -Os -ggdb3

#
# Base library for Muika bot.
#
LIBMUIKABOT := libmuikabot.so
SOURCES_LIBMUIKABOT := \
	muika/modules/m001_hello/Module.cpp \
	muika/modules/m002_jqftu/Card.cpp \
	muika/modules/m002_jqftu/Deck.cpp \
	muika/modules/m002_jqftu/DeckGroup.cpp \
	muika/modules/m002_jqftu/Module.cpp \
	muika/modules/m002_jqftu/Point.cpp \
	muika/modules/m002_jqftu/Session.cpp \
	muika/helpers.cpp \
	muika/Message.cpp \
	muika/Module.cpp \
	muika/Muika.cpp \
	muika/Reactor.cpp
OBJECTS_LIBMUIKABOT := $(SOURCES_LIBMUIKABOT:.cpp=.o)
DEPENDS_LIBMUIKABOT := $(SOURCES_LIBMUIKABOT:.cpp=.d)
CFLAGS_LIBMUIKABOT := $(CFLAGS) -fpic -fPIC
CXXFLAGS_LIBMUIKABOT := $(CXXFLAGS) -fpic -fPIC
LIBS_LIBMUIKABOT := -lpthread -lcurl

#
# Telegram bot executable.
#
MUIKA_TGBOT := muika_tgbot
SOURCES_MUIKA_TGBOT := \
	mtgbot/Bot.cpp \
	mtgbot/main.cpp \
	mtgbot/Reactor.cpp
OBJECTS_MUIKA_TGBOT := $(SOURCES_MUIKA_TGBOT:.cpp=.o)
DEPENDS_MUIKA_TGBOT := $(SOURCES_MUIKA_TGBOT:.cpp=.d)
CFLAGS_MUIKA_TGBOT := $(CFLAGS) -fpie -fPIE -DHAVE_CURL
CXXFLAGS_MUIKA_TGBOT := $(CXXFLAGS) -fpie -fPIE -DHAVE_CURL
LIBS_MUIKA_TGBOT := -lpthread -lTgBot -lcrypto -lssl -lcurl

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

all: $(MUIKA_TGBOT)

$(MUIKA_TGBOT): $(OBJECTS_MUIKA_TGBOT) $(LIBMUIKABOT)
	$(LD) $(LDFLAGS) -fpie -fPIE -Wl,-rpath,'$$ORIGIN' -o $@ $^ $(LIBS_MUIKA_TGBOT)

$(LIBMUIKABOT): $(OBJECTS_LIBMUIKABOT)
	$(LD) $(LDFLAGS) -fpic -fPIC -shared -o $@ $^ $(LIBS_LIBMUIKABOT)

$(OBJECTS_MUIKA_TGBOT): %.o: %.cpp
	$(CXX) $(CXXFLAGS_MUIKA_TGBOT) -MMD -MP -c -o $@ $<

-include $(DEPENDS_MUIKA_TGBOT)

$(OBJECTS_LIBMUIKABOT): %.o: %.cpp
	$(CXX) $(CXXFLAGS_LIBMUIKABOT) -MMD -MP -c -o $@ $<

-include $(DEPENDS_LIBMUIKABOT)

clean:
	rm -vf \
		$(MUIKA_TGBOT) \
		$(LIBMUIKABOT) \
		$(OBJECTS_MUIKA_TGBOT) $(DEPENDS_MUIKA_TGBOT) \
		$(OBJECTS_LIBMUIKABOT) $(DEPENDS_LIBMUIKABOT)

.PHONY: all clean
