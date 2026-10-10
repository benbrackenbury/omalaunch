import QtQuick
import QtQml
import "keys/Keymap.js" as Keymap

Item {
    id: root

    property string context: "list"
    property bool overlay: false

    signal triggered(string id, string sequence)

    Instantiator {
        model: Keymap.sequencesFor(root.context)

        delegate: Shortcut {
            required property var modelData
            sequence: modelData.sequence
            context: Qt.WindowShortcut
            enabled: Keymap.isSequenceEnabled(modelData.binding, modelData.sequence,
                                              root.context, root.overlay)
            onActivated: root.triggered(modelData.id, modelData.sequence)
        }
    }
}
