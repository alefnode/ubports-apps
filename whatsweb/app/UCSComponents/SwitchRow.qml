import QtQuick 2.9
import Ubuntu.Components 1.3

Row {
    property alias text: label.text
    property alias checked: switcher.checked

    height: Math.max(label.implicitHeight, switcher.implicitHeight) + units.gu(0.75) * 2    
    width: parent ? parent.width : units.gu(40)
    spacing: units.gu(2)
    leftPadding: units.gu(5)    
    Label {
        id: label
        text: "Option"
        wrapMode: Text.Wrap
        color: "#ffffff"
    }
    Switch {
        id:switcher
    }
}
 
