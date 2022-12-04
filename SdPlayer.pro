#-------------------------------------------------
#
# Project created by QtCreator 2010-10-15T16:22:57
#
#-------------------------------------------------

QT       += core gui

# Required on Qt5+ - https://stackoverflow.com/a/9112452/1797496
QT       += widgets

TARGET = SdPlayer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    playlistmodel.cpp \
    loop.cpp \
    shortcuts.cpp \
    shortcut.cpp \
    settings.cpp \
    breaktimer.cpp \
    speed.cpp

HEADERS  += mainwindow.h \
    playlistmodel.h \
    loop.h \
    shortcuts.h \
    shortcut.h \
    settings.h \
    breaktimer.h \
    speed.h

FORMS    += mainwindow.ui \
    loop.ui \
    shortcuts.ui \
    shortcut.ui \
    settings.ui \
    breaktimer.ui \
    speed.ui

# for windows
win32 {
  OTHER_FILES += \
      sdplayer.rc
  RC_FILE += sdplayer.rc

  # path from my project to vlc include files
  INCLUDEPATH += "../vlc-3.0.18/sdk/include"

  # path to vlc libraries
  LIBS += -L"../vlc-3.0.18/sdk/lib/"
  LIBS += -llibvlc
}

# for linux
!win32 {
  LIBS += -lvlc
}

# for macos
macx {
  INCLUDEPATH += "/Applications/VLC.app/Contents/MacOS/include"
  # path to vlc libraries
  LIBS += -L"/Applications/VLC.app/Contents/MacOS/lib"
  LIBS += -lvlc
}
