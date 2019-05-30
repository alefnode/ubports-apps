QT += network dbus multimedia
CONFIG += mobility link_pkgconfig
PKGCONFIG += connectivity-qt1

INCLUDEPATH += $$PWD

SOURCES += ../libQtSpotify/qspotifysession.cpp \
    ../libQtSpotify/qspotifyuser.cpp \
    ../libQtSpotify/qspotifyobject.cpp \
    ../libQtSpotify/qspotifytrack.cpp \
    ../libQtSpotify/qspotifyplaylist.cpp \
    ../libQtSpotify/qspotifyplaylistcontainer.cpp \
    ../libQtSpotify/qspotifyplaylistsearchentry.cpp \
    ../libQtSpotify/qspotifytracklist.cpp \
    ../libQtSpotify/qspotifyartist.cpp \
    ../libQtSpotify/qspotifyalbum.cpp \
    ../libQtSpotify/qspotifyimageprovider.cpp \
    ../libQtSpotify/qspotifysearch.cpp \
    ../libQtSpotify/qspotifyplayqueue.cpp \
    ../libQtSpotify/qspotifyalbumbrowse.cpp \
    ../libQtSpotify/qspotifyartistbrowse.cpp \
    ../libQtSpotify/qspotifytoplist.cpp \
    ../libQtSpotify/listmodels/listmodelbase.cpp \
    ../libQtSpotify/listmodels/tracklistfiltermodel.cpp \
    ../libQtSpotify/qspotifyaudiothreadworker.cpp \
    ../libQtSpotify/qspotifyevents.cpp \
    ../libQtSpotify/listmodels/qspotifyartistlist.cpp \
    ../libQtSpotify/listmodels/qspotifyalbumlist.cpp \
    ../libQtSpotify/listmodels/qspotifyplaylistsearchlist.cpp \
    ../libQtSpotify/qspotifycachemanager.cpp \
    ../libQtSpotify/qspotifyringbuffer.cpp \
    ../libQtSpotify/mpris/mprismediaplayerplayer.cpp

HEADERS += ../libQtSpotify/qspotifysession.h \
    ../libQtSpotify/spotify_key.h \
    ../libQtSpotify/qspotifyuser.h \
    ../libQtSpotify/qspotifyobject.h \
    ../libQtSpotify/qspotifytrack.h \
    ../libQtSpotify/qspotifyplaylist.h \
    ../libQtSpotify/qspotifyplaylistcontainer.h \
    ../libQtSpotify/qspotifyplaylistsearchentry.h \
    ../libQtSpotify/qspotifytracklist.h \
    ../libQtSpotify/QtSpotify \
    ../libQtSpotify/qspotify_qmlplugin.h \
    ../libQtSpotify/qspotifyartist.h \
    ../libQtSpotify/qspotifyalbum.h \
    ../libQtSpotify/qspotifyimageprovider.h \
    ../libQtSpotify/qspotifysearch.h \
    ../libQtSpotify/qspotifyplayqueue.h \
    ../libQtSpotify/qspotifyalbumbrowse.h \
    ../libQtSpotify/qspotifyartistbrowse.h \
    ../libQtSpotify/qspotifytoplist.h \
    ../libQtSpotify/qspotifyevents.h \
    ../libQtSpotify/listmodels/listmodelbase.h \
    ../libQtSpotify/listmodels/tracklistfiltermodel.h \
    ../libQtSpotify/shared_ptr.h \
    ../libQtSpotify/qspotifyaudiothreadworker.h \
    ../libQtSpotify/listmodels/qspotifyartistlist.h \
    ../libQtSpotify/listmodels/qspotifyalbumlist.h \
    ../libQtSpotify/listmodels/qspotifyplaylistsearchlist.h \
    ../libQtSpotify/qspotifycachemanager.h \
    ../libQtSpotify/qspotifyringbuffer.h \
    ../libQtSpotify/mpris/mprismediaplayer.h \
    ../libQtSpotify/mpris/mprismediaplayerplayer.h

QMAKE_CXXFLAGS += -std=c++0x -Wno-unused-function

ARCH = $$QMAKE_HOST.arch
equals(ARCH, armv7l) {
message(arm build)
    INCLUDEPATH += ../libspotify/include
    LIBS += -L$$PWD/../libspotify/lib -lspotify
    DEFINES += ARM_BUILD
} else {
    message(non-arm build)
    INCLUDEPATH += ../libspotify_emu/include
    LIBS += -L$$PWD/../libspotify_emu/lib -lspotify
}
