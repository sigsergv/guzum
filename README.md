What is Guzum
=============

**Attention! Guzum mainly is an experiment and it's not supposed to be used on regular basis**

Guzum (pronounced as //güzüm//) — is a simple encryption tool that consists of two independent parts:

* viewer/editor of text files encryted with GnuPG;
* notification area icon with the list of most recently viewed encrypted files.

To launch notification area icon execute command `guzum --tray` (in the preferences dialog you can turn on program autolaunch on system startup).

To edit encrypted file just launch program as follows:

~~~~~
guzum path/to/file.txt.gpg
~~~~~

To open file selection dialog just type (since version 1.3):

~~~~~
guzum --dialog
~~~~~

you'll be asked for passphrase. Now you can view file, edit and save back to encrypted file using toolbar button or shortcut Ctrl+S. You can also change font in the editor, it will be remembered, also if you open that file again the editor will be exactly there on the screen where you left it before (and the same size).

To open encrypted files you MUST have corresponding secret key in your keyring of course.



