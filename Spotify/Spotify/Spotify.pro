TEMPLATE = aux
TARGET = Spotify

RESOURCES += Spotify.qrc

QML_FILES += $$files(*.qml,true) \
             $$files(*.js,true)

CONF_FILES +=  Spotify.apparmor \
               Spotify.png

#AP_TEST_FILES += tests/autopilot/run \
#                 $$files(tests/*.py,true)               

OTHER_FILES += $${CONF_FILES} \
               $${QML_FILES} \
               Spotify.desktop

#               $${AP_TEST_FILES} \
#               Spotify.desktop

#specify where the qml/js files are installed to
qml_files.path = /Spotify
qml_files.files += $${QML_FILES}

#specify where the config files are installed to
config_files.path = /Spotify
config_files.files += $${CONF_FILES}

#install the desktop file, a translated version is 
#automatically created in the build directory
desktop_file.path = /Spotify
desktop_file.files = Spotify.desktop
desktop_file.CONFIG += no_check_exist

INSTALLS += config_files qml_files desktop_file

DISTFILES += \
    scripts/userscript.js \
    userscript.js
