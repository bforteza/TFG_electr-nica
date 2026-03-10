# Makefile para desarrollo en PC (Debian x86_64)
# El archivo .cbp es para Raspberry Pi (aarch64) con Code::Blocks

CXX      := g++
CXXFLAGS := -Wall -fexceptions -g -std=c++17 \
            $(shell pkg-config gtkmm-3.0 --cflags) \
            -Iinclude \
            -I/usr/include \
            -I/usr/include/mariadb

LDFLAGS  := $(shell pkg-config gtkmm-3.0 --libs) \
            /usr/local/lib/static/liblitesql.a \
            /usr/local/lib/static/liblitesql-util.a \
            -L/usr/lib/x86_64-linux-gnu \
            -lmysqlclient \
            -lnfc \
            -lpthread

SRCS := main.cpp \
        src/application.cpp \
        src/db_schema.cpp \
        src/globals.cpp \
        src/home_stack.cpp \
        src/key_create_stack.cpp \
        src/key_view_stack.cpp \
        src/login_stack.cpp \
        src/nfc_manager.cpp \
        src/solenoid_panel.cpp \
        src/user_create_stack.cpp \
        src/users_view_stack.cpp \
        src/window.cpp

OBJS   := $(SRCS:.cpp=.o)
TARGET := bin/Debug/Pruebas2

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p bin/Debug
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	ln -sfn ../../ui bin/Debug/ui

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f main.o src/*.o
