import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page
    property var model: backend.missions
    property bool compact: false
    property bool showFilters: true
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

    function currentSlug() {
        if (!list.model || list.currentIndex < 0)
            return ""
        return list.model.slugAt(list.currentIndex)
    }

    function openCurrent() {
        var slug = currentSlug()
        if (slug.length)
            backend.openMission(slug)
    }

    function followCurrent() {
        var slug = currentSlug()
        if (slug.length)
            backend.toggleFollow(slug)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        RowLayout {
            visible: page.showFilters
            Layout.fillWidth: true
            spacing: 8

            Repeater {
                model: [
                    { id: "", label: "All" },
                    { id: "starlink", label: "Starlink" },
                    { id: "starship", label: "Starship" },
                    { id: "hsf", label: "Crew" },
                    { id: "nssl", label: "NSSL" }
                ]
                delegate: Rectangle {
                    required property var modelData
                    radius: 8
                    color: backend.typeFilter === modelData.id ? backend.themeAccent : backend.themeSurface
                    implicitHeight: 28
                    implicitWidth: chipLabel.implicitWidth + 16
                    Text {
                        id: chipLabel
                        anchors.centerIn: parent
                        text: modelData.label
                        color: backend.typeFilter === modelData.id ? backend.themeBackground : backend.themeForeground
                        font.pixelSize: 12
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: backend.typeFilter = modelData.id
                    }
                }
            }

            Item { Layout.fillWidth: true }

            Text {
                text: backend.loading ? "Refreshing…" : (backend.lastUpdated.length ? "Updated " + backend.lastUpdated : "")
                color: backend.themeMuted
                font.pixelSize: 12
            }
        }

        Text {
            visible: backend.error.length > 0
            text: backend.error
            color: backend.themeDanger
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        ListView {
            id: list
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8
            model: page.model
            boundsBehavior: Flickable.StopAtBounds
            highlightFollowsCurrentItem: true
            keyNavigationEnabled: false
            currentIndex: 0

            section.property: "section"
            section.delegate: Text {
                required property string section
                width: list.width
                text: section
                color: backend.themeMuted
                font.pixelSize: 12
                font.bold: true
                topPadding: 10
                bottomPadding: 4
            }

            delegate: MissionCard {
                width: list.width
                compact: page.compact
                current: ListView.isCurrentItem
                slug: model.slug
                title: model.title
                subtitle: model.subtitle
                vehicle: model.vehicle
                launchSite: model.launchSite
                status: model.status
                typeLabel: model.typeLabel
                imageUrl: model.imageUrl
                netMs: model.netMs
                netDisplay: model.netDisplay
                hasNet: model.hasNet
                isLive: model.isLive
                followed: model.followed
            }

            ScrollBar.vertical: ScrollBar { }
        }
    }
}
