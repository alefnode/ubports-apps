import QtQuick 2.9
import Ubuntu.Components 1.3

Row {
    id: root
    property string text: "Option"
    property int value 
    property int minimumValue: 50
    property int maximumValue: 5000
    property int stepSize: 1

    width: parent ? parent.width : units.gu(40)
    spacing: units.gu(1)
    anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
    leftPadding: units.gu(5)

    Label {
        id:label
        text: root.text
        width: units.gu(20)
        verticalAlignment: Text.AlignVCenter
        color: "#ffffff"
    }

    Button {
        text: "-"
         width: units.gu(3)
        onClicked: {
            if (root.value - root.stepSize >= root.minimumValue) {
                root.value -= root.stepSize
                root.valueChanged(root.value)
            }
        }
    }

    TextEdit {
        id:textEdit1
        text: root.value.toString()
        width: units.gu(8)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: "#ffffff"
        Keys.onReturnPressed: {
        if (parseInt(text)>root.minimumValue && parseInt(text)<root.maximumValue)
                value=parseInt(text)
        root.valueChanged(root.value)
        event.accepted = true   // empêche l’ajout d’un saut de ligne
        focus = false           // déselectionne le TextEdit
        }
        onFocusChanged: {
            if (parseInt(text)>root.minimumValue && parseInt(text)<root.maximumValue)
                    value=parseInt(text)
            root.valueChanged(root.value)
        }
    }

    Button {
        text: "+"
        width: units.gu(3)
        onClicked: {
            if (root.value + root.stepSize <= root.maximumValue) {
                root.value += root.stepSize
                root.valueChanged(root.value)
            }
        }
    }

    height: Math.max(label.implicitHeight, Button.implicitHeight) + units.gu(1)
}
