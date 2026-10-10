import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ListView {
    clip: true
    spacing: 8
    model: backend.otherLaunches
    boundsBehavior: Flickable.StopAtBounds

    delegate: Rectangle {
        required property var modelData
        width: ListView.view.width
        height: 96
        radius: 12
        color: backend.themeSurface

        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            Rectangle {
                Layout.preferredWidth: 120
                Layout.fillHeight: true
                radius: 8
                color: backend.themeBackground
                clip: true
                Image {
                    anchors.fill: parent
                    source: modelData.image || ""
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Text {
                    text: modelData.name
                    color: backend.themeForeground
                    font.bold: true
                    font.pixelSize: 15
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                Text {
                    text: [modelData.provider, modelData.vehicle].filter(function(s) { return s && s.length; }).join("  ·  ")
                    color: backend.themeMuted
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
                Text {
                    text: modelData.netDisplay
                    color: backend.themeAccent
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            enabled: (modelData.webcast || "").length > 0
            cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: backend.openUrl(modelData.webcast)
        }
    }

    ScrollBar.vertical: ScrollBar { }
}
