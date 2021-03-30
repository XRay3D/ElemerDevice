QT += core serialport

include(../CommonInterfaces/CommonInterfaces.pri)

HEADERS += \
    $$PWD/ed_common_types.h \
    $$PWD/ed_device.h \
    $$PWD/ed_port.h \
    $$PWD/ed_utils.h

INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/ed_device.cpp \
    $$PWD/ed_port.cpp
