TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../redist C:/msys64/mingw64/include ..
DEFINES += lplex_win32  __cplusplus=201903
LIBS += -ldvdread
QMAKE_CXX=C:/msys64//mingw64/bin/g++
QMAKE_LFLAGS += -LC:/msys64//mingw64/lib #-lstdc++fs
QMAKE_CXXFLAGS += -std=c++17 -march=core-avx2 -static-libstdc++ -static -static-libgcc

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
    ../redist/lplex_precompile.h \
    ../src/color.h \
    ../src/platform.h \
    ../src/util.h \
    ../src/dvd.hpp \
    ../src/flac.hpp \
    ../src/lpcm.hpp \
    ../src/lplex.hpp \
    ../src/processor.hpp \
    ../src/wx.hpp

DISTFILES += \
    ../Makefile.am \
    ../configure.ac \
    ../lplex.def
