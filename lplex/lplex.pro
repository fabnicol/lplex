TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ../src/dvd.cpp \
    ../src/exec.cpp \
    ../src/flac.cpp \
    ../src/layout.cpp \
    ../src/lgzip.cpp \
    ../src/lpcm.cpp \
    ../src/lplex.cpp \
    ../src/main.cpp \
    ../src/reader.cpp \
    ../src/util.cpp \
    ../src/video.cpp \
    ../src/writer.cpp \
    ../src/wx.cpp

HEADERS += \
    ../src/color.h \
    ../src/platform.h \
    ../src/util.h \
    ../src/dvd.hpp \
    ../src/flac.hpp \
    ../src/lpcm.hpp \
    ../src/lplex.hpp \
    ../src/processor.hpp \
    ../src/wx.hpp
