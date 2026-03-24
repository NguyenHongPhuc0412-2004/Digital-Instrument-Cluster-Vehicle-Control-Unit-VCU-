import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12

Item {
    width: parent.width
    height: parent.height

    // Background image
    Image {
        id: rootBg
        anchors.fill: parent
        source: "qrc:/assets/car_bg"  // Đảm bảo đường dẫn đúng với project của bạn
        opacity: 0.1
        fillMode: Image.PreserveAspectCrop
    }

    // Welcome text
    Text {
        id: welcomeText
        anchors.centerIn: parent
        text: qsTr("Welcome..")
        font.pixelSize: 48
        font.family: "Inter"
        font.bold: Font.DemiBold
        color: "#FFFFFF"
    }

    // Current time and date display
    Column {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 100
        spacing: 10

        Text {
            id: currentTime
            anchors.horizontalCenter: parent.horizontalCenter
            text: Qt.formatDateTime(new Date(), "hh:mm:ss")
            font.pixelSize: 32
            font.family: "Inter"
            font.bold: Font.DemiBold
            color: "#FFFFFF"
        }

        Text {
            id: currentDate
            anchors.horizontalCenter: parent.horizontalCenter
            text: Qt.formatDateTime(new Date(), "dddd, MMMM dd, yyyy")
            font.pixelSize: 18
            font.family: "Inter"
            color: "#FFFFFF"
            opacity: 0.8
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "23° Windy"
            font.pixelSize: 16
            font.family: "Inter"
            color: "#FFFFFF"
            opacity: 0.6
        }
    }

    // Timer to update time
    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            currentTime.text = Qt.formatDateTime(new Date(), "hh:mm:ss")
            currentDate.text = Qt.formatDateTime(new Date(), "dddd, MMMM dd, yyyy")
        }
    }

    // Background fade-in animation
    OpacityAnimator {
        target: rootBg
        from: 0.1
        to: 1.0
        duration: 2000
        running: true
        onStopped: {
            // Delay 3 seconds then signal to switch to dashboard
            transitionTimer.start()
        }
    }

    // Text fade-in animation (slightly delayed)
    OpacityAnimator {
        target: welcomeText
        from: 0
        to: 1
        duration: 1500
        running: true
    }

    // Timer for transition to dashboard
    Timer {
        id: transitionTimer
        interval: 3000
        running: false
        repeat: false
        onTriggered: {
            // Send signal to parent to switch to dashboard
            parent.switchToDashboard()
        }
    }
}
