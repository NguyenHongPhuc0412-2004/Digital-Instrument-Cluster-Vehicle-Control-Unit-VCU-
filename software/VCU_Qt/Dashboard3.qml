import QtQuick 2.9
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Style 1.0
import QtGraphicalEffects 1.15
import "Components"
import "qrc:/LayoutManager.js" as Responsive

Rectangle {
    id: root
    anchors.fill: parent
    color: "#171717"

    property var adaptive: new Responsive.AdaptiveLayoutManager(1920, 1200, width, height)

    onWidthChanged: {
        if(adaptive)
            adaptive.updateWindowWidth(root.width)
    }

    onHeightChanged: {
        if(adaptive)
            adaptive.updateWindowHeight(root.height)
    }

    // Background - switch between map view and car view
    Loader {
        id: backgroundLoader
        anchors.fill: parent
        sourceComponent: Style.mapAreaVisible ? backgroundRect : backgroundImage
    }

    // Header
    Header {
        z: 99
        id: headerLayout
        width: parent.width
        anchors.top: parent.top
    }

    // Footer
    Footer {
        id: footerLayout
        width: parent.width
        anchors.bottom: parent.bottom
        onOpenLauncher: launcher.open()
    }

    // Left side icon column (always visible on car view)
    TopLeftButtonIconColumn {
        z: 99
        visible: !Style.mapAreaVisible
        anchors.left: parent.left
        anchors.top: headerLayout.bottom
        anchors.leftMargin: 18
    }

    // Map layout (visible when Style.mapAreaVisible is true)
    RowLayout {
        id: mapLayout
        visible: Style.mapAreaVisible
        spacing: 0
        anchors.fill: parent
        anchors.topMargin: headerLayout.height
        anchors.bottomMargin: footerLayout.height

        Item {
            Layout.preferredWidth: 620
            Layout.fillHeight: true
            Image {
                anchors.centerIn: parent
                source: Style.isDark ? "qrc:/icons/light/sidebar.png" : "qrc:/icons/dark/sidebar-light.png"
            }
        }

        NavigationMapHelperScreen {
            Layout.fillWidth: true
            Layout.fillHeight: true
            runMenuAnimation: true
        }
    }

    // Launcher popup
    LaunchPadControl {
        id: launcher
        y: (root.height - height) / 2 + (footerLayout.height)
        x: (root.width - width) / 2
    }

    // Background Rectangle Component (for map view)
    Component {
        id: backgroundRect
        Rectangle {
            color: "#171717"
            anchors.fill: parent
        }
    }

    // Background Image Component (for car view)
    Component {
        id: backgroundImage
        Image {
            id: carBackground
            anchors.fill: parent
            source: Style.getImageBasedOnTheme()
            fillMode: Image.PreserveAspectFit

            // Lock icon on top of car
            Icon {
                icon.source: Style.isDark ? "qrc:/icons/car_action_icons/dark/lock.svg" : "qrc:/icons/car_action_icons/lock.svg"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: -350
                anchors.horizontalCenterOffset: 37
            }

            // Power/charging icon on right side
            Icon {
                icon.source: Style.isDark ? "qrc:/icons/car_action_icons/dark/Power.svg" : "qrc:/icons/car_action_icons/Power.svg"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: -77
                anchors.horizontalCenterOffset: 550
            }

            // Trunk control label (right side, upper)
            ColumnLayout {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: -230
                anchors.horizontalCenterOffset: 440
                spacing: 5

                Text {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    text: "Trunk"
                    font.family: "Inter"
                    font.pixelSize: 14
                    font.bold: Font.DemiBold
                    color: Style.black20
                }
                Text {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    text: "Open"
                    font.family: "Inter"
                    font.pixelSize: 16
                    font.bold: Font.Bold
                    color: Style.isDark ? Style.white : "#171717"
                }
            }

            // Frunk control label (left side, upper)
            ColumnLayout {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: -180
                anchors.horizontalCenterOffset: -350
                spacing: 5

                Text {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    text: "Frunk"
                    font.family: "Inter"
                    font.pixelSize: 14
                    font.bold: Font.DemiBold
                    color: Style.black20
                }
                Text {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    text: "Open"
                    font.family: "Inter"
                    font.pixelSize: 16
                    font.bold: Font.Bold
                    color: Style.isDark ? Style.white : "#171717"
                }
            }
        }
    }
}
