import QtQuick 2.9
import Ubuntu.Components 1.3
import QtQuick.Window 2.2
import Morph.Web 0.1
import "UCSComponents"
import QtWebEngine 1.7
import Qt.labs.settings 1.0
import QtSystemInfo 5.5

MainView {
  id:window

  ScreenSaver {
    id: screenSaver
    screenSaverEnabled: !(Qt.application.active)
  }
  objectName: "mainView"
  theme.name: "Ubuntu.Components.Themes.SuruDark"
  applicationName: "whatsweb"
  backgroundColor : "transparent"

  WebEngineView {
    id: webview
    focus: true
    anchors{ fill: parent }
    settings.fullScreenSupportEnabled: true
    property var currentWebview: webview
    settings.pluginsEnabled: true
    onFullScreenRequested: function(request) {
      nav.visible = !nav.visible
      request.accept();
    }
    profile:  WebEngineProfile {
      id: webContext
      httpUserAgent: "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/71.0.3578.98 Safari/537.36"
      storageName: "Storage"
      persistentStoragePath: "/home/phablet/.cache/alefnode.whatsweb/alefnode.whatsweb/QtWebEngine"
    }
    anchors {
      fill:parent
      centerIn: parent.verticalCenter
    }
    url: "https://web.whatsapp.com"
    userScripts: [
      WebEngineScript {
        injectionPoint: WebEngineScript.DocumentCreation
        worldId: WebEngineScript.MainWorld
        name: "QWebChannel"
        sourceUrl: "ubuntutheme.js"
      }
    ]
  }

/*
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
        id: home
        iconName: "home"
        onTriggered: {
          webview.url = 'https://web.whatsapp.com'
        }
        text: qsTr("Home")
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




         Connections {
        target: Qt.inputMethod
        onVisibleChanged: nav.visible = !nav.visible
    }

    Connections {
        target: webview

                        onIsFullScreenChanged: {
                        window.setFullscreen()
                        if (currentWebview.isFullScreen) {

                    nav.state = "hidden"
        }
        else {

        nav.state = "shown"
        }
   }

    }
     Connections {
        target: window.webview

               onIsFullScreenChanged: window.setFullscreen(window.webview.isFullScreen)
               }
                      function setFullscreen(fullscreen) {
         if (!window.forceFullscreen) {
             if (fullscreen) {
                 if (window.visibility != Window.FullScreen) {
                     internal.currentWindowState = window.visibility
                    window.visibility = 5
                }
            } else {
                window.visibility = internal.currentWindowState
                //window.currentWebview.fullscreen = false
                //window.currentWebview.fullscreen = false
            }
        }


        }
*/
}
