TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp

INCLUDEPATH += $$PWD/ffmpeg/include

LIBS += -L$$PWD/ffmpeg/lib/ -lavcodec  \
        -L$$PWD/ffmpeg/lib/ -lavfilter \
        -L$$PWD/ffmpeg/lib/ -lavformat \
        -L$$PWD/ffmpeg/lib/ -lswscale  \
        -L$$PWD/ffmpeg/lib/ -lavutil  \
