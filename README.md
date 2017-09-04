Yat (yet another terminal) is a VT100-emulating terminal written with Qt Quick.

Currently, master branch requires Qt 5.10; previous versions targeted older versions of Qt.

## To compile and try it out

```
qmake
make
qml -I imports yat_app/yat.qml
```

## To install on your system

First install the dependencies in your system Qt path, then install yat.qml somewhere in your PATH
(use sudo if it gives you permission errors, and if you agree about the installation directory):

```
qmake
make install
cp yat_app/yat.qml /usr/bin/yat
```

yat.qml is a script starting with a shebang, so you can run it just like that, the same way
you could run a Python script for example.  And as with Python, the dependencies need to
be installed too; ```make install``` will by default install in a directory called Yat
under Qt's Qml2ImportsPath, which may be something like /usr/lib/qt/qml
if you are using your distro's Qt packages.

If the QML is installed in this way, yat can also be used as a component inside other applications.


## To build a self-contained executable

This is not necessary, but if you prefer to make a self-contained executable instead of installing the QML files separately:

```
cd yat_app
qmake
make
```

It will incorporate the QML into the executable's resources, and you now have a moveable executable.
But the downside is you cannot customize the QML at runtime - you need to compile it again
if you make a change.

