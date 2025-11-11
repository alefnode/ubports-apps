import QtQuick 2.9
import Ubuntu.Components 1.3
import Qt.labs.settings 1.1
import "UCSComponents"

Page {
    id: settingsPage
    title: i18n.tr("Configuration")
    
      header: PageHeader {
                id:header
                title: i18n.tr("Settings")          
                leadingActionBar.actions: 
                [
                    Action {
                    iconName: "back"
                    text: i18n.tr("Back")
                    onTriggered: {
                        parent.pageStack.pop()
                        }
                    }
                ]
    }  
    

    // --- Image de fond ---
    Image {
        id: imageBackground
        source: Qt.resolvedUrl("Backgrounds/screensaver-black.png")  // ton image de fond
        anchors.centerIn: parent
        width: parent.width
        height: parent.height
        fillMode: Image.PreserveAspectCrop
    }
    
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
    }
        
 

    Flickable {
    id: flick
     anchors { fill: parent; topMargin: header.height }
    clip: true

    // Dimensions du contenu à défiler
    contentWidth: width
    contentHeight: column.implicitHeight
        Column {
            id:column
            width: parent.width
            spacing: units.gu(2)
            padding: units.gu(1)
            //anchors { fill: parent}
            
             // --- Warning Sections --

                    // Warning 1
                    Row {
                        width: parent ? parent.width : units.gu(40)
                        anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
                        leftPadding: units.gu(2)
                        height: Math.max(imgWarning1.height, labelWarning1.implicitHeight) + units.gu(1)
                        spacing: units.gu(1)
                        Image {
                            source: "Icons/warning.png"
                            width: units.gu(3)
                            height: units.gu(3)
                            fillMode: Image.PreserveAspectFit
                            id:imgWarning1
                        }
                        Label {
                            text: i18n.tr("Warning: changing the following parameters may prevent the app from working correctly. Only modify if you understand what you're doing. You can always go back to default parameters.")
                            wrapMode: Text.Wrap
                            font.bold: true
                            color: "orange"
                            width: parent.width - units.gu(7)
                            id:labelWarning1
                        }
                    }

                    // Warning 2
                    Row {
                        height: Math.max(imgWarning2.height, labelWarning2.implicitHeight) + units.gu(1)
                        width: parent ? parent.width : units.gu(40)
                        anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
                        leftPadding: units.gu(2)
                        spacing: units.gu(1)
                        Image {
                            source: "Icons/warning.png"
                            width: units.gu(3)
                            height: units.gu(3)
                            fillMode: Image.PreserveAspectFit
                            id:imgWarning2
                        }
                        Label {
                            text: i18n.tr("Warning: parameters will only be applied after restarting the app")
                            wrapMode: Text.Wrap
                            font.bold: true
                            color: "orange"
                            width: parent.width - units.gu(7)
                            id:labelWarning2
                        }
                    }
                

    
            Row{
            Button {
            text: i18n.tr("Reset to default values")
            onClicked: {
                config.webviewWidthPortait = 410
                webviewWidthPortait.value = 410
                config.webviewWidthLandscape = 900
                webviewWidthLandscape.value = 900
                config.textFontSize = 106
                textFontSize.value = 106
                config.spanFontSize = 107
                spanFontSize.value = 107

                config.enableDesktopNotifications = true
                enableDesktopNotifications.checked = true
                config.enableTitleChangeNotifications = true
                enableTitleChangeNotifications.checked = true
                config.enableSoundNotifications = true
                enableSoundNotifications.checked = true
                config.enableNotificationCounter = true
                enableNotificationCounter.checked = true

                config.enableScreensaver = true
                enableScreensaver.checked = true
                config.disableBackgroundAudio = true
                disableBackgroundAudio.checked = true

                config.enableQuickCopy = true
                enableQuickCopy.checked = true
                config.enableGpu = true
                enableGpu.checked = true
                }
            }
                
            }
            // --- Scaling ---
            Label { text: i18n.tr("Scaling"); font.bold: true; fontSize: "large"; color: UbuntuColors.orange }
            NumberEditRow { id:webviewWidthPortait; text: i18n.tr("Webview width (portrait)"); value: config.webviewWidthPortait; onValueChanged: config.webviewWidthPortait = value }
            NumberEditRow { id:webviewWidthLandscape; text: i18n.tr("Webview width (landsca.)"); value: config.webviewWidthLandscape; onValueChanged: config.webviewWidthLandscape = value }
            NumberEditRow { id:textFontSize; text: i18n.tr("Text fontsize (%)"); value: config.textFontSize; onValueChanged: config.textFontSize = value }
            NumberEditRow { id:spanFontSize; text: i18n.tr("Span fontsize (%)"); value: config.spanFontSize; onValueChanged: config.spanFontSize = value }

            // --- Notifications ---
            Label { text: i18n.tr("Notifications"); font.bold: true; fontSize: "large"; color: UbuntuColors.orange }
            SwitchRow {  id:enableDesktopNotifications; text: i18n.tr("Enable notifications from desktop not."); checked: config.enableDesktopNotifications; onCheckedChanged: config.enableDesktopNotifications = checked }
            SwitchRow { id:enableTitleChangeNotifications; text: i18n.tr("Enable notifications from title change"); checked: config.enableTitleChangeNotifications; onCheckedChanged: config.enableTitleChangeNotifications = checked }
            SwitchRow { id:enableSoundNotifications; text: i18n.tr("Enable notifications from sound event"); checked: config.enableSoundNotifications; onCheckedChanged: config.enableSoundNotifications = checked }
            SwitchRow { id:enableNotificationCounter; text: i18n.tr("Enable notification counter"); checked: config.enableNotificationCounter; onCheckedChanged: config.enableNotificationCounter = checked }

            // --- Background behaviour ---
            Label { text: i18n.tr("Background behaviour"); font.bold: true; fontSize: "large"; color: UbuntuColors.orange }
            SwitchRow { id:enableScreensaver; text: i18n.tr("Enable screensaver"); checked: config.enableScreensaver; onCheckedChanged: config.enableScreensaver = checked }
            SwitchRow {id:disableBackgroundAudio; text: i18n.tr("Disable background audio"); checked: config.disableBackgroundAudio; onCheckedChanged: config.disableBackgroundAudio = checked }

            // --- Tweaking ---
            Label { text: i18n.tr("Tweaking"); font.bold: true; fontSize: "large"; color: UbuntuColors.orange }
            SwitchRow { id:enableQuickCopy; text: i18n.tr("Enable quick copy to clipboard"); checked: config.enableQuickCopy; onCheckedChanged: config.enableQuickCopy = checked }
            SwitchRow {id:enableGpu;  text: i18n.tr("Enable GPU"); checked: config.enableGpu; onCheckedChanged: config.enableGpu = checked }
        }
       }
}
