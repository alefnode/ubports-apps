import QtQuick 2.4
import Ubuntu.Components 1.3
import QtWebEngine 1.7
//import com.canonical.Oxide 1.14 as Oxide
import Ubuntu.Components.Popups 1.3
//import QtSystemInfo 5.0
import Qt.labs.settings 1.0

MainView {
    objectName: "mainView"
    applicationName: "spotify"

    width: units.gu(50)
    height: units.gu(75)

    backgroundColor: "#141526"

    Page {
        header: PageHeader {
            id: pageHeader
            title: i18n.tr("Spotify")
            StyleHints {
                foregroundColor: "#ffffff"
                backgroundColor: "#141526"
                dividerColor: "#1cbfff"
            }
        }

        WebContext {
            id: webcontext
            userAgent: "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.131 Safari/537.36"
            //userScripts: [
            //    Oxide.UserScript {
            //        context: "oxide://"
            //        url: Qt.resolvedUrl("userscript.js")
            //        matchAllFrames: true
            //    }
            //]
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
            Component.onCompleted: {
                url = "https://open.spotify.com/"
            }
            preferences.localStorageEnabled: true
            preferences.appCacheEnabled: true
            preferences.javascriptCanAccessClipboard: true
            preferences.allowFileAccessFromFileUrls: true
            preferences.allowUniversalAccessFromFileUrls: true
        }
    }
}
