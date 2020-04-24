import QtQuick 2.7
import Ubuntu.Components 1.3
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import io.thp.pyotherside 1.3


MainView {
    id: main_view
    objectName: 'mainView'
    applicationName: 'spotify.alefnode'
    automaticOrientation: true

    property bool log_file: false


    Component.onCompleted: {
        page_stack.push(Qt.resolvedUrl("MainPage.qml"))
    }
    PageStack {
        id: page_stack
    }
}
