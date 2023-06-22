TARGET = db_helper
include(../../common.pri)

QT -= gui
QT += sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = lib
CONFIG += staticlib

HEADERS *= $$files(../SRC/inc/*.h, SRC/inc)
SOURCES *= $$files(../SRC/src/*.cpp, SRC/src)
FORMS *= $$files(../SRC/ui/*.ui, SRC/ui)
INCLUDEPATH *= ../SRC/inc

DESTDIR = $$PWD/../EXE

## Default rules for deployment.
#unix {
#    target.path = $$[QT_INSTALL_PLUGINS]/generic
#}
#!isEmpty(target.path): INSTALLS += target
