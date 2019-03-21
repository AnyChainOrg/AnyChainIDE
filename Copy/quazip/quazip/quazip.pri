INCLUDEPATH += $$PWD\

DEPENDPATH += $$PWD
HEADERS += \
        $$PWD/crypt.h \
        $$PWD/ioapi.h \
        $$PWD/JlCompress.h \
        $$PWD/quaadler32.h \
        $$PWD/quachecksum32.h \
        $$PWD/quacrc32.h \
        $$PWD/quagzipfile.h \
        $$PWD/quaziodevice.h \
        $$PWD/quazipdir.h \
        $$PWD/quazipfile.h \
        $$PWD/quazipfileinfo.h \
        $$PWD/quazip_global.h \
        $$PWD/quazip.h \
        $$PWD/quazipnewinfo.h \
        $$PWD/unzip.h \
        $$PWD/zip.h

SOURCES += $$PWD/qioapi.cpp \
           $$PWD/JlCompress.cpp \
           $$PWD/quaadler32.cpp \
           $$PWD/quacrc32.cpp \
           $$PWD/quagzipfile.cpp \
           $$PWD/quaziodevice.cpp \
           $$PWD/quazip.cpp \
           $$PWD/quazipdir.cpp \
           $$PWD/quazipfile.cpp \
           $$PWD/quazipfileinfo.cpp \
           $$PWD/quazipnewinfo.cpp \
           $$PWD/unzip.c \
           $$PWD/zip.c
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
