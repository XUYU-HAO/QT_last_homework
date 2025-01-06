QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TcpFileServerandSender
TEMPLATE = app

SOURCES += main.cpp \
           TcpFileServerandSender.cpp \
           tcpfilesender.cpp \
           tcpfileserver.cpp

HEADERS += TcpFileServerandSender.h \
           tcpfilesender.h \
           tcpfileserver.h
