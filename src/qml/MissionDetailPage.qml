import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Flickable {
    id: page
    clip: true
    contentWidth: width
    contentHeight: column.implicitHeight
    boundsBehavior: Flickable.StopAtBounds

    readonly property var mission: backend.selectedMission

    ColumnLayout {
        id: column
        width: page.width
        spacing: 14

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 220
            radius: 14
            color: backend.themeSurface
            clip: true

            Image {
                anchors.fill: parent
                source: mission.heroUrl || mission.imageUrl || ""
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
            }

            Rectangle {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: 12
                radius: 8
                color: Qt.rgba(0, 0, 0, 0.55)
                implicitWidth: heroTitle.implicitWidth + 16
                implicitHeight: heroTitle.implicitHeight + 12
                visible: mission.title !== undefined
                Text {
                    id: heroTitle
                    anchors.centerIn: parent
                    text: mission.title || ""
                    color: "#ffffff"
                    font.pixelSize: 18
                    font.bold: true
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: "Back"
                onClicked: backend.closeMission()
            }
            Button {
                text: mission.followed ? "Unfollow" : "Follow"
                onClicked: backend.toggleFollow(mission.slug)
            }
            Button {
                text: "SpaceX"
                onClicked: backend.openUrl(mission.spacexUrl)
            }
            Button {
                visible: (mission.webcastUrl || "").length > 0
                text: "Webcast"
                onClicked: backend.openUrl(mission.webcastUrl)
            }
            Item { Layout.fillWidth: true }
        }

        Text {
            visible: mission.hasNet === true && mission.status !== "final"
            text: backend.countdown(mission.netMs, mission.hasNet)
            color: backend.themeAccent
            font.pixelSize: 28
            font.bold: true
            Layout.fillWidth: true
        }

        Text {
            text: mission.netDisplay || ""
            color: backend.themeForeground
            font.pixelSize: 15
            Layout.fillWidth: true
        }

        Text {
            text: [mission.vehicle, mission.launchSite, mission.returnSite, mission.typeLabel]
                  .filter(function(s) { return s && s.length; }).join("  ·  ")
            color: backend.themeMuted
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        Repeater {
            model: backend.selectedParagraphs
            delegate: Text {
                required property var modelData
                Layout.fillWidth: true
                text: modelData
                color: backend.themeForeground
                wrapMode: Text.Wrap
                font.pixelSize: 14
            }
        }

        Repeater {
            model: backend.selectedWebcasts
            delegate: Button {
                required property var modelData
                text: modelData.title || "Webcast"
                onClicked: backend.openUrl(modelData.url)
            }
        }

        Repeater {
            model: backend.selectedTimeline
            delegate: RowLayout {
                required property var modelData
                Layout.fillWidth: true
                spacing: 12
                Text {
                    text: modelData.time
                    color: backend.themeAccent
                    font.pixelSize: 12
                    Layout.preferredWidth: 110
                }
                Text {
                    Layout.fillWidth: true
                    text: modelData.description
                    color: backend.themeForeground
                    wrapMode: Text.Wrap
                    font.pixelSize: 13
                }
            }
        }
    }

    Timer {
        interval: 1000
        running: page.visible && mission.hasNet === true && mission.status !== "final"
        repeat: true
        onTriggered: page.contentHeight = column.implicitHeight
    }

    ScrollBar.vertical: ScrollBar { }
}
