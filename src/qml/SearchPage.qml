import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
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
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: backend.searchMissions
            compact: true
            showFilters: false
        }
    }
}
