
TARGET = ModbusScope
TEMPLATE = app

include("../ModbusScope.pri")

SOURCES += \
        $$PWD/main.cpp

win32 {
    RC_ICONS = icon/application.ico
}

HEADERS +=



