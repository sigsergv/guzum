Compilation
===========

basic Requirements
------------------

* Qt 5.7
* gpgme
* C++ compiler
* Linux/MacOS

For MacOS you *must* install Qt from official website to default location (~/.Qt). Also you need to install 
brew and gpgme package from there.


Linux (debian/ubuntu)
---------------------

Install the following packages:

* cmake
* qtbase5-dev
* libgpgme-dev

Build instructions:

~~~~~
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ make
~~~~~

To see debug output you must create file `~/.config/QtProject/qtlogging.ini`.

Translations
============

~~~~~
lupdate -no-obsolete -recursive ./src -ts src/guzum-ru.ts
~~~~~

or (for macos)

~~~~~
~/Qt/5.7/clang_64/bin/lupdate -no-obsolete -recursive ./src -ts src/guzum-ru.ts
~~~~~

Deployment
==========

Debian/Ubuntu
-------------

To create dpkg package use these commands:

~~~~~
sudo apt install dpkg-dev cmake debhelper dh-exec qtbase5-dev libgpgme-dev fakeroot
dpkg-buildpackage -rfakeroot -b
~~~~~



Prepare MacOS bundle
====================

First build a release version:

~~~~~
$ rm -rf build
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
~~~~~

Use macdeployqt to prepare bundle (inside directory `build`):

~~~~~
$ ~/Qt/5.7/clang_64/bin/macdeployqt guzum.app
~~~~~

Check deps (we need to provide only Qt frameworks):

~~~~~
$ otool -L guzum.app/Contents/MacOS/guzum
~~~~~

Examine the output and make sure that the only external dependencies are standard system ones.

Then create DMG package using script `create-dmg`:

~~~~~
$ ../scripts/create-dmg
~~~~~
