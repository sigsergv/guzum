TEMPLATE = subdirs

# Directories
SUBDIRS += src

isEmpty(PREFIX) {
PREFIX = /usr
}

DATADIR =$$PREFIX/share

update-translations.commands = lupdate src/src.pro
compile-translations.commands = lrelease src/src.pro
QMAKE_EXTRA_TARGETS = update-translations compile-translations

translations.depends = compile-translations
translations.path = $$DATADIR/guzum/translations
translations.files = translations/*.qm
#make_default.depends += compile-translations

INSTALLS += translations
