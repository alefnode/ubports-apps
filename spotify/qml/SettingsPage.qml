import QtQuick 2.4
import Ubuntu.Components 1.3

Page {
    id: root_settings

    header: PageHeader {
        id: main_header
        title: i18n.tr("Settings")
    }
    Column {
        anchors {
            top: main_header.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        ListItemLayout {
            title.text: i18n.tr("Vibration")

            Switch {
                checked: vibOn
                onCheckedChanged: vibOn = checked
            }
        }

        ListItemLayout {
            title.text: i18n.tr("Sound")

            Switch {
                checked: soundOn
                onCheckedChanged: soundOn = checked
            }
        }
    }
}
