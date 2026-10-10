import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: card

    property string slug
    property string title
    property string subtitle
    property string vehicle
    property string launchSite
    property string status
    property string typeLabel
    property string imageUrl
    property var netMs: 0
    property string netDisplay
    property bool hasNet: false
    property bool isLive: false
    property bool followed: false
    property bool compact: false
    property bool current: false

    width: ListView.view ? ListView.view.width : 400
    height: compact ? 88 : 118
    radius: 12
    color: backend.themeSurface
    border.color: current || followed ? backend.themeAccent : "transparent"
    border.width: current ? 2 : (followed ? 1 : 0)

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 12

        Rectangle {
            Layout.preferredWidth: compact ? 88 : 140
            Layout.fillHeight: true
            radius: 8
            color: backend.themeBackground
            clip: true

            Image {
                anchors.fill: parent
                source: card.imageUrl
                fillMode: Image.PreserveAspectCrop
                asynchronous: true
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4

            RowLayout {
                Layout.fillWidth: true
                Text {
                    Layout.fillWidth: true
                    text: card.title
                    color: backend.themeForeground
                    font.pixelSize: 16
                    font.bold: true
                    elide: Text.ElideRight
                }
                Rectangle {
                    visible: card.isLive || card.status === "in-progress"
                    radius: 4
                    color: backend.themeDanger
                    implicitHeight: liveLabel.implicitHeight + 4
                    implicitWidth: liveLabel.implicitWidth + 10
                    Text {
                        id: liveLabel
                        anchors.centerIn: parent
                        text: card.isLive ? "LIVE" : "In Progress"
                        color: backend.themeBackground
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                text: [card.vehicle, card.launchSite].filter(function(s) { return s && s.length; }).join("  ·  ")
                color: backend.themeMuted
                font.pixelSize: 12
                elide: Text.ElideRight
            }

            Text {
                Layout.fillWidth: true
                text: card.netDisplay
                color: backend.themeMuted
                font.pixelSize: 12
            }

            Text {
                visible: card.hasNet && card.status !== "final"
                text: countdownLabel.text
                color: backend.themeAccent
                font.pixelSize: 13
                font.bold: true
            }
        }
    }

    Text {
        id: countdownLabel
        visible: false
        text: backend.countdown(card.netMs, card.hasNet)
    }

    Timer {
        interval: 1000
        running: card.visible && card.hasNet && card.status !== "final"
        repeat: true
        onTriggered: countdownLabel.text = backend.countdown(card.netMs, card.hasNet)
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: backend.openMission(card.slug)
    }
}
