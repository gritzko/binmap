#-------------------------------------------------
#
# Project created by QtCreator 2010-01-15T00:37:17
#
#-------------------------------------------------

QT =

TARGET   = unitTest
CONFIG  += console
CONFIG  -= app_bundle
TEMPLATE = app
SOURCES += unit-tests.cpp
INCLUDEPATH += ..
INCLUDEPATH += ../cRandom
LIBS        += -L.. -lbinmap
LIBS        += -L../cRandom -lcrandom
LIBS        += -lgtest
