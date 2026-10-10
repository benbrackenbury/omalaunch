import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page
    property alias queryField: query
    property alias view: listPage.view
    readonly property bool fieldFocused: query.activeFocus

    function moveBy(delta) { listPage.moveBy(delta) }
    function goEdge(last) { listPage.goEdge(last) }
    function openCurrent() { listPage.openCurrent() }
    function followCurrent() { listPage.followCurrent() }
    function currentSlug() { return listPage.currentSlug() }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        TextField {
            id: query
            Layout.fillWidth: true
            placeholderText: "Search missions"
            color: backend.themeForeground
            onTextChanged: backend.searchQuery = text
        }

        Text {
            visible: query.text.length > 0
            text: backend.searchMissions.count + " matches"
            color: backend.themeMuted
        }

        MissionListPage {
            id: listPage
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: backend.searchMissions
            compact: true
            showFilters: false
        }
    }
}
