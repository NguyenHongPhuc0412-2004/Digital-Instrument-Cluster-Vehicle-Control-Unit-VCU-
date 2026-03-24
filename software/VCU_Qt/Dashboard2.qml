import QtQuick 2.12
import CustomControls 1.0
import QtQuick.Window 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtGraphicalEffects 1.0
import "./"
Item {
    width: 1920
    height: 960
    property int nextSpeed: 60
    property int distanceValue: 188
    property int fuelValue: 34
    property int avgSpeedValue: 78

    // THÊM CÁC BEHAVIOR NÀY
    Behavior on distanceValue {
        NumberAnimation { duration: 1000; easing.type: Easing.OutQuad }
    }
    Behavior on fuelValue {
        NumberAnimation { duration: 1000; easing.type: Easing.OutQuad }
    }
    Behavior on avgSpeedValue {
        NumberAnimation { duration: 1000; easing.type: Easing.OutQuad }
    }

    function generateRandom(maxLimit = 70){
        let rand = Math.random() * maxLimit;
        rand = Math.floor(rand);
        return rand;
    }

    function speedColor(value){
        if(value < 60 ){
            return "green"
        } else if(value > 60 && value < 150){
            return "yellow"
        }else{
            return "Red"
        }
    }

    Timer {
        interval: 500
        running: true
        repeat: true
        onTriggered:{

            currentTime.text = Qt.formatDateTime(new Date(), "hh:mm")
        }
    }

    Timer{
        repeat: true
        interval: 3000
        running: true
        onTriggered: {
            nextSpeed = generateRandom()
        }
    }
    // Thêm sau Timer{} hiện có (khoảng dòng 40-45)
    Timer {
        interval: 5000  // Cập nhật mỗi 5 giây
        running: true
        repeat: true
        onTriggered: {
            // Random distance 150-250 km
            distanceValue = Math.floor(Math.random() * 100) + 150
            // Random fuel usage 28-40 mpg
            fuelValue = Math.floor(Math.random() * 12) + 28
            // Random avg speed 60-90 mph
            avgSpeedValue = Math.floor(Math.random() * 30) + 60
        }
    }


//    Shortcut {
//        sequence: "Ctrl+Q"
//        context: Qt.ApplicationShortcut
//        onActivated: Qt.quit()
//    }


    Image {
        id: dashboard
        width: parent.width
        height: parent.height
        anchors.centerIn: parent
        source: "qrc:/assets/Dashboard5.svg"

        /*
          Top Bar Of Screen
        */

        Image {
            id: topBar
            width: 1357
            source: "qrc:/assets/Vector_ex.svg"

            anchors{
                top: parent.top
                topMargin: 20
                horizontalCenter: parent.horizontalCenter
            }

            Image {
                id: headLight
                property bool indicator: false
                width: 42.5
                height: 38.25
                anchors{
                    top: parent.top
                    topMargin: 25
                    leftMargin: 230
                    left: parent.left
                }
                source: indicator ? "qrc:/assets/Low beam headlights.svg" : "qrc:/assets/Low_beam_headlights_white.svg"
                Behavior on indicator { NumberAnimation { duration: 300 }}
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        headLight.indicator = !headLight.indicator
                    }
                }
            }

            Label{
                id: currentTime
                text: Qt.formatDateTime(new Date(), "hh:mm")
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.DemiBold
                color: "#FFFFFF"
                anchors.top: parent.top
                anchors.topMargin: 25
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Label{
                id: currentDate
                text: Qt.formatDateTime(new Date(), "dd/MM/yyyy")
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.DemiBold
                color: "#FFFFFF"
                anchors.right: parent.right
                anchors.rightMargin: 230
                anchors.top: parent.top
                anchors.topMargin: 25
            }
        }



        /*
          Speed Label
        */

        //        Label{
        //            id:speedLabel
        //            text: "68"
        //            font.pixelSize: 134
        //            font.family: "Inter"
        //            color: "#01E6DE"
        //            font.bold: Font.DemiBold
        //            anchors.top: parent.top
        //            anchors.topMargin:Math.floor(parent.height * 0.35)
        //            anchors.horizontalCenter: parent.horizontalCenter
        //        }
        MapGauge {
            id: speedLabel
            width: 450
            height: 450
            property bool accelerating
            value: accelerating ? maximumValue : 0
            maximumValue: 250

            anchors.top: parent.top
            anchors.topMargin:Math.floor(parent.height * 0.25)
            anchors.horizontalCenter: parent.horizontalCenter

            Component.onCompleted: forceActiveFocus()

            Behavior on value { NumberAnimation { duration: 1000 }}

            Keys.onSpacePressed: accelerating = true
            Keys.onReleased: {
                if (event.key === Qt.Key_Space) {
                    accelerating = false;
                    event.accepted = true;
                }else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    speedLabel.accelerating = false; // <-- Sửa thành speedLabel
                    event.accepted = true
                }
            }

            Keys.onEnterPressed: speedLabel.accelerating = true // <-- Sửa thành speedLabel
            Keys.onReturnPressed: speedLabel.accelerating = true // <-- Sửa thành speedLabel
        }

        //        Label{
        //            text: "MPH"
        //            font.pixelSize: 46
        //            font.family: "Inter"
        //            color: "#01E6DE"
        //            font.bold: Font.Normal
        //            anchors.top:speedLabel.bottom
        //            anchors.horizontalCenter: parent.horizontalCenter
        //        }


        /*
          Speed Limit Label
        */

        Rectangle{
            id:speedLimit
            width: 130
            height: 130
            radius: height/2
            color: "#D9D9D9"
            border.color: speedColor(maxSpeedlabel.text)
            border.width: 10

            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 50

            Label{
                id:maxSpeedlabel
                text: getRandomInt(150, speedLabel.maximumValue).toFixed(0)
                font.pixelSize: 45
                font.family: "Inter"
                font.bold: Font.Bold
                color: "#01E6DE"
                anchors.centerIn: parent

                function getRandomInt(min, max) {
                    return Math.floor(Math.random() * (max - min + 1)) + min;
                }
            }
        }

        Image {
            anchors{
                bottom: car.top
                bottomMargin: 30
                horizontalCenter:car.horizontalCenter
            }
            source: "qrc:/assets/Model 3.png"
        }


        Image {
            id:car
            anchors{
                bottom: speedLimit.top
                bottomMargin: 30
                horizontalCenter:speedLimit.horizontalCenter
            }
            source: "qrc:/assets/Car.svg"

            // Thêm rotation nhẹ để xe trông như đang lắc trên đường
            RotationAnimation on rotation {
                running: speedLabel.value > 20
                loops: Animation.Infinite
                from: -1
                to: 1
                duration: 800
                easing.type: Easing.InOutQuad
            }
        }

        // IMGonline.com.ua  ==> Compress Image With


        /*
          Left Road
        */

        Image {
            id: leftRoad
            width: 127
            height: 397
            anchors{
                left: speedLimit.left
                leftMargin: 100
                bottom: parent.bottom
                bottomMargin: 26.50
            }

            source: "qrc:/assets/Vector 2.svg"
        }

        RowLayout{
            spacing: 20

            anchors{
                left: parent.left
                leftMargin: 250
                bottom: parent.bottom
                bottomMargin: 26.50 + 65
            }

            RowLayout{
                spacing: 3
                    Label{
                        id: tempFromDHT11  // ← THÊM id này
                        text: "25.0°C"
                        font.pixelSize: 32
                        font.family: "Inter"
                        font.bold: Font.Normal
                        font.capitalization: Font.AllUppercase
                        color: "#FFFFFF"
                    }

            Label{
                        id: humidityFromDHT11  // ← THÊM humidity
                        text: "50%"
                        font.pixelSize: 28
                        font.family: "Inter"
                        font.bold: Font.Normal
                        opacity: 0.7
                        color: "#01E6DE"
                    }
            }

            RowLayout{
                spacing: 1
                Layout.topMargin: 10
                Rectangle{
                    width: 20
                    height: 15
                    color: speedLabel.value.toFixed(0) > 31.25 ? speedLabel.speedColor : "#01E6DC"
                }
                Rectangle{
                    width: 20
                    height: 15
                    color: speedLabel.value.toFixed(0) > 62.5 ? speedLabel.speedColor : "#01E6DC"
                }
                Rectangle{
                    width: 20
                    height: 15
                    color: speedLabel.value.toFixed(0) > 93.75 ? speedLabel.speedColor : "#01E6DC"
                }
                Rectangle{
                    width: 20
                    height: 15
                    color: speedLabel.value.toFixed(0) > 125.25 ? speedLabel.speedColor : "#01E6DC"
                }
                Rectangle{
                    width: 20
                    height: 15
                    color: speedLabel.value.toFixed(0) > 156.5 ? speedLabel.speedColor : "#01E6DC"
                }
                Rectangle{
                    width: 20
                    height: 15
                    color: speedLabel.value.toFixed(0) > 187.75 ? speedLabel.speedColor : "#01E6DC"
                }
                Rectangle{
                    width: 20
                    height: 15
                    color: speedLabel.value.toFixed(0) > 219 ? speedLabel.speedColor : "#01E6DC"
                }
            }

            Label{
                text: speedLabel.value.toFixed(0) + " MPH "
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.Normal
                font.capitalization: Font.AllUppercase
                color: "#FFFFFF"
            }
        }

        /*
          Right Road
        */

        Image {
            id: rightRoad
            width: 127
            height: 397
            anchors{
                right: speedLimit.right
                rightMargin: 100
                bottom: parent.bottom
                bottomMargin: 26.50
            }

            source: "qrc:/assets/Vector 1.svg"
        }

        /*
          Right Side gear
        */

        RowLayout{
            spacing: 20
            anchors{
                right: parent.right
                rightMargin: 350
                bottom: parent.bottom
                bottomMargin: 26.50 + 65
            }

            Label{
                text: "Ready"
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.Normal
                font.capitalization: Font.AllUppercase
                color: "#32D74B"
            }

            Label{
                text: "P"
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.Normal
                font.capitalization: Font.AllUppercase
                color: "#FFFFFF"
            }

            Label{
                text: "R"
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.Normal
                font.capitalization: Font.AllUppercase
                opacity: 0.2
                color: "#FFFFFF"
            }
            Label{
                text: "N"
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.Normal
                font.capitalization: Font.AllUppercase
                opacity: 0.2
                color: "#FFFFFF"
            }
            Label{
                text: "D"
                font.pixelSize: 32
                font.family: "Inter"
                font.bold: Font.Normal
                font.capitalization: Font.AllUppercase
                opacity: 0.2
                color: "#FFFFFF"
            }
        }

        /*Left Side Icons*/
        Image {
            id:forthLeftIndicator
            property bool parkingLightOn: true
            width: 72
            height: 62
            anchors{
                left: parent.left
                leftMargin: 175
                bottom: thirdLeftIndicator.top
                bottomMargin: 25
            }
            Behavior on parkingLightOn { NumberAnimation { duration: 300 }}
            source: parkingLightOn ? "qrc:/assets/Parking lights.svg" : "qrc:/assets/Parking_lights_white.svg"
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    forthLeftIndicator.parkingLightOn = !forthLeftIndicator.parkingLightOn
                }
            }
        }

        Image {
            id:thirdLeftIndicator
            property bool lightOn: true
            width: 52
            height: 70.2
            anchors{
                left: parent.left
                leftMargin: 145
                bottom: secondLeftIndicator.top
                bottomMargin: 25
            }
            source: lightOn ? "qrc:/assets/Lights.svg" : "qrc:/assets/Light_White.svg"
            Behavior on lightOn { NumberAnimation { duration: 300 }}
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    thirdLeftIndicator.lightOn = !thirdLeftIndicator.lightOn
                }
            }
        }

        Image {
            id:secondLeftIndicator
            property bool headLightOn: true
            width: 51
            height: 51
            anchors{
                left: parent.left
                leftMargin: 125
                bottom: firstLeftIndicator.top
                bottomMargin: 30
            }
            Behavior on headLightOn { NumberAnimation { duration: 300 }}
            source:headLightOn ?  "qrc:/assets/Low beam headlights.svg" : "qrc:/assets/Low_beam_headlights_white.svg"

            MouseArea{
                anchors.fill: parent
                onClicked: {
                    secondLeftIndicator.headLightOn = !secondLeftIndicator.headLightOn
                }
            }
        }

        Image {
            id:firstLeftIndicator
            property bool rareLightOn: false
            width: 51
            height: 51
            anchors{
                left: parent.left
                leftMargin: 100
                verticalCenter: speedLabel.verticalCenter
            }
            source: rareLightOn ? "qrc:/assets/Rare_fog_lights_red.svg" : "qrc:/assets/Rare fog lights.svg"
            Behavior on rareLightOn { NumberAnimation { duration: 300 }}
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    firstLeftIndicator.rareLightOn = !firstLeftIndicator.rareLightOn
                }
            }
        }

        /*Right Side Icons*/

        Image {
            id:forthRightIndicator
            property bool indicator: true
            width: 56.83
            height: 36.17
            anchors{
                right: parent.right
                rightMargin: 195
                bottom: thirdRightIndicator.top
                bottomMargin: 50
            }
            source: indicator ? "qrc:/assets/FourthRightIcon.svg" : "qrc:/assets/FourthRightIcon_red.svg"
            Behavior on indicator { NumberAnimation { duration: 300 }}
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    forthRightIndicator.indicator = !forthRightIndicator.indicator
                }
            }
        }

        Image {
            id:thirdRightIndicator
            property bool indicator: true
            width: 56.83
            height: 36.17
            anchors{
                right: parent.right
                rightMargin: 155
                bottom: secondRightIndicator.top
                bottomMargin: 50
            }
            source: indicator ? "qrc:/assets/thirdRightIcon.svg" : "qrc:/assets/thirdRightIcon_red.svg"
            Behavior on indicator { NumberAnimation { duration: 300 }}
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    thirdRightIndicator.indicator = !thirdRightIndicator.indicator
                }
            }
        }

        Image {
            id:secondRightIndicator
            property bool indicator: true
            width: 56.83
            height: 36.17
            anchors{
                right: parent.right
                rightMargin: 125
                bottom: firstRightIndicator.top
                bottomMargin: 50
            }
            source: indicator ? "qrc:/assets/SecondRightIcon.svg" : "qrc:/assets/SecondRightIcon_red.svg"
            Behavior on indicator { NumberAnimation { duration: 300 }}
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    secondRightIndicator.indicator = !secondRightIndicator.indicator
                }
            }
        }

        Image {
            id:firstRightIndicator
            property bool sheetBelt: true
            width: 36
            height: 45
            anchors{
                right: parent.right
                rightMargin: 100
                verticalCenter: speedLabel.verticalCenter
            }
            source: sheetBelt ? "qrc:/assets/FirstRightIcon.svg" : "qrc:/assets/FirstRightIcon_grey.svg"
            Behavior on sheetBelt { NumberAnimation { duration: 300 }}
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    firstRightIndicator.sheetBelt = !firstRightIndicator.sheetBelt
                }
            }
        }

        // 3D Battery - Thay thế RadialBar bằng battery 3D đẹp
        Item {
            id: battery3D
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: parent.width / 6
            }
            width: 338
            height: 338

            property int batteryLevel: speedLabel.accelerating ? 100 : 65
            Behavior on batteryLevel { NumberAnimation { duration: 1000 }}

            // Xóa viền nền xung quanh - không cần glow effect ngoài nữa

            // Container cho battery
            Item {
                anchors.centerIn: parent
                width: 220
                height: 320

                // Battery background image
                Image {
                    id: batteryBg
                    anchors.centerIn: parent
                    width: 220
                    height: 320
                    source: "qrc:/assets/battery3.png"
                    fillMode: Image.PreserveAspectFit
                    opacity: 0.4

                    // Rotating animation nhẹ
                    NumberAnimation on rotation {
                        from: -1.5
                        to: 1.5
                        duration: 4000
                        loops: Animation.Infinite
                        easing.type: Easing.InOutSine
                    }
                }

                // Clip container cho fill effect - TĂNG KÍCH THƯỚC để mực nước cao hơn
                Item {
                    anchors.centerIn: parent
                    width: 160  // Giảm width để vừa với battery3.png
                    height: 240 // Giảm height để vừa với battery3.png
                    anchors.verticalCenterOffset: 5 // Đẩy xuống một chút
                    clip: true

                    // Fill rectangle với gradient
                    Rectangle {
                        id: fillLevel
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: parent.height * (battery3D.batteryLevel / 100)
                        radius: 25

                        gradient: Gradient {
                            GradientStop {
                                position: 0.0
                                color: battery3D.batteryLevel > 80 ? "#00FF41" :
                                       battery3D.batteryLevel > 50 ? "#01E4E0" :
                                       battery3D.batteryLevel > 20 ? "#FFA500" : "#FF4757"
                            }
                            GradientStop {
                                position: 0.5
                                color: battery3D.batteryLevel > 80 ? "#01E4E0" :
                                       battery3D.batteryLevel > 50 ? "#0099FF" :
                                       battery3D.batteryLevel > 20 ? "#FF8C00" : "#FF3333"
                            }
                            GradientStop {
                                position: 1.0
                                color: battery3D.batteryLevel > 80 ? "#6C5CE7" :
                                       battery3D.batteryLevel > 50 ? "#6C5CE7" :
                                       battery3D.batteryLevel > 20 ? "#FF6B35" : "#FF0000"
                            }
                        }

                        Behavior on height {
                            NumberAnimation {
                                duration: 1000
                                easing.type: Easing.OutElastic
                            }
                        }

                        // Shimmer effect
                        Rectangle {
                            anchors.fill: parent
                            radius: parent.radius
                            opacity: 0.4
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: "#00FFFFFF" }
                                GradientStop { position: 0.5; color: "#CCFFFFFF" }
                                GradientStop { position: 1.0; color: "#00FFFFFF" }
                            }

                            SequentialAnimation on x {
                                loops: Animation.Infinite
                                NumberAnimation { from: -200; to: 200; duration: 3000 }
                                PauseAnimation { duration: 500 }
                            }
                        }

                        // Bubbles effect
                        Repeater {
                            model: 8
                            Rectangle {
                                width: 6 + (index % 3) * 3
                                height: width
                                radius: width / 2
                                color: "#40FFFFFF"
                                x: 30 + (index * 20)
                                y: fillLevel.height

                                SequentialAnimation on y {
                                    loops: Animation.Infinite
                                    NumberAnimation {
                                        from: fillLevel.height + 10
                                        to: -20
                                        duration: 3000 + (index * 400)
                                    }
                                    PauseAnimation { duration: index * 200 }
                                }

                                SequentialAnimation on opacity {
                                    loops: Animation.Infinite
                                    NumberAnimation { from: 0.3; to: 0.9; duration: 1500 }
                                    NumberAnimation { from: 0.9; to: 0.3; duration: 1500 }
                                }
                            }
                        }
                    }
                }

                // Battery outline image (trên cùng)
                Image {
                    id: batteryOutline
                    anchors.centerIn: parent
                    width: 220
                    height: 320
                    source: "qrc:/assets/battery3.png"
                    fillMode: Image.PreserveAspectFit
                }
            }

            // Lightning bolt - GIỮ NGUYÊN VỊ TRÍ
            Item {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -20  // Đẩy lên cao để nằm trong battery
                width: 80   // Giảm size
                height: 100

                Image {
                    id: lightningBolt
                    anchors.centerIn: parent
                    width: 60   // Giảm size
                    height: 90
                    source: "qrc:/assets/lightning.png"
                    fillMode: Image.PreserveAspectFit
                    opacity: battery3D.batteryLevel > 90 ? 1.0 : 0.7

                    // Pulsing scale
                    SequentialAnimation on scale {
                        loops: Animation.Infinite
                        NumberAnimation { from: 0.9; to: 1.1; duration: 1200; easing.type: Easing.InOutSine }
                        NumberAnimation { from: 1.1; to: 0.9; duration: 1200; easing.type: Easing.InOutSine }
                    }
                }

                // Lightning glow effect đơn giản - GIẢM SIZE
                Rectangle {
                    anchors.centerIn: parent
                    width: 100
                    height: 130
                    radius: 50
                    color: "#30FFFF00"
                    z: -1

                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        NumberAnimation { from: 0.2; to: 0.6; duration: 800 }
                        NumberAnimation { from: 0.6; to: 0.2; duration: 800 }
                    }
                }
            }

            // Text overlay NHỎ GỌN TRONG BATTERY
            ColumnLayout {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 30  // Đẩy lên cao hơn để nằm trong battery
                spacing: 2

                Label {
                    id: percentText
                    text: battery3D.batteryLevel.toFixed(0) + "%"
                    font.pixelSize: 45  // Giảm size cho vừa vặn
                    font.family: "Inter"
                    font.bold: Font.Bold
                    color: "#FFFFFF"
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: "Battery charge"
                    font.pixelSize: 16  // Giảm size
                    font.family: "Inter"
                    font.bold: Font.Normal
                    opacity: 0.9
                    color: "#FFFFFF"
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            // Floating particles - XÓA BỚT để gọn gàng hơn
            Repeater {
                model: 6  // Giảm từ 12 xuống 6
                Rectangle {
                    width: 3  // Giảm size
                    height: width
                    radius: width / 2

                    color: index % 3 == 0 ? "#4000FF41" :
                           index % 3 == 1 ? "#4001E4E0" : "#406C5CE7"

                    x: 80 + (index * 30)
                    y: battery3D.height + 20

                    SequentialAnimation on y {
                        loops: Animation.Infinite
                        NumberAnimation {
                            from: battery3D.height + 20
                            to: -20
                            duration: 3500 + (index * 400)
                            easing.type: Easing.InOutSine
                        }
                        PauseAnimation { duration: index * 200 }
                    }

                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        NumberAnimation { from: 0.1; to: 0.5; duration: 1800 + (index * 200) }
                        NumberAnimation { from: 0.5; to: 0.1; duration: 1800 + (index * 200) }
                    }
                }
            }
        }

        ColumnLayout{
            spacing: 40

            anchors{
                verticalCenter: parent.verticalCenter
                right: parent.right
                rightMargin: parent.width / 6
            }

            RowLayout{
                spacing: 30
                Image {
                    width: 72
                    height: 50
                    source: "qrc:/assets/road.svg"
                }

                ColumnLayout{
                    Label{
                        text: distanceValue + " KM"
                        font.pixelSize: 30
                        font.family: "Inter"
                        font.bold: Font.Normal
                        opacity: 0.8
                        color: "#FFFFFF"
                    }
                    Label{
                        text: "Distance"
                        font.pixelSize: 20
                        font.family: "Inter"
                        font.bold: Font.Normal
                        opacity: 0.8
                        color: "#FFFFFF"
                    }
                }
            }
            RowLayout{
                spacing: 30
                Image {
                    width: 72
                    height: 78
                    source: "qrc:/assets/fuel.svg"
                }

                ColumnLayout{
                    Label{
                        text: fuelValue + " mpg"
                        font.pixelSize: 30
                        font.family: "Inter"
                        font.bold: Font.Normal
                        opacity: 0.8
                        color: "#FFFFFF"
                    }
                    Label{
                        text: "Avg. Fuel Usage"
                        font.pixelSize: 20
                        font.family: "Inter"
                        font.bold: Font.Normal
                        opacity: 0.8
                        color: "#FFFFFF"
                    }
                }
            }
            RowLayout{
                spacing: 30
                Image {
                    width: 72
                    height: 72
                    source: "qrc:/assets/speedometer.svg"
                }

                ColumnLayout{
                    Label{
                        text: avgSpeedValue + " mph"
                        font.pixelSize: 30
                        font.family: "Inter"
                        font.bold: Font.Normal
                        opacity: 0.8
                        color: "#FFFFFF"
                    }
                    Label{
                        text: "Avg. Speed"
                        font.pixelSize: 20
                        font.family: "Inter"
                        font.bold: Font.Normal
                        opacity: 0.8
                        color: "#FFFFFF"
                    }
                }
            }
        }
    }
    // Thêm vào cuối Dashboard2.qml

        // CAN Handler Connections
        Connections {
            target: canHandler

            // Turn signals - Dashboard 2 không có turn signals visible
            // nhưng vẫn log để debug
            onLeftLightChanged: {
                console.log("Left turn signal:", leftLight)
            }

            onRightLightChanged: {
                console.log("Right turn signal:", rightLight)
            }

            onHazardLightsChanged: {
                console.log("Hazard lights:", hazardLights)
            }

            // Headlights
            onHighBeamChanged: {
                console.log("High beam:", highBeam)
            }

            onLowBeamChanged: {
                headLight.indicator = lowBeam
            }

            onParkingLightsChanged: {
                forthLeftIndicator.parkingLightOn = parkingLights
            }

            // Speed from CAN
            onSpeedChanged: {
                // Convert from raw CAN value to speed (adjust formula based on your sensor)
                // Assuming speed is in range 0-4000 from CAN, convert to 0-250 MPH
                var calculatedSpeed = speed * speedLabel.maximumValue / 4000
                speedLabel.value = Math.max(0, Math.min(speedLabel.maximumValue, calculatedSpeed))
            }

            // Battery from CAN
            onBatteryChanged: {
                // Update 3D battery display
                battery3D.batteryLevel = battery
                console.log("Battery level from CAN:", battery)
            }

            // Temperature from CAN
            onTemperatureChanged: {
                    // Convert từ raw ADC value sang Celsius
                    // Công thức: temp_celsius = (temperature / 4095.0) * 50.0
                    var tempCelsius = (temperature / 4095.0) * 50.0
                    tempFromDHT11.text = tempCelsius.toFixed(1) + "°C"
                }

                // Humidity from DHT11
            onHumidityChanged: {
                    // Convert từ raw ADC value sang phần trăm
                    // Công thức: humidity_percent = (humidity / 4095.0) * 100.0
                    var humidityPercent = (humidity / 4095.0) * 100.0
                    humidityFromDHT11.text = humidityPercent.toFixed(0) + "%"
                }
            onCarLightsToggled: {
                    car.carLightsOn = state
                    console.log("🚗 S11 pressed: Car lights =", state)
                }
        }
}
