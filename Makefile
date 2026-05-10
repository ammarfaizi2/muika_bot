# SPDX-License-Identifier: GPL-2.0-only

CC = gcc
CXX = g++
LD = $(CXX)
CFLAGS := -Wall -Wextra -Os -ggdb3 -std=gnu11 $(CFLAGS) -I. -Wno-deprecated -fvisibility=hidden -DHAVE_CURL
CXXFLAGS := -Wall -Wextra -Os -ggdb -std=gnu++17 $(CXXFLAGS) -I./muika/json/include -I. -Wno-deprecated -fvisibility=hidden -DHAVE_CURL
LDFLAGS := -Os -ggdb3 $(LDFLAGS)
LIBS := -lpthread -lTgBot -lcrypto -lssl -lcurl

#
# Base library for Muika bot.
#
LIBMUIKABOT := libmuikabot.so
SOURCES_LIBMUIKABOT := \
	muika/helpers.cpp \
	muika/Message.cpp \
	muika/Module.cpp \
	muika/Muika.cpp \
	muika/Reactor.cpp
OBJECTS_LIBMUIKABOT := $(SOURCES_LIBMUIKABOT:.cpp=.o)
DEPENDS_LIBMUIKABOT := $(SOURCES_LIBMUIKABOT:.cpp=.d)
CFLAGS_LIBMUIKABOT := -fPIC $(CFLAGS)
CXXFLAGS_LIBMUIKABOT := -fPIC $(CXXFLAGS)

#
# Telegram bot executable.
#
MUIKA_TGBOT := muika_tgbot
SOURCES_MUIKA_TGBOT := \
	tgbot/main.cpp
OBJECTS_MUIKA_TGBOT := $(SOURCES_MUIKA_TGBOT:.cpp=.o)
DEPENDS_MUIKA_TGBOT := $(SOURCES_MUIKA_TGBOT:.cpp=.d)
CFLAGS_MUIKA_TGBOT := $(CFLAGS)
CXXFLAGS_MUIKA_TGBOT := $(CXXFLAGS)

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
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

$(LIBMUIKABOT): $(OBJECTS_LIBMUIKABOT)
	$(LD) $(LDFLAGS) -shared -o $@ $^ $(LIBS)

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
