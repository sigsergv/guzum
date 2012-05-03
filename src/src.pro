DEFINES += GUZUM_VERSION="\\\"1.1.0\\\""
DEFINES += _FILE_OFFSET_BITS="64"

HEADERS += settings.h \
    encryptedtextwindow.h \
    gpgmewrapper.h \
    passphrasedialog.h \
    traymanager.h \
    traymenuadaptor.h \
    aboutdialog.h \
    prefsdialog.h

SOURCES += main.cpp \
    settings.cpp \
    encryptedtextwindow.cpp \
    gpgmewrapper.cpp \
    passphrasedialog.cpp \
    traymanager.cpp \
    traymenuadaptor.cpp \
    aboutdialog.cpp \
    prefsdialog.cpp

FORMS += passphrasedialog.ui \
    prefsdialog.ui \
    aboutdialog.ui

CODECFORTR = UTF-8
TRANSLATIONS = ../translations/guzum_ru.ts

QT += dbus

RESOURCES = ../resources/application.qrc
TARGET = guzum
DESTDIR = ../

LIBS += -lgpgme

unix {
    #VARIABLES
    isEmpty(PREFIX) {
    PREFIX = /usr
    }

    BINDIR = $$PREFIX/bin
    DATADIR =$$PREFIX/share

    DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"

    target.path = $$BINDIR

    desktop.path = $$DATADIR/applications
    desktop.files += ../guzum.desktop

    icon16.path = $$DATADIR/icons/hicolor/16x16/apps/
    icon16.files = ../resources/icons/16x16/Guzum.png

    icon32.path = $$DATADIR/icons/hicolor/32x32/apps/
    icon32.files = ../resources/icons/32x32/Guzum.png

    icon48.path = $$DATADIR/icons/hicolor/48x48/apps/
    icon48.files = ../resources/icons/48x48/Guzum.png

    INSTALLS += target desktop icon16 icon32 icon48


}
