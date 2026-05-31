import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.20 as Kirigami

// The grouped overview: one tile per application (logo + name + window count),
// MRU-ordered, with the controller's highlight reflected.
Item {
    id: overview
    property QtObject controller
    property QtObject groupModel

    readonly property int highlighted: controller ? controller.highlightedGroupIndex : 0

    PC3.ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        GridView {
            id: grid
            anchors.fill: parent
            cellWidth: 220
            cellHeight: 180
            currentIndex: overview.highlighted
            model: overview.groupModel
            focus: true

            delegate: Item {
                width: grid.cellWidth
                height: grid.cellHeight

                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 8
                    radius: 10
                    color: index === overview.highlighted
                        ? Kirigami.Theme.highlightColor
                        : Qt.rgba(Kirigami.Theme.backgroundColor.r,
                                  Kirigami.Theme.backgroundColor.g,
                                  Kirigami.Theme.backgroundColor.b, 0.6)

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 8
                        Kirigami.Icon {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: 64
                            Layout.preferredHeight: 64
                            source: model.appIconName
                        }
                        PC3.Label {
                            Layout.alignment: Qt.AlignHCenter
                            text: model.appDisplayName
                            elide: Text.ElideRight
                            Layout.maximumWidth: grid.cellWidth - 32
                        }
                        PC3.Label {
                            Layout.alignment: Qt.AlignHCenter
                            text: model.windowCount + (model.windowCount === 1 ? " window" : " windows")
                            opacity: 0.7
                            font: Kirigami.Theme.smallFont
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: overview.controller.confirm()
                        onDoubleClicked: overview.controller.enterHighlightedGroup()
                    }
                }
            }
        }
    }
}
