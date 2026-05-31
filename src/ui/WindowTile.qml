import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kwin 3.0 as KWinComponents

// One window thumbnail tile. The live thumbnail is composited by KWin, so the
// title/selection chrome sits BESIDE it, never overlaid (research §4). Minimized
// windows have no live texture, so we fall back to the application icon.
Item {
    id: tile

    property bool selected: false
    property string caption: ""
    property string iconName: ""
    property bool minimized: false
    property var windowId

    signal clicked()

    Rectangle {
        anchors.fill: parent
        anchors.margins: 8
        radius: 10
        color: "transparent"
        border.width: tile.selected ? 3 : 0
        border.color: Kirigami.Theme.highlightColor

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 6

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                // Live thumbnail for mapped windows…
                KWinComponents.WindowThumbnailItem {
                    anchors.fill: parent
                    visible: !tile.minimized
                    wId: tile.windowId
                }
                // …icon fallback when minimized / no texture available.
                Kirigami.Icon {
                    anchors.centerIn: parent
                    width: 96
                    height: 96
                    visible: tile.minimized
                    source: tile.iconName
                }
            }

            PC3.Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: tile.caption
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: tile.clicked()
        }
    }
}
