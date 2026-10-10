import QtQuick
import QtQuick.Layouts

Item {
    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        Text {
            text: "SpaceX totals"
            color: backend.themeForeground
            font.pixelSize: 22
            font.bold: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Repeater {
                model: [
                    { label: "Launches", key: "totalLaunches" },
                    { label: "Landings", key: "totalLandings" },
                    { label: "Reflights", key: "totalReflights" }
                ]
                delegate: Rectangle {
                    required property var modelData
                    Layout.fillWidth: true
                    Layout.preferredHeight: 110
                    radius: 14
                    color: backend.themeSurface
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 6
                        Text {
                            text: modelData.label
                            color: backend.themeMuted
                            font.pixelSize: 13
                        }
                        Text {
                            text: String(backend.stats[modelData.key] || 0)
                            color: backend.themeForeground
                            font.pixelSize: 32
                            font.bold: true
                        }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
