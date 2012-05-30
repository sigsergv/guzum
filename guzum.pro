TEMPLATE = subdirs

# Directories
SUBDIRS += src

isEmpty(PREFIX) {
    PREFIX = /usr
}

DATADIR =$$PREFIX/share

update_translations.commands = lupdate $$PWD/src/src.pro
compile_translations.commands = lrelease $$PWD/src/src.pro
QMAKE_EXTRA_TARGETS = update_translations compile_translations

translations.depends = compile_translations
translations.path = $$DATADIR/guzum/translations
translations.files = translations/*.qm

INSTALLS += translations
