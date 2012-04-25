Features
========

* Open and edit encrypted/gpg text files.
* Automatically close opened file in a few minutes after last access.
* Do not ask stupid annoying questions about keys and confirmations.
* Remember each window (by filename) position and size on the screen.

Detailed description
====================

Open file
---------

To open file just call ``guzum`` with that file path as an argument.

Access recently opened files
----------------------------

Guzum stores list of most recently used files, they could be easily reopened
using notification area icon, to display that icon you should launch application
with some command line option, like:

    guzum --tray

this will open application as a tray icon.

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

