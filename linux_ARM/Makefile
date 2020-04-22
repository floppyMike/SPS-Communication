IDIR =../include ../asio/asio/include ../libnodave ../rapidjson/include
LDIR =../libnodave
SDIR =../src

SRC =$(SDIR)/main.cpp
LIBS =openSocket.o setport.o nodave.o

CC=g++
CFLAGS=-std=c++17 -DLINUX $(IDIR:%=-I%) $(LDIR:%=-L%) $(LIBS:%=-l:%) -lpthread -Wall -Wextra -O3

SPS_Com: $(SRC)
	$(CC) -o SPS_Com $(SRC) $(CFLAGS)

.PHONY: install

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

install: SPS_Com
	install -d $(DESTDIR)$(PREFIX)/bin