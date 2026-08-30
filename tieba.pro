VERSION = 1.0.0

# Add more folders to ship with the application, here
folder_01.source = qml/tieba
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

symbian:TARGET.UID3 = 0xE235BCBE

# Allow network access on Symbian
symbian:TARGET.CAPABILITY += NetworkServices

# NOTE: qdeclarative-boostable disabled - the slim MADDE sysroot lacks
# applauncherd's MDeclarativeCache header. The app still launches normally.

QT += network sql
LIBS += -lz

# Desktop Qt 4.7 installations do not always ship QtDBus. Harmattan does, and
# the notification implementation is only useful there.
contains(MEEGO_EDITION, harmattan) {
    QT += dbus
    DEFINES += TIEBA_HAS_DBUS
}
INCLUDEPATH += src

SOURCES += main.cpp \
    src/json.cpp \
    src/pbwire.cpp \
    src/signutil.cpp \
    src/clientinfo.cpp \
    src/httpclient.cpp \
    src/db.cpp \
    src/accountmanager.cpp \
    src/appsettings.cpp \
    src/thememanager.cpp \
    src/imagecache.cpp \
    src/notifier.cpp \
    src/util.cpp \
    src/content.cpp \
    src/proto_messages.cpp \
    src/tiebaapi.cpp \
    src/logstore.cpp \
    src/qrlogin.cpp

HEADERS += \
    src/json.h \
    src/pbwire.h \
    src/signutil.h \
    src/clientinfo.h \
    src/httpclient.h \
    src/db.h \
    src/accountmanager.h \
    src/appsettings.h \
    src/thememanager.h \
    src/imagecache.h \
    src/notifier.h \
    src/util.h \
    src/content.h \
    src/proto_messages.h \
    src/tiebaapi.h \
    src/logstore.h \
    src/qrlogin.h

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    qtc_packaging/debian_harmattan/rules \
    qtc_packaging/debian_harmattan/README \
    qtc_packaging/debian_harmattan/manifest.aegis \
    qtc_packaging/debian_harmattan/copyright \
    qtc_packaging/debian_harmattan/control \
    qtc_packaging/debian_harmattan/compat \
    qtc_packaging/debian_harmattan/changelog
