TEMPLATE = app
QT -= gui
QT += network testlib
CONFIG += console
CONFIG -= app_bundle
DEPENDPATH += .
!win32: LIBS += -lz
win32 {
    # workaround for qdatetime.h macro bug
    DEFINES += NOMINMAX
}

CONFIG(staticlib): DEFINES += QUAZIP_STATIC

# Input
HEADERS += qztest.h \
testjlcompress.h \
testquachecksum32.h \
testquagzipfile.h \
testquaziodevice.h \
testquazipdir.h \
testquazipfile.h \
testquazip.h \
    testquazipnewinfo.h \
    testquazipfileinfo.h

SOURCES += qztest.cpp \
testjlcompress.cpp \
testquachecksum32.cpp \
testquagzipfile.cpp \
testquaziodevice.cpp \
testquazip.cpp \
testquazipdir.cpp \
testquazipfile.cpp \
    testquazipnewinfo.cpp \
    testquazipfileinfo.cpp

OBJECTS_DIR = .obj
MOC_DIR = .moc

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../quazip/release/ -lquazip
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../quazip/debug/ -lquazipd
else:mac:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../quazip/debug/ -lquazip_debug
else:unix: LIBS += -L$$OUT_PWD/../quazip/ -lquazip

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD/../quazip

unix{
HEADERS +=\
    $$PWD/qzlib/crc32.h \
    $$PWD/qzlib/deflate.h \
    $$PWD/qzlib/gzguts.h \
    $$PWD/qzlib/inffast.h \
    $$PWD/qzlib/inffixed.h \
    $$PWD/qzlib/inflate.h \
    $$PWD/qzlib/inftrees.h \
    $$PWD/qzlib/trees.h \
    $$PWD/qzlib/zconf.h \
    $$PWD/qzlib/zlib.h \
    $$PWD/qzlib/zutil.h
SOURCES +=\
    $$PWD/qzlib/adler32.c \
    $$PWD/qzlib/compress.c \
    $$PWD/qzlib/crc32.c \
    $$PWD/qzlib/deflate.c \
    $$PWD/qzlib/gzclose.c \
    $$PWD/qzlib/gzlib.c \
    $$PWD/qzlib/gzread.c \
    $$PWD/qzlib/gzwrite.c \
    $$PWD/qzlib/infback.c \
    $$PWD/qzlib/inffast.c \
    $$PWD/qzlib/inflate.c \
    $$PWD/qzlib/inftrees.c \
    $$PWD/qzlib/trees.c \
    $$PWD/qzlib/uncompr.c \
    $$PWD/qzlib/zutil.c
}
