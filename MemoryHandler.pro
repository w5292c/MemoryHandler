QT += core
QT -= gui
QT += testlib

TARGET = MemoryHandler
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += src/main.cpp \
    src/memory-handler.cpp \
    src/memoryhandlertest.cpp

HEADERS += \
    src/memory-handler.h \
    src/memoryhandlertest.h
