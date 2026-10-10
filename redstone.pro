QT += core gui network qml quick quickcontrols2 dbus

CONFIG += c++17 release
TARGET = redstone
TEMPLATE = app

HEADERS += \
    src/backend.h \
    src/missionlistmodel.h \
    src/snapshot.h \
    src/systemtheme.h

SOURCES += \
    src/main.cpp \
    src/backend.cpp \
    src/missionlistmodel.cpp \
    src/snapshot.cpp \
    src/systemtheme.cpp

RESOURCES += src/qml/resources.qrc

target.path = /usr/bin
desktop.path = /usr/share/applications
desktop.files = packaging/redstone.desktop
icon.path = /usr/share/icons/hicolor/scalable/apps
icon.files = packaging/redstone.svg
icon256.path = /usr/share/icons/hicolor/256x256/apps
icon256.files = packaging/redstone.png
INSTALLS += target desktop icon icon256
