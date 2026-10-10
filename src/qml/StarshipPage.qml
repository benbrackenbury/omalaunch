import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        Text {
            text: "Starbase"
            color: backend.themeForeground
            font.pixelSize: 22
            font.bold: true
        }

        Repeater {
            model: backend.telemetry
            delegate: Rectangle {
                required property var modelData
                Layout.fillWidth: true
                radius: 12
                color: backend.themeSurface
                implicitHeight: 72
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    Text {
                        text: modelData.id
                        color: backend.themeAccent
                        font.bold: true
                        Layout.preferredWidth: 110
                    }
                    Text {
                        text: Math.round(modelData.altitude) + " m"
                        color: backend.themeForeground
                    }
                    Text {
                        text: Math.round(modelData.speed) + " m/s"
                        color: backend.themeForeground
                    }
                    Text {
                        text: "T+" + Math.round(modelData.missionTime) + "s"
                        color: backend.themeMuted
                    }
                    Item { Layout.fillWidth: true }
                }
            }
        }

        Text {
            visible: backend.roadClosures.length === 0 && backend.telemetry.length === 0
            text: "No road closures or live vehicles right now."
            color: backend.themeMuted
        }

        Repeater {
            model: backend.roadClosures
            delegate: Rectangle {
                required property var modelData
                Layout.fillWidth: true
                radius: 12
                color: backend.themeSurface
                implicitHeight: 64
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    Text {
                        text: modelData.title
                        color: backend.themeForeground
                        font.bold: true
                    }
                    Text {
                        text: modelData.dateText
                        color: backend.themeMuted
                    }
                }
            }
        }

        MissionListPage {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: backend.starshipMissions
            compact: true
            showFilters: false
        }
    }

}
