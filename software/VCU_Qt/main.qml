import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12

ApplicationWindow {
    id: root
    width: 1920
    height: 960
    visible: true
    title: qsTr("Car Dashboard")
    color: "#1E1E1E"
    visibility: "FullScreen"

    property bool showWelcome: true
    property var dashboardList: [
        "qrc:/Dashboard1.qml",
        "qrc:/Dashboard2.qml",
        "qrc:/Dashboard3.qml"
    ]
    property int currentIndex: 0

    // Welcome Screen
    WelcomeScreen {
        id: welcomeScreen
        visible: showWelcome
        anchors.fill: parent
        z: 2

        function switchToDashboard() {
            welcomeFadeOut.start()
        }
    }

    OpacityAnimator {
        id: welcomeFadeOut
        target: welcomeScreen
        from: 1.0
        to: 0.0
        duration: 1000
        onStopped: {
            showWelcome = false
            welcomeScreen.visible = false
        }
    }

    // SwipeView for dashboards
    SwipeView {
        id: swipeView
        anchors.fill: parent
        visible: !showWelcome
        z: 1
        currentIndex: root.currentIndex
        interactive: true // Enable swipe gestures

        onCurrentIndexChanged: {
            root.currentIndex = currentIndex
        }

        // Dashboard 1
        Loader {
            source: dashboardList[0]
            asynchronous: false

            onStatusChanged: {
                if (status === Loader.Error) {
                    console.log("Error loading Dashboard 1")
                } else if (status === Loader.Ready) {
                    console.log("Dashboard 1 loaded successfully")
                }
            }
        }

        // Dashboard 2
        Loader {
            source: dashboardList[1]
            asynchronous: false

            onStatusChanged: {
                if (status === Loader.Error) {
                    console.log("Error loading Dashboard 2")
                } else if (status === Loader.Ready) {
                    console.log("Dashboard 2 loaded successfully")
                }
            }
        }

        // Dashboard 3
        Loader {
            source: dashboardList[2]
            asynchronous: false

            onStatusChanged: {
                if (status === Loader.Error) {
                    console.log("Error loading Dashboard 3")
                } else if (status === Loader.Ready) {
                    console.log("Dashboard 3 loaded successfully")
                }
            }
        }
    }



    // Debug button
    Button {
        id: backToWelcomeButton
        text: "⟲"
        visible: !showWelcome && Qt.application.arguments.indexOf("--debug") !== -1
        z: 999
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 20
        width: 50
        height: 50

        onClicked: {
            showWelcome = true
            welcomeScreen.visible = true
            welcomeScreen.opacity = 1.0
        }

        background: Rectangle {
            color: "#80FF6B35"
            radius: 25
            border.color: "#FFFFFF"
            border.width: 2
        }

        contentItem: Text {
            text: parent.text
            color: "white"
            font.pixelSize: 24
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    // Keyboard shortcuts
    Shortcut {
        sequence: "Ctrl+Q"
        context: Qt.ApplicationShortcut
        onActivated: Qt.quit()
    }

    Shortcut {
        sequence: "Ctrl+W"
        context: Qt.ApplicationShortcut
        onActivated: {
            showWelcome = true
            welcomeScreen.visible = true
            welcomeScreen.opacity = 1.0
        }
    }

    // Direct access with Ctrl+1/2/3 (GIỮ LẠI - không liên quan đến phím mũi tên)
    Shortcut {
        sequence: "Ctrl+1"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (!showWelcome) swipeView.currentIndex = 0
        }
    }

    Shortcut {
        sequence: "Ctrl+2"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (!showWelcome) swipeView.currentIndex = 1
        }
    }

    Shortcut {
        sequence: "Ctrl+3"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (!showWelcome) swipeView.currentIndex = 2
        }
    }
}
