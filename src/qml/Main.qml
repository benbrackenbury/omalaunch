import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window
import "keys/Keymap.js" as Keymap

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
    property bool helpVisible: false

    readonly property var navItems: {
        var all = [
            { id: 0, label: "Missions" },
            { id: 1, label: "Other", other: true },
            { id: 2, label: "Stats" },
            { id: 3, label: "Starship" },
            { id: 4, label: "Search" },
            { id: 5, label: "Settings" }
        ]
        var visible = []
        var slot = 1
        for (var i = 0; i < all.length; i++) {
            if (all[i].other && !backend.showOtherLaunches)
                continue
            visible.push({
                id: all[i].id,
                label: all[i].label,
                slot: slot
            })
            slot += 1
        }
        return visible
    }

    readonly property var activeList: {
        if (win.currentPage === 0) return missionsPage
        if (win.currentPage === 1) return otherPage
        if (win.currentPage === 3) return starshipPage
        if (win.currentPage === 4) return searchPage
        return null
    }

    readonly property string keyContext: {
        if (searchPage.fieldFocused) return "search"
        if (settingsPage.fieldFocused) return "settings"
        if (win.detailOpen) return "reader"
        if (win.currentPage === 5) return "settings"
        if (win.currentPage === 2) return "page"
        return "list"
    }

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
        parkKeyboard()
    }

    onClosing: backend.saveWindowGeometry(x, y, width, height, visibility === Window.Maximized)
    onKeyContextChanged: Qt.callLater(applyContextFocus)

    function parkKeyboard() {
        keyboardHome.forceActiveFocus()
    }

    function applyContextFocus() {
        if (win.keyContext === "search")
            searchPage.queryField.forceActiveFocus()
        else if (win.keyContext === "settings" && settingsPage.fieldFocused)
            return
        else
            parkKeyboard()
    }

    function goPage(pageId) {
        win.helpVisible = false
        win.currentPage = pageId
        backend.closeMission()
        if (pageId === 3)
            backend.startTelemetry()
        else
            backend.stopTelemetry()
        if (pageId === 4)
            Qt.callLater(function() { searchPage.queryField.forceActiveFocus() })
        else
            parkKeyboard()
    }

    function runShortcut(id, sequence) {
        if (win.helpVisible && (id === "cursorDown" || id === "scrollDown")) {
            shortcutHelp.scrollBy(1)
            return
        }
        if (win.helpVisible && (id === "cursorUp" || id === "scrollUp")) {
            shortcutHelp.scrollBy(-1)
            return
        }

        if (id === "help") {
            win.helpVisible = !win.helpVisible
            if (!win.helpVisible)
                parkKeyboard()
            return
        }
        if (id === "quit") {
            win.close()
            return
        }
        if (id === "refresh") {
            backend.refresh()
            return
        }
        if (id === "settings") {
            goPage(5)
            return
        }
        if (id === "search") {
            goPage(4)
            return
        }
        if (id === "goPage") {
            var slot = Keymap.slotFor(id, sequence)
            if (slot >= 0 && slot < win.navItems.length)
                goPage(win.navItems[slot].id)
            return
        }
        if (id === "back") {
            if (win.helpVisible) {
                win.helpVisible = false
                parkKeyboard()
            } else if (win.detailOpen) {
                backend.closeMission()
                parkKeyboard()
            } else if (win.keyContext === "search" || win.keyContext === "settings") {
                parkKeyboard()
            }
            return
        }
        if (id === "backToList") {
            backend.closeMission()
            parkKeyboard()
            return
        }
        if (id === "scrollDown") {
            detailPage.scrollBy(1)
            return
        }
        if (id === "scrollUp") {
            detailPage.scrollBy(-1)
            return
        }
        if (id === "filter" && win.currentPage === 0 && !win.detailOpen) {
            var filters = ["", "starlink", "starship", "hsf", "nssl"]
            var index = Keymap.slotFor(id, sequence)
            if (index >= 0 && index < filters.length)
                backend.typeFilter = filters[index]
            return
        }

        var list = win.activeList
        if (id === "cursorDown" || id === "cursorUp") {
            if (!list)
                return
            list.moveBy(id === "cursorDown" ? 1 : -1)
            if (win.detailOpen && list.currentSlug)
                list.openCurrent()
            return
        }
        if (id === "top" || id === "bottom") {
            if (!list)
                return
            list.goEdge(id === "bottom")
            if (win.detailOpen && list.openCurrent)
                list.openCurrent()
            return
        }
        if (id === "open" || id === "openReader") {
            if (list && list.openCurrent)
                list.openCurrent()
            return
        }
        if (id === "follow") {
            var slug = win.detailOpen ? backend.selectedMission.slug
                      : (list && list.currentSlug ? list.currentSlug() : "")
            if (slug)
                backend.toggleFollow(slug)
            return
        }
        if (id === "webcast") {
            if (win.detailOpen)
                backend.openUrl(backend.selectedMission.webcastUrl)
            return
        }
        if (id === "spacex") {
            if (win.detailOpen)
                backend.openUrl(backend.selectedMission.spacexUrl)
            return
        }
    }

    FocusScope {
        id: focusScope
        anchors.fill: parent
        focus: true
        property bool ctrlDown: false
        readonly property bool ctrlHeld: ctrlDown && activeFocus

        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Control)
                focusScope.ctrlDown = true
        }
        Keys.onReleased: function(event) {
            if (event.key === Qt.Key_Control)
                focusScope.ctrlDown = false
        }
        onActiveFocusChanged: if (!activeFocus) ctrlDown = false

        Item {
            id: keyboardHome
            width: 0
            height: 0
            focus: true
            Keys.onPressed: function(event) {
                if (event.key === Qt.Key_Control)
                    focusScope.ctrlDown = true
            }
            Keys.onReleased: function(event) {
                if (event.key === Qt.Key_Control)
                    focusScope.ctrlDown = false
            }
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
                        model: win.navItems
                        delegate: Rectangle {
                            required property var modelData
                            Layout.fillWidth: true
                            radius: 8
                            implicitHeight: 36
                            color: win.currentPage === modelData.id ? backend.themeSelection : "transparent"

                            Rectangle {
                                visible: focusScope.ctrlHeld
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.leftMargin: 8
                                width: 18
                                height: 18
                                radius: 4
                                color: backend.themeAccent
                                Text {
                                    anchors.centerIn: parent
                                    text: String(modelData.slot)
                                    color: backend.themeBackground
                                    font.pixelSize: 11
                                    font.bold: true
                                }
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.leftMargin: focusScope.ctrlHeld ? 32 : 12
                                text: modelData.label
                                color: backend.themeForeground
                                font.pixelSize: 14
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: win.goPage(modelData.id)
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

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 0

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    StackLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        currentIndex: win.detailOpen ? 6 : win.currentPage

                        MissionListPage { id: missionsPage }
                        OtherPage { id: otherPage }
                        StatsPage { }
                        StarshipPage { id: starshipPage }
                        SearchPage { id: searchPage }
                        SettingsPage { id: settingsPage }
                        MissionDetailPage { id: detailPage }
                    }
                }

                KeyHints {
                    Layout.fillWidth: true
                    Layout.leftMargin: 20
                    Layout.rightMargin: 20
                    Layout.bottomMargin: 12
                    textColor: backend.themeForeground
                    dimColor: backend.themeMuted
                    fillColor: backend.themeSurface
                    hints: Keymap.hintsFor(win.keyContext)
                }
            }
        }

        ShortcutHelp {
            id: shortcutHelp
            anchors.fill: parent
            visible: win.helpVisible
            textColor: backend.themeForeground
            backgroundColor: backend.themeBackground
            dimColor: backend.themeMuted
            onDismissed: {
                win.helpVisible = false
                win.parkKeyboard()
            }
        }
    }

    KeyRouter {
        context: win.keyContext
        overlay: win.helpVisible
        onTriggered: function(id, sequence) { win.runShortcut(id, sequence) }
    }
}
