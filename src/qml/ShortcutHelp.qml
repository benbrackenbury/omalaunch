import QtQuick
import QtQuick.Controls
import "keys/Keymap.js" as Keymap

Rectangle {
    id: root

    required property color textColor
    required property color backgroundColor
    required property color dimColor

    signal dismissed()

    function scrollBy(steps) {
        if (!scroller.interactive)
            return
        var next = scroller.contentY + steps * 24
        var max = Math.max(0, scroller.contentHeight - scroller.height)
        scroller.contentY = Math.max(0, Math.min(max, next))
    }

    readonly property real columnWidth: 280
    readonly property int columnCount: Math.max(1, Math.min(3,
        Math.floor((width - 48) / columnWidth)))
    readonly property var columns: Keymap.helpColumns(columnCount, [])

    color: Qt.rgba(backgroundColor.r, backgroundColor.g, backgroundColor.b, 0.96)

    MouseArea {
        anchors.fill: parent
        onClicked: root.dismissed()
    }

    Flickable {
        id: scroller
        anchors.fill: parent
        anchors.margins: 24
        contentWidth: width
        contentHeight: holder.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        interactive: contentHeight > height
        ScrollBar.vertical: ScrollBar { }

        TapHandler {
            onTapped: root.dismissed()
        }

        Item {
            id: holder
            width: scroller.width
            implicitHeight: Math.max(column.implicitHeight, scroller.height)

            Column {
                id: column
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                width: Math.min(scroller.width,
                    root.columnCount * root.columnWidth + (root.columnCount - 1) * 28)
                spacing: 8

                Text {
                    text: "Keyboard shortcuts"
                    color: root.textColor
                    font.pixelSize: 22
                    font.bold: true
                }

                Text {
                    text: "Hold Ctrl to see sidebar numbers. Press ? or Esc to close."
                    color: root.dimColor
                    font.pixelSize: 12
                }

                Row {
                    width: parent.width
                    spacing: 28

                    Repeater {
                        model: root.columns

                        Column {
                            id: sheetColumn
                            required property var modelData
                            width: (column.width - (root.columnCount - 1) * 28) / root.columnCount
                            spacing: 8

                            Repeater {
                                model: sheetColumn.modelData

                                Column {
                                    id: group
                                    required property var modelData
                                    width: sheetColumn.width
                                    spacing: 6

                                    Text {
                                        text: group.modelData.name
                                        color: root.dimColor
                                        font.pixelSize: 11
                                        font.bold: true
                                        topPadding: 10
                                    }

                                    Repeater {
                                        model: group.modelData.rows

                                        Item {
                                            required property var modelData
                                            width: group.width
                                            implicitHeight: 22

                                            Text {
                                                anchors.left: parent.left
                                                anchors.verticalCenter: parent.verticalCenter
                                                width: Math.round(parent.width * 0.42)
                                                text: Keymap.keycapLabel(modelData.keys)
                                                color: root.textColor
                                                font.pixelSize: 13
                                                elide: Text.ElideRight
                                            }

                                            Text {
                                                anchors.left: parent.left
                                                anchors.leftMargin: Math.round(parent.width * 0.42) + 8
                                                anchors.right: parent.right
                                                anchors.verticalCenter: parent.verticalCenter
                                                text: modelData.action
                                                color: root.dimColor
                                                font.pixelSize: 13
                                                elide: Text.ElideRight
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
