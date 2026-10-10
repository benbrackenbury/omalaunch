import QtQuick
import "keys/Keymap.js" as Keymap

Row {
    id: root

    required property color textColor
    required property color dimColor
    required property color fillColor
    property var hints: []
    spacing: 10

    Repeater {
        model: root.hints

        delegate: Row {
            id: pair
            required property var modelData
            spacing: 6

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: cap.implicitWidth + 10
                height: cap.implicitHeight + 4
                radius: 4
                color: root.fillColor

                Text {
                    id: cap
                    anchors.centerIn: parent
                    text: Keymap.keycapLabel(pair.modelData.key)
                    color: root.textColor
                    font.pixelSize: 11
                }
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: pair.modelData.label
                color: root.dimColor
                font.pixelSize: 11
            }
        }
    }
}
