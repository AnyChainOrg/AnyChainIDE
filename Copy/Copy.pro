QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Update
TEMPLATE = app

SOURCES += main.cpp \
    DataUtil.cpp \
    UpdateWidget.cpp \
    MoveableDialog.cpp

HEADERS += \
    DataUtil.h \
    UpdateWidget.h \
    MoveableDialog.h

FORMS += \
    UpdateWidget.ui

QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

RESOURCES += \
    resource.qrc

INCLUDEPATH += $$PWD/quazip/quazip

TRANSLATIONS +=   Chinese.ts  English.ts

macx{
    LIBS += $$PWD/lib/libquazip.a
    ICON = logo.icns
}
win32{
    LIBS += $$PWD/lib/quazip.lib
    RC_FILE += logo.rc
}
