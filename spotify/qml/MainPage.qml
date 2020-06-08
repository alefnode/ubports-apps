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

import QtQuick 2.9
import Ubuntu.Components 1.3
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import io.thp.pyotherside 1.3
import QtWebEngine 1.7
import Morph.Web 0.1
import "UCSComponents"


Page {
    id: root_mainpage

    anchors.fill: parent

    header: PageHeader {
        id: main_header
        title: i18n.tr('Spotify')

        leadingActionBar.actions: []
        trailingActionBar.actions: [
            Action {
                iconName: "settings"
                text: i18n.tr("Settings")

                onTriggered: {
                    page_stack.push(Qt.resolvedUrl("SettingsPage.qml"))
                }
            },
            Action {
                iconName: "info"
                text: i18n.tr("Installation")

                onTriggered: {
                    page_stack.push(Qt.resolvedUrl("Installation.qml"))
                }
            }
        ]
    }
    WebEngineView {
        id: webview
        anchors{ fill: parent }
        focus: true
        url: "http://127.0.0.1:6680/iris"
    }
    RadialBottomEdge {
        id: nav
        visible: true
        actions: [
            RadialAction {
                id: reload
                iconName: "reload"
                onTriggered: {
                    webview.reload()
                }
                text: qsTr("Reload")
            },
            RadialAction {
                id: forward
                enabled: webview.canGoForward
                iconName: "go-next"
                onTriggered: {
                    webview.goForward()
                }
               text: qsTr("Forward")
             },
            RadialAction {
                id: back
                enabled: webview.canGoBack
                iconName: "go-previous"
                onTriggered: {
                    webview.goBack()
                }
                text: qsTr("Back")
            }
        ]
    }

    Python {
        id: python

        Component.onCompleted: {
            addImportPath(Qt.resolvedUrl('../src/'));

            importModule('install', function() {
                console.log('module install imported');
                python.call('install.first_start', [], {
                });
            });
        }

        onError: {
            console.log('python error: ' + traceback);
        }
    }
}
