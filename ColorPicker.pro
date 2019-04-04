QT       += core gui widgets

TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp widget.cpp
HEADERS += widget.h
FORMS += widget.ui
RESOURCES += assets.qrc

OBJECTS_DIR = .obj
MOC_DIR = .moc
RCC_DIR = .rcc

win32: {
    DISTFILES += data.rc
    RC_FILE += data.rc
    HEADERS += data.h
    TARGET = ColorPicker
}
unix: {
    TARGET = smcolorpicker
}


