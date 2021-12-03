DEFINES += GUZUM_VERSION="\\\"1.5\\\""
DEFINES += _FILE_OFFSET_BITS="64"

HEADERS += settings.h \
    encryptedtextwindow.h \
    gpgmewrapper.h \
    passphrasedialog.h \
    traymanager.h \
    aboutdialog.h \
    prefsdialog.h \
    managehistorydialog.h \
    secureplaintexteditor.h \
    controlpeer.h

SOURCES += main.cpp \
    settings.cpp \
    encryptedtextwindow.cpp \
    gpgmewrapper.cpp \
    passphrasedialog.cpp \
    traymanager.cpp \
    aboutdialog.cpp \
    prefsdialog.cpp \
    managehistorydialog.cpp \
    secureplaintexteditor.cpp \
    controlpeer.cpp

FORMS += passphrasedialog.ui \
    prefsdialog.ui \
    aboutdialog.ui \
    managehistorydialog.ui

CODECFORTR = UTF-8
TRANSLATIONS = ../translations/guzum_ru.ts

QT += widgets network testlib

RESOURCES = ../resources/application.qrc
TARGET = guzum
DESTDIR = ../

macx {
    LIBS += -L/usr/local/lib
    INCLUDEPATH += /usr/local/include
    ICON = ../resources/icons/guzum.icns

    QMAKE_INFO_PLIST = ../resources/Info.plist
    mactrans.target = mactrans
    mactrans.commands = ~/Qt/5.15.2/clang_64/bin/lrelease src.pro && cp ../translations/*.qm ../guzum.app/Contents/Resources
    QMAKE_EXTRA_TARGETS += mactrans
    PRE_TARGETDEPS += mactrans
}

LIBS += -lgpgme

CONFIG(release, release|debug) {
    message(RELEASE)
    DEFINES += QT_NO_DEBUG_OUTPUT
}

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
