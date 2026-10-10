import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page
    property alias view: list

    function moveBy(delta) {
        if (!list.count)
            return
        if (list.currentIndex < 0)
            list.currentIndex = delta > 0 ? 0 : list.count - 1
        else
            list.currentIndex = Math.max(0, Math.min(list.count - 1, list.currentIndex + delta))
        list.positionViewAtIndex(list.currentIndex, ListView.Contain)
    }

    function goEdge(last) {
        if (!list.count)
            return
        list.currentIndex = last ? list.count - 1 : 0
        list.positionViewAtIndex(list.currentIndex, last ? ListView.End : ListView.Beginning)
    }

    function currentRow() {
        if (list.currentIndex < 0 || list.currentIndex >= backend.otherLaunches.length)
            return ({})
        return backend.otherLaunches[list.currentIndex]
    }

    function openCurrent() {
        var row = currentRow()
        if (row && row.webcast)
            backend.openUrl(row.webcast)
    }

    ListView {
        id: list
        anchors.fill: parent
        clip: true
        spacing: 8
        model: backend.otherLaunches
        boundsBehavior: Flickable.StopAtBounds
        highlightFollowsCurrentItem: true
        keyNavigationEnabled: false
        currentIndex: 0

        delegate: Rectangle {
            required property var modelData
            required property int index
            width: ListView.view.width
            height: 96
            radius: 12
            color: backend.themeSurface
            border.color: ListView.isCurrentItem ? backend.themeAccent : "transparent"
            border.width: ListView.isCurrentItem ? 2 : 0

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
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    list.currentIndex = index
                    if ((modelData.webcast || "").length > 0)
                        backend.openUrl(modelData.webcast)
                }
            }
        }

        ScrollBar.vertical: ScrollBar { }
    }
}
