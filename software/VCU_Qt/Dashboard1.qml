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

    // Road glow khi bật đèn xe (R)
    property real roadGlowRadius: 0
    Behavior on roadGlowRadius {
        NumberAnimation { duration: 1500; easing.type: Easing.InOutQuad }
    }
    property real roadGlowSpread: 0.0
    Behavior on roadGlowSpread {
        NumberAnimation { duration: 1500; easing.type: Easing.InOutQuad }
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

    // Timer cho hiệu ứng nhấp nháy xi nhan trái
    Timer {
        id: leftTurnSignalTimer
        interval: 500
        repeat: true
        running: false
        onTriggered: {
            leftTurnSignal.opacity = leftTurnSignal.opacity === 1.0 ? 0.2 : 1.0
            leftTurnIndicator.opacity = leftTurnIndicator.opacity === 1.0 ? 0.2 : 1.0
            if (leftTurnSignal.opacity === 1.0) {
                        canHandler.sendOutputCommand(8, 1, 100);  // ON
                    } else {
                        canHandler.sendOutputCommand(8, 1, 0);    // OFF (duty=0)
                    }
        }
    }

    // Timer cho hiệu ứng nhấp nháy xi nhan phải
    Timer {
        id: rightTurnSignalTimer
        interval: 500
        repeat: true
        running: false
        onTriggered: {
            rightTurnSignal.opacity = rightTurnSignal.opacity === 1.0 ? 0.2 : 1.0
            rightTurnIndicator.opacity = rightTurnIndicator.opacity === 1.0 ? 0.2 : 1.0
            if (rightTurnSignal.opacity === 1.0) {
                        canHandler.sendOutputCommand(12, 1, 100); // ON
                    } else {
                        canHandler.sendOutputCommand(12, 1, 0);   // OFF
                    }
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

            // Xi nhan trái trên thanh top - hiển thị mờ
            Image {
                id: leftTurnSignal
                width: 42.5
                height: 38.25
                visible: true
                opacity: 0.2  // Hiển thị mờ
                anchors{
                    top: parent.top
                    topMargin: 25
                    leftMargin: 50
                    left: parent.left
                }
                source: "qrc:/assets/icon-park-solid_left-two.svg"
                Behavior on opacity { NumberAnimation { duration: 200 } }
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

            // Xi nhan phải trên thanh top - hiển thị mờ
            Image {
                id: rightTurnSignal
                width: 42.5
                height: 38.25
                visible: true
                opacity: 0.2  // Hiển thị mờ
                anchors{
                    top: parent.top
                    topMargin: 25
                    rightMargin: 50
                    right: parent.right
                }
                source: "qrc:/assets/icon-park-solid_right-two.svg"
                Behavior on opacity { NumberAnimation { duration: 200 } }
            }
        }

        /*
          Speed Label
        */

        Gauge {
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
            Keys.onEnterPressed: radialBar.accelerating = true
            Keys.onReturnPressed: radialBar.accelerating = true
            Keys.onLeftPressed: leftGauge.accelerating  = true
            Keys.onRightPressed: rightGauge.accelerating = true

            Keys.onReleased: {
                if (event.key === Qt.Key_Space) {
                    accelerating = false;
                    event.accepted = true;
                }else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
                    radialBar.accelerating = false;
                    event.accepted = true;
                }else if(event.key === Qt.Key_Left){
                    leftGauge.accelerating = false;
                    event.accepted = true;
                }else if(event.key === Qt.Key_Right){
                    rightGauge.accelerating = false;
                    event.accepted = true;
                }
            }

            // Controls Lights and Turn Signals
            Keys.onPressed: {
                if(event.key === Qt.Key_M){
                   forthRightIndicator.indicator = !forthRightIndicator.indicator
                   event.accepted = true;
                }else if(event.key === Qt.Key_L){
                    firstLeftIndicator.rareLightOn = !firstLeftIndicator.rareLightOn
                    event.accepted = true;
                }else if(event.key === Qt.Key_N){
                    secondLeftIndicator.headLightOn = !secondLeftIndicator.headLightOn
                    event.accepted = true;
                }else if(event.key === Qt.Key_B){
                    thirdLeftIndicator.lightOn = !thirdLeftIndicator.lightOn
                    event.accepted = true;
                }else if(event.key === Qt.Key_C){
                    forthLeftIndicator.parkingLightOn = !forthLeftIndicator.parkingLightOn
                    event.accepted = true;
                }else if(event.key === Qt.Key_V){
                    firstRightIndicator.sheetBelt = !firstRightIndicator.sheetBelt
                    event.accepted = true;
                }else if(event.key === Qt.Key_Z){
                    event.accepted = true;
                    secondRightIndicator.indicator = !secondRightIndicator.indicator
                }else if(event.key === Qt.Key_X){
                    event.accepted = true;
                    thirdRightIndicator.indicator = !thirdRightIndicator.indicator
                }else if(event.key === Qt.Key_Q){
                    // Bật/tắt xi nhan trái
                    if(leftTurnSignalTimer.running){
                        leftTurnSignalTimer.stop();
                        leftTurnSignal.opacity = 0.2;
                        leftTurnIndicator.opacity = 0.2;
                    } else {
                        rightTurnSignalTimer.stop();
                        rightTurnSignal.opacity = 0.2;
                        rightTurnIndicator.opacity = 0.2;
                        leftTurnSignalTimer.start();
                    }
                    event.accepted = true;
                }else if(event.key === Qt.Key_W){
                    // Bật/tắt xi nhan phải
                    if(rightTurnSignalTimer.running){
                        rightTurnSignalTimer.stop();
                        rightTurnSignal.opacity = 0.2;
                        rightTurnIndicator.opacity = 0.2;
                    } else {
                        leftTurnSignalTimer.stop();
                        leftTurnSignal.opacity = 0.2;
                        leftTurnIndicator.opacity = 0.2;
                        rightTurnSignalTimer.start();
                    }
                    event.accepted = true;
                }else if(event.key === Qt.Key_E){
                    // Tắt cả hai xi nhan
                    leftTurnSignalTimer.stop();
                    rightTurnSignalTimer.stop();
                    leftTurnSignal.opacity = 0.2;
                    rightTurnSignal.opacity = 0.2;
                    leftTurnIndicator.opacity = 0.2;
                    rightTurnIndicator.opacity = 0.2;
                    event.accepted = true;
                }else if(event.key === Qt.Key_R){
                    // Chuyển đổi giữa car.svg và newcar.svg + bật/tắt road glow
                    car.carLightsOn = !car.carLightsOn;
                    roadGlowRadius = car.carLightsOn ? 21 : 0;
                    roadGlowSpread = car.carLightsOn ? 0.35 : 0.0;
                    event.accepted = true;
                }
            }
        }

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
                    id: carModel3Text
                    z: 15
                    // Anchor vào speedLimit (cố định) thay vì car.top
                    // → KHÔNG bị ảnh hưởng dù car.svg hay newcar.svg có chiều cao khác nhau
                    anchors{
                        bottom: speedLimit.top
                        bottomMargin: 133  // Cố định: thân xe luôn ở bottom, phần đèn pha mở rộng lên trên
                        horizontalCenter: speedLimit.horizontalCenter
                    }
                    source: "qrc:/assets/Model 3.png"

                    property real floatY: 0
                    transform: Translate { y: carModel3Text.floatY }

                    // ===== MODEL 3 NỔI LÊN VÀ BIẾN MẤT =====
                    SequentialAnimation {
                        running: speedLabel.value > 10
                        loops: Animation.Infinite

                        NumberAnimation {
                            target: carModel3Text
                            property: "floatY"
                            from: 0; to: -28
                            duration: 1800
                            easing.type: Easing.InOutSine
                        }

                        NumberAnimation {
                            target: carModel3Text
                            property: "opacity"
                            from: 1.0; to: 0.0
                            duration: 350
                        }

                        PauseAnimation { duration: 250 }

                        PropertyAction {
                            target: carModel3Text
                            property: "floatY"
                            value: 0
                        }

                        NumberAnimation {
                            target: carModel3Text
                            property: "opacity"
                            from: 0.0; to: 1.0
                            duration: 350
                        }

                        PauseAnimation { duration: 700 }
                    }
                }

        Image {
            id: car
            z: 15
            property bool carLightsOn: false
            property real bounceY: 0
            property real bounceRot: 0
            // Kích thước tính đúng: scale newcar.svg sao cho phần thân xe = car.svg
            // Car.svg (120x114) render 116x110 → thân xe = 116x110
            // newcar.svg (253x183) thân xe ~120x114, đèn pha mở rộng canvas
            // → Scale: 116*(253/120)=245, 110*(183/114)=177 → thân xe khớp 116x110
            width: carLightsOn ? 245 : 112
            height: carLightsOn ? 177 : 107
            fillMode: Image.PreserveAspectFit
            anchors{
                bottom: speedLimit.top
                bottomMargin: carLightsOn ? 5 : 15
                horizontalCenter: speedLimit.horizontalCenter
            }
            source: carLightsOn ? "qrc:/assets/newcar.svg" : "qrc:/assets/Car.svg"

            transform: [
                Translate { y: car.bounceY },
                Rotation {
                    origin.x: car.width / 2
                    origin.y: car.height
                    angle: car.bounceRot
                }
            ]

            // ===== XE NHÚN THỰC TẾ — mô phỏng giảm xóc =====
            SequentialAnimation {
                running: speedLabel.value > 10
                loops: Animation.Infinite

                // Xuống nhẹ (nén lò xo)
                NumberAnimation {
                    target: car; property: "bounceY"
                    from: 0; to: 3.5
                    duration: 160; easing.type: Easing.InQuad
                }
                // Bật lên quá vị trí cân bằng (lò xo nhả)
                NumberAnimation {
                    target: car; property: "bounceY"
                    from: 3.5; to: -4
                    duration: 220; easing.type: Easing.OutQuad
                }
                // Dao động tắt dần
                NumberAnimation {
                    target: car; property: "bounceY"
                    from: -4; to: 1.5
                    duration: 160; easing.type: Easing.InOutSine
                }
                NumberAnimation {
                    target: car; property: "bounceY"
                    from: 1.5; to: 0
                    duration: 120; easing.type: Easing.OutSine
                }
                // Nghỉ giữa các nhún
                PauseAnimation { duration: 480 }
            }

            // Xoay nhẹ độc lập — lệch nhẹ qua ổ gà
            SequentialAnimation {
                running: speedLabel.value > 10
                loops: Animation.Infinite

                PauseAnimation { duration: 250 }
                NumberAnimation {
                    target: car; property: "bounceRot"
                    from: 0; to: 0.7
                    duration: 220; easing.type: Easing.InOutSine
                }
                NumberAnimation {
                    target: car; property: "bounceRot"
                    from: 0.7; to: -0.5
                    duration: 260; easing.type: Easing.InOutSine
                }
                NumberAnimation {
                    target: car; property: "bounceRot"
                    from: -0.5; to: 0
                    duration: 180; easing.type: Easing.OutSine
                }
                PauseAnimation { duration: 430 }
            }
        }

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

        // Glow đèn pha — chỉ render ánh sáng, KHÔNG render vạch kẻ thứ 2
        Item {
            visible: car.carLightsOn
            anchors.fill: leftRoad
            Image {
                id: leftRoadGlowSrc
                anchors.fill: parent
                source: "qrc:/assets/Vector 2.svg"
                visible: false  // Ẩn source, Glow chỉ dùng texture
            }
            Glow {
                anchors.fill: leftRoadGlowSrc
                source: leftRoadGlowSrc
                radius: roadGlowRadius
                samples: 17
                color: "#FFE080"
                spread: roadGlowSpread
                transparentBorder: true
            }
        }

        // Xi nhan trái bên cạnh đường - hiển thị mờ
        Image {
            id: leftTurnIndicator
            width: 60
            height: 60
            visible: true
            opacity: 0.2  // Hiển thị mờ
            anchors{
                left: leftRoad.left
                leftMargin: -200
                verticalCenter: leftRoad.verticalCenter
            }
            source: "qrc:/assets/arrow_left.svg"
            Behavior on opacity { NumberAnimation { duration: 200 } }
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
                        text: "25.0°C"     // ← Giá trị mặc định
                        font.pixelSize: 32
                        font.family: "Inter"
                        font.bold: Font.Normal
                        font.capitalization: Font.AllUppercase
                        color: "#FFFFFF"
                    }

            Label{
                        id: humidityFromDHT11  // ← THÊM humidity display
                        text: "50°F"
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

        // Glow đèn pha — chỉ render ánh sáng, KHÔNG render vạch kẻ thứ 2
        Item {
            visible: car.carLightsOn
            anchors.fill: rightRoad
            Image {
                id: rightRoadGlowSrc
                anchors.fill: parent
                source: "qrc:/assets/Vector 1.svg"
                visible: false
            }
            Glow {
                anchors.fill: rightRoadGlowSrc
                source: rightRoadGlowSrc
                radius: roadGlowRadius
                samples: 17
                color: "#FFE080"
                spread: roadGlowSpread
                transparentBorder: true
            }
        }

        // Xi nhan phải bên cạnh đường - hiển thị mờ
        Image {
            id: rightTurnIndicator
            width: 60
            height: 60
            visible: true
            opacity: 0.2  // Hiển thị mờ
            anchors{
                right: rightRoad.right
                rightMargin: -200
                verticalCenter: rightRoad.verticalCenter
            }
            source: "qrc:/assets/arrow_right.svg"
            Behavior on opacity { NumberAnimation { duration: 200 } }
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

        SideGauge {
            id:leftGauge
            anchors{
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: parent.width / 7
            }
            property bool accelerating
            width: 400
            height: 400
            value: accelerating ? maximumValue : 0
            maximumValue: 250
            Component.onCompleted: forceActiveFocus()
            Behavior on value { NumberAnimation { duration: 1000 }}

            Keys.onSpacePressed: accelerating = true
            Keys.onReleased: {
                if (event.key === Qt.Key_Space) {
                    accelerating = false;
                    event.accepted = true;
                }
            }
        }

        // Progress Bar
        RadialBar {
            id:radialBar
            visible: false
            anchors{
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: parent.width / 6
            }

            width: 338
            height: 338
            penStyle: Qt.RoundCap
            dialType: RadialBar.NoDial
            progressColor: "#01E4E0"
            backgroundColor: "transparent"
            dialWidth: 17
            startAngle: 270
            spanAngle: 3.6 * value
            minValue: 0
            maxValue: 100
            value: accelerating ? maxValue : 65
            textFont {
                family: "inter"
                italic: false
                bold: Font.Medium
                pixelSize: 60
            }
            showText: false
            suffixText: ""
            textColor: "#FFFFFF"

            property bool accelerating
            Behavior on value { NumberAnimation { duration: 1000 }}

            ColumnLayout{
                anchors.centerIn: parent
                Label{
                    text: radialBar.value.toFixed(0) + "%"
                    font.pixelSize: 65
                    font.family: "Inter"
                    font.bold: Font.Normal
                    color: "#FFFFFF"
                    Layout.alignment: Qt.AlignHCenter
                }

                Label{
                    text: "Battery charge"
                    font.pixelSize: 28
                    font.family: "Inter"
                    font.bold: Font.Normal
                    opacity: 0.8
                    color: "#FFFFFF"
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        SideGauge {
            id:rightGauge
            anchors{
                verticalCenter: parent.verticalCenter
                right: parent.right
                rightMargin: parent.width / 7
            }
            property bool accelerating
            width: 400
            height: 400
            value: accelerating ? maximumValue : 0
            maximumValue: 250
            Component.onCompleted: forceActiveFocus()
            Behavior on value { NumberAnimation { duration: 1000 }}

            Keys.onSpacePressed: accelerating = true
            Keys.onReleased: {
                if (event.key === Qt.Key_Space) {
                    accelerating = false;
                    event.accepted = true;
                }
            }
        }

        ColumnLayout{
            visible: false
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
                        text: "188 KM"
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
                        text: "34 mpg"
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
                        text: "78 mph"
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
    // Thêm vào cuối Dashboard1.qml
        // CAN Handler Connections
        Connections {
            target: canHandler

            // Turn signals
            onLeftLightChanged: {
                leftTurnSignal.opacity = leftLight ? 1.0 : 0.2
                leftTurnIndicator.opacity = leftLight ? 1.0 : 0.2
            }

            onRightLightChanged: {
                rightTurnSignal.opacity = rightLight ? 1.0 : 0.2
                rightTurnIndicator.opacity = rightLight ? 1.0 : 0.2
            }

            onHazardLightsChanged: {
                leftTurnSignal.opacity = hazardLights ? 1.0 : 0.2
                leftTurnIndicator.opacity = hazardLights ? 1.0 : 0.2
                rightTurnSignal.opacity = hazardLights ? 1.0 : 0.2
                rightTurnIndicator.opacity = hazardLights ? 1.0 : 0.2
            }

            // Headlights
            onHighBeamChanged: {
                // Cập nhật hiển thị đèn pha cao
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
                // Update battery display if you have battery gauge
                console.log("Battery level:", battery)
            }

            // Temperature from CAN
            onTemperatureChanged: {
                    // Convert từ raw ADC value sang Celsius
                    // Công thức: temp_celsius = (temperature / 4095.0) * 50.0
                    var tempCelsius = (temperature / 4095.0) * 50.0
                    tempFromDHT11.text = tempCelsius.toFixed(1) + "°C"
                }
            onHumidityChanged: {
                    // Convert từ raw ADC value sang phần trăm
                    // Công thức: humidity_percent = (humidity / 4095.0) * 100.0
                    var humidityPercent = (humidity / 4095.0) * 100.0
                    humidityFromDHT11.text = humidityPercent.toFixed(0) + "%"
                }
            //  Encoder điều khiển cả 2 SideGauges
                    onEncoderSpeedChanged: {
                        leftGauge.value = encoderSpeed
                        rightGauge.value = encoderSpeed

                        // Debug log (có thể xóa sau khi test xong)
                        console.log("Encoder speed:", encoderSpeed)
                }
                     onButtonStateChanged: {
                            // buttonIndex: 0-15 (S1-S16)
                            // state: 0 or 1
                            if (buttonStatusPanel) {
                                buttonStatusPanel.updateButtonState(buttonIndex, state);
                            }
                        }
                     onCarLightsToggled: {
                             car.carLightsOn = state
                             roadGlowRadius = state ? 20 : 0;
                             roadGlowSpread = state ? 0.3 : 0.0;
                             console.log("S11 pressed: Car lights =", state)
                         }
                     onSpecificButtonPressed: {
        console.log("Button pressed:", buttonId)

        switch(buttonId) {
            case 0: // S1 - Parking Light
                forthLeftIndicator.parkingLightOn = !forthLeftIndicator.parkingLightOn
                break;
            case 1: // S2 - Main Light
                thirdLeftIndicator.lightOn = !thirdLeftIndicator.lightOn
                break;
            case 2: // S3 - Low Beam
                secondLeftIndicator.headLightOn = !secondLeftIndicator.headLightOn
                break;
            case 3: // S4 - Fog Light
                firstLeftIndicator.rareLightOn = !firstLeftIndicator.rareLightOn
                break;
            case 4: // S5 - Right Icon 4
                forthRightIndicator.indicator = !forthRightIndicator.indicator
                break;
            case 5: // S6 - Right Icon 3
                thirdRightIndicator.indicator = !thirdRightIndicator.indicator
                break;
            case 6: // S7 - Right Icon 2
                secondRightIndicator.indicator = !secondRightIndicator.indicator
                break;
            case 7: // S8 - Seat Belt
                firstRightIndicator.sheetBelt = !firstRightIndicator.sheetBelt
                break;

            case 8: // S9 - XI NHAN TRÁI
                if(leftTurnSignalTimer.running) {
                    // TẮT xi nhan trái
                    leftTurnSignalTimer.stop();
                    leftTurnSignal.opacity = 0.2;
                    leftTurnIndicator.opacity = 0.2;

                    // GỬI LỆNH TẮT
                    canHandler.sendOutputCommand(8, 0, 0);
                } else {
                    // Tắt xi nhan phải trước
                    rightTurnSignalTimer.stop();
                    rightTurnSignal.opacity = 0.2;
                    rightTurnIndicator.opacity = 0.2;
                    canHandler.sendOutputCommand(12, 0, 0);

                    // BẬT xi nhan trái
                    leftTurnSignalTimer.start();
                    // Lệnh ON sẽ được gửi trong Timer.onTriggered
                }
                break;

            case 9: // S10 - XI NHAN PHẢI
                if(rightTurnSignalTimer.running) {
                    rightTurnSignalTimer.stop();
                    rightTurnSignal.opacity = 0.2;
                    rightTurnIndicator.opacity = 0.2;

                    canHandler.sendOutputCommand(12, 0, 0);
                } else {
                    leftTurnSignalTimer.stop();
                    leftTurnSignal.opacity = 0.2;
                    leftTurnIndicator.opacity = 0.2;
                    canHandler.sendOutputCommand(8, 0, 0);

                    rightTurnSignalTimer.start();
                }
                break;

            default:
                console.log("Unknown button ID:", buttonId)
        }
        }
    }
}
}
