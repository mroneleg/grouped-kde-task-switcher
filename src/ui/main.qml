import QtQuick 2.15
import org.kde.plasma.core 2.0 as PlasmaCore

// Root of the per-screen QuickSceneView. Properties are injected by
// GroupedSwitcherEffect::initialProperties().
FocusScope {
    id: root
    focus: true

    property QtObject effect
    property QtObject controller
    property QtObject groupModel
    property QtObject entryModel

    // Level enum: 0 = Closed, 1 = Overview, 2 = InGroup (SessionController::Level).
    readonly property int level: controller ? controller.level : 0

    // Dimmed backdrop; clicking it cancels.
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.5)
        MouseArea {
            anchors.fill: parent
            onClicked: if (root.controller) root.controller.cancel()
        }
    }

    PlasmaCore.ColorScope {
        id: scope
        anchors.fill: parent
        colorGroup: PlasmaCore.Theme.NormalColorGroup // follows system light/dark live

        Loader {
            id: content
            anchors.centerIn: parent
            width: Math.min(parent.width * 0.9, 1600)
            height: Math.min(parent.height * 0.85, 1000)
            sourceComponent: root.level === 2 ? gridComponent : overviewComponent
        }

        Component {
            id: overviewComponent
            GroupOverview {
                controller: root.controller
                groupModel: root.groupModel
            }
        }
        Component {
            id: gridComponent
            ApplicationGroupGrid {
                controller: root.controller
                entryModel: root.entryModel
            }
        }
    }

    // Persistent input: the overlay stays open until confirm/cancel; the bound
    // shortcut advances rather than closes (FR-008/009).
    Keys.onPressed: function (event) {
        if (!root.controller) {
            return;
        }
        switch (event.key) {
        case Qt.Key_Escape:
            root.controller.cancel();
            event.accepted = true;
            break;
        case Qt.Key_Return:
        case Qt.Key_Enter:
        case Qt.Key_Space:
            root.controller.confirm();
            event.accepted = true;
            break;
        case Qt.Key_Tab:
        case Qt.Key_Right:
            root.controller.advanceHighlight();
            event.accepted = true;
            break;
        case Qt.Key_Backtab:
        case Qt.Key_Left:
            root.controller.retreatHighlight();
            event.accepted = true;
            break;
        case Qt.Key_Down:
            if (root.level === 1) {
                root.controller.enterHighlightedGroup();
            } else {
                root.controller.advanceHighlight();
            }
            event.accepted = true;
            break;
        case Qt.Key_Up:
            if (root.level === 2) {
                root.controller.back();
            }
            event.accepted = true;
            break;
        default:
            break;
        }
    }
}
