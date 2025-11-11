import QtQuick 2.9
import Ubuntu.Components 1.3
import QtQuick.Window 2.2
import Morph.Web 0.1
import "UCSComponents"
import QtWebEngine 1.9
import Qt.labs.settings 1.0
import QtSystemInfo 5.5
import Ubuntu.Components.ListItems 1.3 as ListItemm
import Ubuntu.Content 1.3
import Pparent.DownloadHelper 1.0


MainView {
  id: mainView
  
  property var appID: "alefnode.whatsweb";
  property var hook: "whatsweb";  
  property var localStorage: "/home/phablet/.cache/alefnode.whatsweb/alefnode.whatsweb/QtWebEngine";    
  
  
  property int lastUnreadCount: -1;
  property var lastNotifyTimestamp: 0;  

    
  Settings {
        id: config
        category: "AppSettings"

        property int webviewWidthPortait: 410
        property int webviewWidthLandscape: 900
        property int textFontSize: 106
        property int spanFontSize: 107

        property bool enableDesktopNotifications: true
        property bool enableTitleChangeNotifications: true
        property bool enableSoundNotifications: true
        property bool enableNotificationCounter: true

        property bool enableScreensaver: true
        property bool disableBackgroundAudio: true

        property bool enableQuickCopy: true
        property bool enableGpu: true
        
        property int updateRevision: 0;
    }
    
 Component.onCompleted: {
   if (config.updateRevision != 1)
   {
    config.textFontSize=106
    config.spanFontSize=107
    config.updateRevision=1;
   }
   
 }
 DownloadHelper {
        id: downloadHelper
        blob_path: localStorage+"/IndexedDB/https_web.whatsapp.com_0.indexeddb.blob/"
    }   
    
    
 Notifier {
    id: notifier
     push_app_id: appID + "_" + hook
 }


  objectName: "mainView"
  applicationName: appID
  backgroundColor : "transparent"
  

  ScreenSaver {
    id: screenSaver
    screenSaverEnabled: !(Qt.application.active) 
  }
    
  ScreenSaverView {
          id: screenSaverView
          visible: (! Qt.application.active) && config.enableScreensaver
  } 
  
  PageStack {
    id: mainPageStack
    visible: Qt.application.active || (! config.enableScreensaver)
    anchors.fill: parent
    Component.onCompleted: mainPageStack.push(pageMain)


    Page {
      id: pageMain
      anchors.fill: parent
      visible: Qt.application.active || (! config.enableScreensaver)
      
      //Webview-----------------------------------------------------------------------------------------------------
      WebEngineView {
        id: webview
        audioMuted: config.disableBackgroundAudio && (!Qt.application.active)   
        visible: Qt.application.active || (! config.enableScreensaver)
        property int keyboardSize: UbuntuApplication.inputMethod.visible ? 10+UbuntuApplication.inputMethod.keyboardRectangle.height/(units.gridUnit / 8) : 0
        anchors{ fill: parent }
        focus: true
        property var currentWebview: webview
        settings.pluginsEnabled: true
        zoomFactor: mainView.width<mainView.height ? Math.round(100 * mainView.width / config.webviewWidthPortait ) / 100 : Math.round(100 * mainView.width / config.webviewWidthLandscape ) / 100
        
        onKeyboardSizeChanged: {
        //Don't hide the text edit with keyboard
        var realKeyboardSize=keyboardSize/zoomFactor
        const jsCode = `document.querySelector('footer').style.paddingBottom = "${realKeyboardSize}px"`;
        webview.runJavaScript(jsCode);
        }
        
        profile:  WebEngineProfile {
          id: webContext
          httpUserAgent: "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.144 Safari/537.36"
          storageName: "Storage"
          persistentStoragePath: localStorage
          //----------------------------------------------------------------------
          //  Notification based on web desktop notifications (Higher priority)
          //----------------------------------------------------------------------   
          onPresentNotification: (notification) => {
            if ( config.enableDesktopNotifications )
                notifier.notifyMain(notification.title, notification.message);
          }
          
          onDownloadRequested: {
              //Not working for now in Qt5
              //Using Download Helper instead
          }
        
        }//End WebEngineProfile
        
  
        
        anchors {
          fill:parent
          centerIn: parent.verticalCenter
        }
       url: "https://web.whatsapp.com"
        userScripts: [
          WebEngineScript {
                name: "AppConfig"
                injectionPoint: WebEngineScript.DocumentCreation
                worldId: WebEngineScript.MainWorld
                sourceCode: "window.appConfig = " + JSON.stringify({
                    textFontSize: config.textFontSize,
                    spanFontSize: config.spanFontSize,
                    enableQuickCopy: config.enableQuickCopy
                }) + ";"
            },
          WebEngineScript {
            injectionPoint: WebEngineScript.DocumentCreation
            worldId: WebEngineScript.MainWorld
            name: "QWebChannel"
            sourceUrl: "ubuntutheme.js"
          }
          ]
        onFileDialogRequested: function(request) {
           request.accepted = true;
          var importPage = mainPageStack.push(Qt.resolvedUrl("ImportPage.qml"),{"contentType": ContentType.All, "handler": ContentHandler.Source})
          importPage.imported.connect(function(fileUrls) {
            var files = []
            var urls = fileUrls.split("\n")
            for (var i = 0; i < urls.length; i++) {
            files.push(urls[i].trim().replace("file://", ""));
            }
            request.dialogAccept(files);
            mainPageStack.pop(importPage)
          })
          importPage.cancel.connect(function() {
            request.dialogAccept("")
            mainPageStack.pop(importPage)
          })          
        }
        onNewViewRequested: {
            request.action = WebEngineNavigationRequest.IgnoreRequest
            //toast.show(request.requestedUrl);
            if(request.userInitiated) {
                Qt.openUrlExternally(request.requestedUrl)
            }
        }
        onFeaturePermissionRequested: {
	    grantFeaturePermission(securityOrigin, feature, true);
        }
        //----------------------------------------------------------------------
        //   Notification based on title changed (medium priority)
        //----------------------------------------------------------------------        
        onTitleChanged: {
             // 1a. look for a number inside parentheses at start or end
            var match = title.match(/^\s*\((\d+)\)/);
            var unread = -1;
            if (match && match.length > 1) {
                unread = parseInt(match[1]);
            }
            if ( unread>lastUnreadCount && unread>0  )
            {
               if ( config.enableTitleChangeNotifications )
                  notifier.triggerDelayedNotification2(unread+" whatsapp message unread");
            }
            lastUnreadCount=unread
            
            if ( unread > 0 && config.enableNotificationCounter )
              notifier.updateCount(unread)
            else
              notifier.updateCount(0)
        }
        


        onJavaScriptConsoleMessage: function(level, message, line, sourceId) {
          //----------------------------------------------------------------------------
          //  Notification based on audio sound file played (Low priority)
          //---------------------------------------------------------------------------
            if (message.startsWith("[DbgAud] https://static.whatsapp.net/")) {
                //Send notification
                if ( config.enableSoundNotifications )
                  notifier.triggerDelayedNotification1("New Whatsapp audio notification");
            }
            if (message.startsWith("[ClipBoardCopy]")) {
              if (config.enableQuickCopy)
              {
                textEdit.text = message.replace(/^\[ClipBoardCopy\]\s*/, "")
                textEdit.selectAll()
                textEdit.copy()
                toast.show("Message copied!")
              }
            }
            if (message.startsWith("[ShowDebug]")) {
                toast.show(message.replace(/^\[ShowDebug\]\s*/, ""))
            }  
            
            //Handle Download when Js tells us a file has been saved to local storage
            if (message.startsWith("[DownloadBlob]")) {
                let output = downloadHelper.getLastDownloaded()
                var exportPage = mainPageStack.push(Qt.resolvedUrl("ExportPage.qml"),{"url": Qt.resolvedUrl("file://"+output),"contentType": ContentType.All})
            } 
            if (message.startsWith("[HideAppControls]")) 
            {
              notificationsHowto.visible= false;
              settingsButton.visible= false;
            }
            if (message.startsWith("[ThemeBackgroundColorDebug]")) {
              
              if ( message.replace(/^\[ThemeBackgroundColorDebug\]\s*/, "") == "#FFFFFF" )
              {
               screenSaverView.backgroundSource="Backgrounds/screensaver.png";
              }
              else
               screenSaverView.backgroundSource="Backgrounds/screensaver-black.png" ;
            }
        }
        
 

      } //End webview--------------------------------------------------------------------------------------------
      

      //Dummy textedit to me able to copy to ClipBoard
     TextEdit{
        id: textEdit
        visible: false
      }
      
    Toast {
    id: toast
    }
    
    NotificationsHowto{
      id: notificationsHowto
      pageStack: mainPageStack
    }
    
    
    SettingsButton{
      id: settingsButton
      pageStack: mainPageStack
    }
      
    }
    
  }
}
