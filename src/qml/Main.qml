import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window

ApplicationWindow {
    id: win
    width: 1120
    height: 740
    minimumWidth: 780
    minimumHeight: 520
    visible: true
    title: "Redstone"

    readonly property bool darkMode: backend.darkMode
    property int currentPage: 0
    property bool detailOpen: Object.keys(backend.selectedMission).length > 0

    Material.theme: darkMode ? Material.Dark : Material.Light
    Material.accent: backend.themeAccent
    Material.background: backend.themeBackground
    Material.foreground: backend.themeForeground
    color: backend.themeBackground

    Component.onCompleted: {
        var geo = backend.windowGeometry();
        if (geo.width && geo.height) {
            x = geo.x;
            y = geo.y;
            width = geo.width;
            height = geo.height;
            if (geo.maximized)
                visibility = Window.Maximized;
        }
    }

    onClosing: backend.saveWindowGeometry(x, y, width, height, visibility === Window.Maximized)

    Shortcut {
        sequence: "Ctrl+Q"
        context: Qt.ApplicationShortcut
        onActivated: win.close()
    }

    Shortcut {
        sequences: ["Ctrl+R", "F5"]
        context: Qt.ApplicationShortcut
        onActivated: backend.refresh()
    }

    Shortcut {
        sequence: "Escape"
        onActivated: backend.closeMission()
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 196
            color: backend.themeSurface

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 6

                Text {
                    text: "Redstone"
                    color: backend.themeForeground
                    font.pixelSize: 20
                    font.bold: true
                    Layout.bottomMargin: 10
                }

                Repeater {
                    model: [
                        { id: 0, label: "Missions" },
                        { id: 1, label: "Other", other: true },
                        { id: 2, label: "Stats" },
                        { id: 3, label: "Starship" },
                        { id: 4, label: "Search" },
                        { id: 5, label: "Settings" }
                    ]
                    delegate: Rectangle {
                        required property var modelData
                        visible: !modelData.other || backend.showOtherLaunches
                        Layout.fillWidth: true
                        radius: 8
                        implicitHeight: 36
                        color: win.currentPage === modelData.id ? backend.themeSelection : "transparent"
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            text: modelData.label
                            color: backend.themeForeground
                            font.pixelSize: 14
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                win.currentPage = modelData.id;
                                backend.closeMission();
                                if (modelData.id === 3)
                                    backend.startTelemetry();
                                else
                                    backend.stopTelemetry();
                            }
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                Text {
                    text: backend.loading ? "Refreshing…" : ""
                    color: backend.themeMuted
                    font.pixelSize: 11
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            StackLayout {
                anchors.fill: parent
                anchors.margins: 20
                currentIndex: win.detailOpen ? 6 : win.currentPage

                MissionListPage { }
                OtherPage { }
                StatsPage { }
                StarshipPage { }
                SearchPage { }
                SettingsPage { }
                MissionDetailPage { }
            }
        }
    }
}
