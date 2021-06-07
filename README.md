# programConfigQtso
C++ library to help with program configuration: commandline settings, loading settings from a configuration file, saving settings to a file, initializing log, loading/saving translations, signaling important events...

Compilation
-----------
Requires:

Qt library

https://github.com/jouven/logsinJSONQtso

https://github.com/jouven/cryptoQtso

https://github.com/jouven/textQtso

https://github.com/jouven/translatorJSONQtso

https://github.com/jouven/essentialQtso

Check .pro file to know what library names expect (or to change them).

Run (in programConfigQtso source directory or pointing to it):

    qmake

and then:

    make
