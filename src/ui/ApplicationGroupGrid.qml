import QtQuick 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3

// In-group view: a uniform, scrollable grid of live window thumbnails for the
// open application (FR-006/007). Selection follows the controller.
Item {
    id: groupGrid
    property QtObject controller
    property QtObject entryModel

    readonly property int highlighted: controller ? controller.highlightedWindowIndex : 0

    PC3.ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        GridView {
            id: grid
            anchors.fill: parent
            cellWidth: 320
            cellHeight: 220
            currentIndex: groupGrid.highlighted
            model: groupGrid.entryModel
            focus: true
            // Keep the highlighted tile visible while cycling (FR-007).
            highlightRangeMode: GridView.ApplyRange

            delegate: WindowTile {
                width: grid.cellWidth
                height: grid.cellHeight
                selected: index === groupGrid.highlighted
                caption: model.caption
                iconName: model.iconName
                minimized: model.minimized
                windowId: model.internalId
                onClicked: groupGrid.controller.confirm()
            }
        }
    }
}
