# -------------------------------------------------
# Project created by QtCreator 2010-01-06T16:03:32
# -------------------------------------------------
QT -= core \
    gui
TARGET = binmap
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    bin.cpp \
    binmap.cpp \
    cRandom/crandom.c \
    cRandom/dSFMT/dSFMT.c
HEADERS += bin.h \
    binmap.h \
    cRandom/crandom.h \
    cRandom/dSFMT/dSFMT-params216091.h \
    cRandom/dSFMT/dSFMT-params132049.h \
    cRandom/dSFMT/dSFMT-params86243.h \
    cRandom/dSFMT/dSFMT-params44497.h \
    cRandom/dSFMT/dSFMT-params19937.h \
    cRandom/dSFMT/dSFMT-params11213.h \
    cRandom/dSFMT/dSFMT-params4253.h \
    cRandom/dSFMT/dSFMT-params2203.h \
    cRandom/dSFMT/dSFMT-params1279.h \
    cRandom/dSFMT/dSFMT-params521.h \
    cRandom/dSFMT/dSFMT-params.h \
    cRandom/dSFMT/dSFMT.h
