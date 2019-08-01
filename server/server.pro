QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle
CONFIG += qt

LIBS += -lboost_system \
        -lpthread \
        -lboost_iostreams \
        -lboost_filesystem \

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
    acceptor.cpp \
    server.cpp \
    service.cpp \
    ../http/httpgetrequest.cpp \
    ../http/httppostrequest.cpp \
    ../http/httprequest.cpp \
    ../http/httpserver.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    acceptor.h \
    server.h \
    service.h \
    ../http/httpgetrequest.h \
    ../http/httppostrequest.h \
    ../http/httprequest.h \
    ../http/httpserver.h
