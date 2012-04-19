DEFINES += GUZUM_VERSION="\\\"1.0.0\\\""

HEADERS += settings.h \
    encryptedtextwindow.h

SOURCES += main.cpp \
    settings.cpp \
    encryptedtextwindow.cpp

FORMS += 

CODECFORTR = UTF-8
TRANSLATIONS = ../translations/guzum_ru.ts

QT += xml sql network webkit 

RESOURCES = ../resources/application.qrc
TARGET = guzum
DESTDIR = ../

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
