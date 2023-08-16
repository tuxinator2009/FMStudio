TEMPLATE = app
TARGET = FMStudio
DESTDIR = ..

CONFIG+=c++17

DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_RPATHDIR += $ORIGIN/lib

LIBS+=-lm

QT += core gui widgets multimedia
RESOURCES += resources.qrc

CONFIG += debug

SOURCES += \
        CHeaderParser/cheaderarray.cpp \
        CHeaderParser/cheaderobject.cpp \
        CHeaderParser/cheaderparser.cpp \
        CHeaderParser/cheadervalue.cpp \
        cheaderview.cpp \
        fmproject.cpp \
        fmsong.cpp \
        FMSource.cpp \
        globals.cpp \
        instrumenteditor.cpp \
        main.cpp \
        mainwindow.cpp \
        newinstrument.cpp \
        patterneditor.cpp \
        songeditor.cpp \
        spectrumpreview.cpp \
        undo.cpp \
        virtualpiano.cpp \
        waveformanalyzer.cpp \
        waveformpreview.cpp

HEADERS += \
        CHeaderParser/cheaderarray.h \
        CHeaderParser/cheaderobject.h \
        CHeaderParser/cheaderparser.h \
        CHeaderParser/cheadervalue.h \
        cheaderview.h \
        fmproject.h \
        fmsong.h \
        FMSource.h \
        globals.h \
        instrumenteditor.h \
        mainwindow.h \
        newinstrument.h \
        notespinbox.h \
        patterneditor.h \
        songeditor.h \
        spectrumpreview.h \
        undo.h \
        virtualpiano.h \
        waveformanalyzer.h \
        waveformpreview.h

FORMS += \
        cheaderview.ui \
        instrumenteditor.ui \
        mainwindow.ui \
        newinstrument.ui
