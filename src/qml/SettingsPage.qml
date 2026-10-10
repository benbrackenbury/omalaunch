import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Flickable {
    clip: true
    contentWidth: width
    contentHeight: column.implicitHeight

    ColumnLayout {
        id: column
        width: parent.width
        spacing: 14

        Text {
            text: "Settings"
            color: backend.themeForeground
            font.pixelSize: 22
            font.bold: true
        }

        Text {
            text: "redstone-server"
            color: backend.themeMuted
        }

        TextField {
            Layout.fillWidth: true
            text: backend.snapshotUrl
            color: backend.themeForeground
            onEditingFinished: backend.snapshotUrl = text
        }

        Switch {
            text: "Use UTC"
            checked: backend.useUtc
            onToggled: backend.useUtc = checked
        }

        Switch {
            text: "24-hour time"
            checked: backend.use24Hour
            onToggled: backend.use24Hour = checked
        }

        Switch {
            text: "Show other launches"
            checked: backend.showOtherLaunches
            onToggled: backend.showOtherLaunches = checked
        }

        Button {
            text: backend.loading ? "Refreshing…" : "Refresh now"
            enabled: !backend.loading
            onClicked: backend.refresh()
        }

        Text {
            text: "Omarchy theme colors are read live from ~/.local/state/omarchy/current/theme/colors.toml"
            color: backend.themeMuted
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
    }
}
