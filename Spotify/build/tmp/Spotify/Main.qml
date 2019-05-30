import QtQuick 2.4
import Ubuntu.Components 1.3
import Ubuntu.Web 0.2
import com.canonical.Oxide 1.14 as Oxide
import Ubuntu.Components.Popups 1.3
import QtSystemInfo 5.0
import Qt.labs.settings 1.0

MainView {
    objectName: "mainView"
    applicationName: "spotify"

    width: units.gu(50)
    height: units.gu(75)

    backgroundColor: "#141526"

    // Startup settings
    Settings {
        id: settings

        property string donateMe: ""
    }
    property alias donateMe: settings.donateMe

    Page {
        header: PageHeader {
            id: pageHeader
            title: i18n.tr("Spotify")
            StyleHints {
                foregroundColor: "#ffffff"
                backgroundColor: "#141526"
                dividerColor: "#1cbfff"
            }
            trailingActionBar {
                numberOfSlots: 1
                actions: [
                    Action {
                        id: donateAction
                        text: i18n.tr("Donate")
                        iconName: "like"
                        onTriggered: {
                            Qt.openUrlExternally("#")
                        }
                    }
                ]
            }
        }

        WebContext {
            id: webcontext
            userAgent: "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.131 Safari/537.36"
            userScripts: [
                Oxide.UserScript {
                    context: "oxide://"
                    url: Qt.resolvedUrl("userscript.js")
                    matchAllFrames: true
                }
            ]
        }

        WebView {
            id: webview
            anchors {
               top: pageHeader.bottom
               bottom: parent.bottom
               left: parent.left
               right: parent.right
            }

            context: webcontext
            url: "https://open.spotify.com/"
            preferences.localStorageEnabled: true
            preferences.appCacheEnabled: true
            preferences.javascriptCanAccessClipboard: true
            preferences.allowFileAccessFromFileUrls: true
            preferences.allowUniversalAccessFromFileUrls: true
        }

        Component.onCompleted: {
            // Donate me dialog
            if (donateMe === "") {
                PopupUtils.open(donateMeComponent);
                donateMe = "showed"
            }
        }

        Component {
            id: donateMeComponent

            Dialog {
                id: donateMeDialog
                title: i18n.tr("Donate me")
                text: i18n.tr("Donate to support me continue developing for Ubuntu.")

                Row {
                    spacing: units.gu(1)
                    Button {
                        width: parent.width/2 - units.gu(0.5)
                        text: i18n.tr("Ignore")
                        onClicked: PopupUtils.close(donateMeDialog)
                    }

                    Button {
                        width: parent.width/2 - units.gu(0.5)
                        text: i18n.tr("Donate")
                        color: UbuntuColors.blue
                        onClicked: {
                            Qt.openUrlExternally("https://liberapay.com/turanmahmudov")
                            PopupUtils.close(donateMeDialog)
                        }
                    }
                }
            }
        }
    }
}
