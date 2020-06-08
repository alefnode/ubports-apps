/*
 * Copyright (C) 2020  Adrian Campos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * ubuntu-calculator-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.7
import Ubuntu.Components 1.3
import QtQuick.Controls 2.2
//import QtQuick.Layouts 1.3
//import Qt.labs.settings 1.0
import io.thp.pyotherside 1.3


Page {
    id: root_installation
    anchors.fill: parent

    header: PageHeader {
        id: main_header
        title: i18n.tr("Installation")
    }
    Column {
        id: root_column
        anchors {
            top: main_header.bottom
            left: parent.left
            right: parent.right
        }
        ListItemLayout {
            title.text: i18n.tr("Libertine")
            Button {
                id: install_libertine_button
                text: "INSTALL"
                onClicked: {
                    console.log('in onClicked');
                    python.call('install.libertine_create', [], function(result) {
                        console.log('Executing Installation');
                        content.text = "Executing Installation";
                    });
                }
            }
        }
        ListItemLayout {
            title.text: i18n.tr("Mopidy")
            Button {
                id: install_mopidy_button
                text: "INSTALL"
                enabled: true
                onClicked: {
                    console.log('in onClicked');
                    python.call('install.mopidy_install', [], function(result) {
                        console.log('Executing Installation');
                        content.text = "Executing Installation";
                    });
                }
            }
        }
        ListItemLayout {
            title.text: i18n.tr("Spotify")
            Button {
                id: install_spotify_button
                text: "INSTALL"
                enabled: true
                onClicked: {
                    console.log('in onClicked');
                    python.call('install.spotify_install', [], function(result) {
                        console.log('Executing Installation');
                        content.text = "Executing Installation";
                    });
                }
            }
        }
        ListItemLayout {
            id: installation_status
            title.text: i18n.tr("Installation Status")
        }
        ListItemLayout {
            id: test_libertine
            title.text: i18n.tr("Libertine [~ 30min]")
            AnimatedImage {
                id: test_libertineImage
                source: "../assets/loading.gif"
                width: units.gu(4)
                height: units.gu(4)
            }
        }
        ListItemLayout {
            id: test_mopidy
            title.text: i18n.tr("Mopidy [~ 2min]")
            AnimatedImage {
                id: test_mopidyImage
                source: "../assets/loading.gif"
                width: units.gu(4)
                height: units.gu(4)
            }
        }
        ListItemLayout {
            id: test_spotify
            title.text: i18n.tr("Spotify [~ 2min]")
            AnimatedImage {
                id: test_spotifyImage
                source: "../assets/loading.gif"
                width: units.gu(4)
                height: units.gu(4)
            }
        }
        ListItemLayout {
            ScrollView {
                id: frame
                clip: true
                anchors.fill: parent
                anchors.bottom: root_installation.bottom
                ScrollBar.horizontal.interactive: false
                ScrollBar.vertical.interactive: true
                height: content.implicitHeight
                Text {
                    id: content
                    text: " "
                }
            }
        }
        Timer {
            id: readStarter
            interval: 1000
            running: true
            repeat: true
            onTriggered: {
                python.call('install.log_file', [], function(result) {
                    console.log('Autoreload Log');
                    content.text = result;
                });
            }
        }
        Timer {
            id: testsTimer
            interval: 2000
            running: true
            repeat: true
            onTriggered: {
                python.call('tests.test_libertine', [], function(result){
                    test_libertineImage.source = result;
                    if ( test_libertineImage.source == "../assets/done.gif" ) {
                        install_mopidy_button.enabled = true;
                    }
                });
                python.call('tests.test_mopidy', [], function(result){
                    test_mopidyImage.source = result;
                    if ( test_mopidyImage.source == "../assets/done.gif" ) {
                        install_spotify_button.enabled = false;
                    }

                });
                python.call('tests.test_spotify', [], function(result){
                    test_spotifyImage.source = result;
                });
                if ( test_libertineImage.source == "../assets/done.gif" &&  test_mopidyImage.source == "../assets/done.gif" &&  test_spotifyImage.source == "../assets/done.gif") {
                    testsTimer.running = false;
                    readStarter.running = false;
                }
            }
        }
        Timer {
            id: stepsTimer
            interval: 10000
            running: false
            repeat: false
            onTriggered: {
                python.call('install.install', [], {});
            }
        }
    }

    Python {
        id: python

        Component.onCompleted: {
            addImportPath(Qt.resolvedUrl('../src/'));

            importModule('install', function() {
                console.log('module install imported');
                python.call('install.avoid_suspension', [], {});
            });
            importModule('tests', function() {
                console.log('module test imported');
            });
        }

        onError: {
            console.log('python error: ' + traceback);
        }
    }
}
