TEMPLATE = subdirs

SUBDIRS += \
    ChainIDE \
    Copy
win32{
SUBDIRS +=\
    Copy/quazip
}
