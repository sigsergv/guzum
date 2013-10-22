Features
========

* Open and edit encrypted/gpg text files.
* Automatically close opened file in a few minutes after last access.
* Do not ask stupid annoying questions about keys and confirmations.
* Remember each window (by filename) position and size on the screen.

How does it work
================

On first launch without any arguments program places icon to the notification area and
nothing else. If program is already started and icon is placed to the notification
program does nothing.

When started with the argument ``--select-file`` program shows select file dialog
to choose encrypted file to edit. For convenience there is a script named
``guzum-select-file`` that executes command ``guzum --select-file``.

When started with the path to encrypted file guzum opens editor for this file and also
adds it to recently used files (for the notification area icon).

Access recently opened files
----------------------------

Guzum stores list of most recently used files, they could be easily re-opened
using notification area icon.

Internal scenarious and requirements
====================================

Opening encrypted file
----------------------

* detect is file is really GPG-encrypted file
* extract and remember key ID that is used for decryption (and use to encrypt file after modifications)
* do NOT save decrypted data anywhere on the disk
* do NOT open non-encrypted files with extensions gpg/asc
* automatically detect charset

Opening nonexisting encrypted file
----------------------------------

* if user tries to open nonexisting gpg/asc file assume he wants to encrypt it using his own secret key

IPC
---

Guzum uses UNIX-socket for interprocess communcations. When user calls guzum with the argument
``--select-file`` or with the file path it tries to find running instance and pass these data 
there. If there is no running instance (detected by UNIX-socket) it starts one and then performs
the requested action (open file in the editor or open select file dialog).

