#include "canhandler.h"
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <cstring>
#include <QDebug>

digInSignal digInput;
digOutSignal digOutput;
analogInSignal analogInput;
uint64_t softTimer = 0;
uint16_t tick500ms = 0;
uint16_t prevTick500ms = 0xFFFF;
static bool car_lights_state = false;

IOConfig loadIOConfig(const QString& path) {
    IOConfig config;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file:" << path;
        return config;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return config;
    }

    QJsonObject obj = doc.object();

    if (obj.contains("digital_inputs")) {
        QJsonObject inputsObj = obj["digital_inputs"].toObject();
        for (const QString& key : inputsObj.keys()) {
            config.digInputs[key] = static_cast<uint8_t>(inputsObj[key].toInt());
        }
    }

    if (obj.contains("analog_inputs")) {
        QJsonObject analogInputsObj = obj["analog_inputs"].toObject();
        for (const QString& key : analogInputsObj.keys()) {
            config.analogInputs[key] = static_cast<uint8_t>(analogInputsObj[key].toInt());
        }
    }

    if (obj.contains("digital_outputs")) {
        QJsonObject outputsObj = obj["digital_outputs"].toObject();
        for (const QString& key : outputsObj.keys()) {
            config.digOutputs[key] = static_cast<uint8_t>(outputsObj[key].toInt());
        }
    }

    return config;
}

CanTxThread::CanTxThread(QObject *parent)
    : QThread(parent), m_socket(-1), m_running(false) {}

CanTxThread::~CanTxThread() { stop(); }

void CanTxThread::enqueueMessage(const struct can_frame &frame) {
    QMutexLocker locker(&m_mutex);//API hỗ trợ thư viện trong QTCore
    m_queue.enqueue(frame);
}

void CanTxThread::stop() {
    m_running = false;
    wait();

    QMutexLocker locker(&m_mutex);
    m_queue.clear();

    if (m_socket >= 0) {
        close(m_socket);
        m_socket = -1;
    }
}

// ============ CanTxThread::run() ============

void CanTxThread::run() {
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame txFrame;
    
    struct {
        uint32_t last_frame_id[8] = {0}; // Luu 8 frame IDs
        uint8_t last_frame_data[8][8] = {0}; // Luu data cache 8 frames
        uint64_t last_send_time[8] = {0}; // Timestamp
    } frame_cache;
    
    uint64_t current_time_ms = 0;
    const uint64_t MIN_FRAME_INTERVAL_MS = 100;// 1 slot cache tối thiểu cách nhau 100 ms

    txFrame.can_dlc = BYTES_PER_CAN_FRAME;

    m_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (m_socket < 0) {
        qWarning() << "TX: Error opening CAN socket";
        return;
    }

    // set SOCKET TIMEOUT - Tang lên 1 giây
    struct timeval tv;
    tv.tv_sec = 1;  // Tang tu 0.5s lên 1s
    tv.tv_usec = 0;
    setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    // THÊM loopack là gửi ra rồi thì không nhận lại frame của mình
    int loopback = 0;
    setsockopt(m_socket, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));

    // ERROR FILTER
    can_err_mask_t err_mask = CAN_ERR_MASK;
    setsockopt(m_socket, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask));

    strcpy(ifr.ifr_name, "can0");
    if (ioctl(m_socket, SIOCGIFINDEX, &ifr) < 0) {
        qWarning() << "TX: Fail to specify CAN interface";
        close(m_socket);
        return;
    }
    
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    
    if (bind(m_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        qWarning() << "TX: Error binding CAN socket";
        close(m_socket);
        return;
    }

    m_running = true;
    int consecutive_errors = 0;
    bool prev_turn_state = false;
    uint16_t last_processed_tick = 0xFFFF; // ? THÊM: Chi xu lý moi tick 1 l?n

    while (m_running) {
        // KHI TICK THAY DOI(moi 500ms nháy 1 lan)
        if (tick500ms != last_processed_tick) {
            last_processed_tick = tick500ms;
            bool on = (tick500ms % 2 == 0);
            bool state_changed = (on != prev_turn_state);
            prev_turn_state = on;
            current_time_ms = softTimer * 10; // Tính th?i gian hi?n t?i (10ms per tick)

            // ===== HÀM HELPER: Kiem tra xem có con gui frame không =====
            auto shouldSendFrame = [&](uint32_t can_id, uint8_t *data, int cache_idx) -> bool {
                if (cache_idx >= 8) return false;
                
                // Kiem tra interval
                if (current_time_ms - frame_cache.last_send_time[cache_idx] < MIN_FRAME_INTERVAL_MS) {
                    return false; // Quá g?n v?i l?n g?i tru?c
                }
                
                // Kiem tra data có thay doi không
                if (frame_cache.last_frame_id[cache_idx] == can_id) {
                    if (memcmp(frame_cache.last_frame_data[cache_idx], data, 8) == 0) {
                        return false; // Data giong het, không can gui
                    }
                }
                
                return true;
            };

            // HÀM HELPER: Luu cache sau khi gui
            auto updateCache = [&](uint32_t can_id, uint8_t *data, int cache_idx) {
                if (cache_idx >= 8) return;
                frame_cache.last_frame_id[cache_idx] = can_id;
                memcpy(frame_cache.last_frame_data[cache_idx], data, 8);
                frame_cache.last_send_time[cache_idx] = current_time_ms;
            };

            // ===== LEFT TURN SIGNALS =====
            if (digOutput.left_front_light && digOutput.left_rear_light) {
                uint8_t left_front_val = on ? (0xC8 | 0x01) : 0xC8; //C9:C8
                uint8_t left_rear_val = on ? (0xC8 | 0x01) : 0xC8;  //C9:C8

                // Left front light
                uint32_t left_front_id = DIGITAL_OUTPUT_CMD_ID(digOutput.left_front_light_pos / DIGITAL_OUT_CMD_SIGNAL_PER_FRAME);
                memset(txFrame.data, 0xC8, 8);
                txFrame.data[digOutput.left_front_light_pos % DIGITAL_OUT_CMD_SIGNAL_PER_FRAME] = left_front_val;
                
                if (shouldSendFrame(left_front_id, txFrame.data, 0)) {
                    txFrame.can_id = left_front_id;
                    enqueueMessage(txFrame);
                    updateCache(left_front_id, txFrame.data, 0);
                }

                // Left rear light
                uint32_t left_rear_id = DIGITAL_OUTPUT_CMD_ID(digOutput.left_rear_light_pos / DIGITAL_OUT_CMD_SIGNAL_PER_FRAME);
                memset(txFrame.data, 0xC8, 8);
                txFrame.data[digOutput.left_rear_light_pos % DIGITAL_OUT_CMD_SIGNAL_PER_FRAME] = left_rear_val;
                
                if (shouldSendFrame(left_rear_id, txFrame.data, 1)) {
                    txFrame.can_id = left_rear_id;
                    enqueueMessage(txFrame);
                    updateCache(left_rear_id, txFrame.data, 1);
                }

                // Khi các code tren đã được thì set emit cho UI thuc hien viec nhap nhay
                if (state_changed) {
                    if (digInput.hazard_switch) {
                        emit hazardLightsChanged(on);
                    } else if (digInput.turn_left_switch) {
                        emit leftLightChanged(on);
                    }
                }
            }
            // Turn OFF left signals
            else if (!digOutput.left_front_light && !digOutput.left_rear_light) {
                uint32_t left_front_id = DIGITAL_OUTPUT_CMD_ID(digOutput.left_front_light_pos / DIGITAL_OUT_CMD_SIGNAL_PER_FRAME);
                memset(txFrame.data, 0xC8, 8);
                
                if (shouldSendFrame(left_front_id, txFrame.data, 0)) {
                    txFrame.can_id = left_front_id;
                    enqueueMessage(txFrame);
                    updateCache(left_front_id, txFrame.data, 0);
                }

                uint32_t left_rear_id = DIGITAL_OUTPUT_CMD_ID(digOutput.left_rear_light_pos / DIGITAL_OUT_CMD_SIGNAL_PER_FRAME);
                if (shouldSendFrame(left_rear_id, txFrame.data, 1)) {
                    txFrame.can_id = left_rear_id;
                    enqueueMessage(txFrame);
                    updateCache(left_rear_id, txFrame.data, 1);
                }

                if (state_changed) {
                    emit hazardLightsChanged(false);
                    emit leftLightChanged(false);
                }
            }

            // ===== RIGHT TURN SIGNALS (Tuong t?) =====
            if (digOutput.right_front_light && digOutput.right_rear_light) {
                uint8_t right_front_val = on ? (0xC8 | 0x01) : 0xC8;
                uint8_t right_rear_val = on ? (0xC8 | 0x01) : 0xC8;

                uint32_t right_front_id = DIGITAL_OUTPUT_CMD_ID(digOutput.right_front_light_pos / DIGITAL_OUT_CMD_SIGNAL_PER_FRAME);
                memset(txFrame.data, 0xC8, 8);
                txFrame.data[digOutput.right_front_light_pos % DIGITAL_OUT_CMD_SIGNAL_PER_FRAME] = right_front_val;
                
                if (shouldSendFrame(right_front_id, txFrame.data, 2)) {
                    txFrame.can_id = right_front_id;
                    enqueueMessage(txFrame);
                    updateCache(right_front_id, txFrame.data, 2);
                }

                uint32_t right_rear_id = DIGITAL_OUTPUT_CMD_ID(digOutput.right_rear_light_pos / DIGITAL_OUT_CMD_SIGNAL_PER_FRAME);
                memset(txFrame.data, 0xC8, 8);
                txFrame.data[digOutput.right_rear_light_pos % DIGITAL_OUT_CMD_SIGNAL_PER_FRAME] = right_rear_val;
                
                if (shouldSendFrame(right_rear_id, txFrame.data, 3)) {
                    txFrame.can_id = right_rear_id;
                    enqueueMessage(txFrame);
                    updateCache(right_rear_id, txFrame.data, 3);
                }

                if (state_changed) {
                    if (digInput.hazard_switch) {
                        emit hazardLightsChanged(on);
                    } else if (digInput.turn_right_switch) {
                        emit rightLightChanged(on);
                    }
                }
            }
            else if (!digOutput.right_front_light && !digOutput.right_rear_light) {
                uint32_t right_front_id = DIGITAL_OUTPUT_CMD_ID(digOutput.right_front_light_pos / DIGITAL_OUT_CMD_SIGNAL_PER_FRAME);
                memset(txFrame.data, 0xC8, 8);
                
                if (shouldSendFrame(right_front_id, txFrame.data, 2)) {
                    txFrame.can_id = right_front_id;
                    enqueueMessage(txFrame);
                    updateCache(right_front_id, txFrame.data, 2);
                }

                uint32_t right_rear_id = DIGITAL_OUTPUT_CMD_ID(digOutput.right_rear_light_pos / DIGITAL_OUT_CMD_SIGNAL_PER_FRAME);
                if (shouldSendFrame(right_rear_id, txFrame.data, 3)) {
                    txFrame.can_id = right_rear_id;
                    enqueueMessage(txFrame);
                    updateCache(right_rear_id, txFrame.data, 3);
                }

                if (state_changed) {
                    emit hazardLightsChanged(false);
                    emit rightLightChanged(false);
                }
            }
        }

        // ===== XU LÝ QUEUE Va RATE LIMIT =====
        m_mutex.lock();
        bool has_frame = !m_queue.isEmpty();

        if (has_frame) {
            struct can_frame frame = m_queue.dequeue();
            m_mutex.unlock();

            int nbytes = write(m_socket, &frame, sizeof(frame));//bắn frame ra CAN:
            //m_socket là socket đã bind vào can0
            //write() đẩy frame vào kernel → driver → CAN controller → bus CAN


            if (nbytes < 0) {
                consecutive_errors++;
                qWarning() << "TX Error" << consecutive_errors << ":" << strerror(errno);

                // reset if error more
                if (consecutive_errors > 5) { // Gi?m t? 10 xu?ng 5
                    qWarning() << "TX: Resetting socket...";
                    close(m_socket);
                    usleep(200000); // Tang delay lên 200ms

                    m_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
                    if (m_socket >= 0) {
                        strcpy(ifr.ifr_name, "can0");
                        ioctl(m_socket, SIOCGIFINDEX, &ifr);
                        addr.can_family = AF_CAN;
                        addr.can_ifindex = ifr.ifr_ifindex;
                        bind(m_socket, (struct sockaddr *)&addr, sizeof(addr));

                        tv.tv_sec = 1;
                        tv.tv_usec = 0;
                        setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
                        setsockopt(m_socket, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));
                        setsockopt(m_socket, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask));
                    }
                    consecutive_errors = 0;
                    
                    // ? XÓA CACHE d? g?i l?i frame sau khi reset
                    memset(&frame_cache, 0, sizeof(frame_cache));
                }
            } else {
                consecutive_errors = 0;
            }

           
            usleep(10000); 

        } else {
            m_mutex.unlock();
            usleep(5000); 
        }
    }

    close(m_socket);
}
// Gửi lệnh Digital Output Command qua CAN
void CanTxThread::sendDigitalOutputCommand(uint8_t outputIndex, uint8_t switchCmd, uint8_t dutyCycle) {
    struct can_frame frame;
    
    // Tính frame và signal index
    uint8_t frameIndex = outputIndex / DIGITAL_OUT_CMD_SIGNAL_PER_FRAME;
    uint8_t signalIndex = outputIndex % DIGITAL_OUT_CMD_SIGNAL_PER_FRAME; // CAN frame có 8 byte data, và quy ước 1 output = 1 byte)
    
    // CAN ID
    frame.can_id = DIGITAL_OUTPUT_CMD_ID(frameIndex) | (1U << 31); // Extended ID
    frame.can_dlc = 8;
    
    // Tạo data frame (mặc định tất cả = 0xC8 = switch OFF)
    memset(frame.data, 0xC8, 8);
    
    // Set byte cho output cụ thể
    // Mỗi signal = 1 byte: bit[0] = switchCmd, bit[1-7] = dutyCycle
    frame.data[signalIndex] = (switchCmd & 0x01) | ((dutyCycle & 0x7F) << 1);
    
    qDebug() << "Sending CAN CMD: Output" << outputIndex
             << "Frame" << frameIndex << "Signal" << signalIndex
             << "Switch:" << switchCmd << "Duty:" << dutyCycle;
    
    enqueueMessage(frame);
}

CanRxThread::CanRxThread(QObject *parent)
    : QThread(parent), m_socket(-1), m_running(false) {}

CanRxThread::~CanRxThread() {
    stop();
}

void CanRxThread::stop() {
    m_running = false;
    wait();

    if (m_socket >= 0) {
        close(m_socket);
        m_socket = -1;
    }
}

void CanRxThread::run() {
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame rx_frame;
    digInSignal prevInput;
    uint8_t signalIdx = 0xFF;
    uint32_t signalCANID = 0;
    uint8_t signal_value = 0;
    int analogValue = 0;

    IOConfig config = loadIOConfig(":/io_configs/io_config.json");
    if (config.digInputs.isEmpty() || config.analogInputs.isEmpty() || config.digOutputs.isEmpty()) {
        qWarning() << "Failed to load IO configuration.";
        return;
    }

    // Load digital output positions
    for (auto it = config.digOutputs.constBegin(); it != config.digOutputs.constEnd(); ++it) {
        signalIdx = static_cast<uint8_t>(it.value());
        if (it.key() == "left_front_light") {
            digOutput.left_front_light_pos = signalIdx;
        } else if (it.key() == "left_rear_light") {
            digOutput.left_rear_light_pos = signalIdx;
        } else if (it.key() == "right_front_light") {
            digOutput.right_front_light_pos = signalIdx;
        } else if (it.key() == "right_rear_light") {
            digOutput.right_rear_light_pos = signalIdx;
        }
    }

    m_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (m_socket < 0) {
        qWarning() << "RX: Error opening CAN socket";
        return;
    }

    // RX TIMEOUT
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    strcpy(ifr.ifr_name, "can0");
    if (ioctl(m_socket, SIOCGIFINDEX, &ifr) < 0) {
        qWarning() << "RX: Fail to specify CAN interface";
        close(m_socket);
        return;
    }
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(m_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        qWarning() << "RX: Error binding CAN socket";
        close(m_socket);
        return;
    }

    m_running = true;
    while (m_running) {
        int nbytes = read(m_socket, &rx_frame, sizeof(struct can_frame));
        if (nbytes > 0) {
            // Process digital inputs

    for (auto it = config.digInputs.constBegin(); it != config.digInputs.constEnd(); ++it) {
    signalIdx = static_cast<uint8_t>(it.value());
    uint8_t frameIndex = signalIdx / DIGITAL_IN_RESP_SIGNAL_PER_FRAME;
    signalCANID = DIGITAL_INPUT_RES_ID(frameIndex);

    if (rx_frame.can_id == signalCANID) {
        uint8_t byteIndex = signalIdx % DIGITAL_IN_RESP_SIGNAL_PER_FRAME;
        uint8_t rawByte = rx_frame.data[byteIndex];
        uint8_t inputStatus = rawByte & 0x01;
        
        // ?? ALWAYS emit buttonStateChanged for ALL buttons
        emit buttonStateChanged(signalIdx, inputStatus);
        
        // Check if this is button_s1 to button_s8
        // Phát hiện button S1-S10 và ĐIỀU KHIỂN OUTPUT
        if (it.key().startsWith("button_s")) {
            QString buttonNumStr = it.key().mid(8);
            bool ok;
            int buttonNum = buttonNumStr.toInt(&ok);

            if (ok && buttonNum >= 1 && buttonNum <= 16) {  // ← Mở rộng đến S16
                int buttonId = buttonNum - 1;

                static uint8_t prev_button_states[16] = {0};  // ← Tăng lên 16 buttons

                if (inputStatus == 1 && prev_button_states[buttonId] == 0) {
                    qDebug() << "Button S" << buttonNum << "(ID:" << buttonId << ") PRESSED";
                    emit specificButtonPressed(buttonId);

                    //XỬ LÝ ĐẶC BIỆT CHO S11 (buttonId = 10)
                    if (buttonId == 10) {
                        car_lights_state = !car_lights_state;
                        emit carLightsToggled(car_lights_state);
                        qDebug() << "S11: Car lights toggled to" << car_lights_state;
    
    
                    if (car_lights_state) {
        // Bật đèn đỏ
                    emit sendDigitalOutputRequest(22, 1, 100);  // Output 22, Switch ON, Duty 100%
                }   else {
        // Tắt đèn đỏ
                    emit sendDigitalOutputRequest(22, 0, 0);    // Output 22, Switch OFF
    }
}
                }

                prev_button_states[buttonId] = inputStatus;
            }
        }

        
        // Handle special switches (ignition, turn signals, etc.)
        if (it.key() == "ignition") {
            if (digInput.ignition != inputStatus) {
                digInput.ignition = inputStatus;
                qDebug() << "?? Ignition changed to:" << digInput.ignition;
            }
        }
        else if (it.key() == "turn_left_switch") {
            if (digInput.turn_left_switch != inputStatus) {
                digInput.turn_left_switch = inputStatus;
                qDebug() << "?? Turn left switch:" << digInput.turn_left_switch;
            }
        }
        else if (it.key() == "turn_right_switch") {
            if (digInput.turn_right_switch != inputStatus) {
                digInput.turn_right_switch = inputStatus;
                qDebug() << "?? Turn right switch:" << digInput.turn_right_switch;
            }
        }
        else if (it.key() == "hazard_switch") {
            if (digInput.hazard_switch != inputStatus) {
                digInput.hazard_switch = inputStatus;
                qDebug() << "?? Hazard switch:" << digInput.hazard_switch;
            }
        }
        else if (it.key() == "high_beam_switch") {
            if (digInput.high_beam_switch != inputStatus) {
                digInput.high_beam_switch = inputStatus;
                emit highBeamChanged(digInput.high_beam_switch);
            }
        }
        else if (it.key() == "low_beam_switch") {
            if (digInput.low_beam_switch != inputStatus) {
                digInput.low_beam_switch = inputStatus;
                emit lowBeamChanged(digInput.low_beam_switch);
            }
        }
        else if (it.key() == "parking_lights_switch") {
            if (digInput.parking_lights_switch != inputStatus) {
                digInput.parking_lights_switch = inputStatus;
                emit parkingLightsChanged(digInput.parking_lights_switch);
            }
        }
    }
}

            // Handle turn signal state changes
            if (digInput.hazard_switch && digInput.hazard_switch != prevInput.hazard_switch) {
                softTimer = 0;
                tick500ms = 0;
                prevTick500ms = 0xFFFF;
                digOutput.left_front_light = true;
                digOutput.left_rear_light = true;
                digOutput.right_front_light = true;
                digOutput.right_rear_light = true;
            } else if (digInput.turn_left_switch && digInput.turn_left_switch != prevInput.turn_left_switch) {
                softTimer = 0;
                tick500ms = 0;
                prevTick500ms = 0xFFFF;
                digOutput.left_front_light = true;
                digOutput.left_rear_light = true;
                digOutput.right_front_light = false;
                digOutput.right_rear_light = false;
            } else if (digInput.turn_right_switch && digInput.turn_right_switch != prevInput.turn_right_switch) {
                softTimer = 0;
                tick500ms = 0;
                prevTick500ms = 0xFFFF;
                digOutput.left_front_light = false;
                digOutput.left_rear_light = false;
                digOutput.right_front_light = true;
                digOutput.right_rear_light = true;
            } else if ((digInput.hazard_switch == false && digInput.turn_left_switch == false && digInput.turn_right_switch == false) &&
                       (prevInput.hazard_switch || prevInput.turn_left_switch || prevInput.turn_right_switch)) {
                softTimer = 0;
                tick500ms = 0;
                prevTick500ms = 0xFFFF;
                digOutput.left_front_light = false;
                digOutput.left_rear_light = false;
                digOutput.right_front_light = false;
                digOutput.right_rear_light = false;
            }

            // Process analog inputs (speed, battery, temperature, humidity)
        for (auto it = config.analogInputs.constBegin(); it != config.analogInputs.constEnd(); ++it) {
    // index c?a tín hi?u trong io_config.json (0 = speed, 1 = battery, 2 = temp, 3 = humidity)
            uint8_t signalIndex = static_cast<uint8_t>(it.value());

    // M?i frame analog mang ANALOG_IN_RESP_SIGNAL_PER_FRAME tín hi?u
    uint8_t frameIndex = signalIndex / ANALOG_IN_RESP_SIGNAL_PER_FRAME;
    canid_t signalCANID = ANALOG_INPUT_RES_ID(frameIndex);

    // chi xu ly khi nhan dung frame ID
    if (rx_frame.can_id != signalCANID)
        continue;

    // M?i tín hi?u chi?m 2 byte trong frame
    uint8_t byteIndex = (signalIndex % ANALOG_IN_RESP_SIGNAL_PER_FRAME) * 2;

    uint16_t raw = static_cast<uint16_t>(rx_frame.data[byteIndex]) |
                   (static_cast<uint16_t>(rx_frame.data[byteIndex + 1]) << 8);

    // L?y 14 bit giá tr? analog (ph?n còn l?i là ch?n doán)
    uint16_t analogValue = raw & ((1u << ANALOG_VALUE_BITS) - 1u);
    // uint8_t diag = (raw >> ANALOG_VALUE_BITS) & ((1u << ANALOG_EL_DIAGNOSIS_BITS) - 1u);

    if (it.key() == "speed") {
        if (analogInput.speed != analogValue) {
            analogInput.speed = analogValue;
            emit speedChanged(analogInput.speed);
        }
    } else if (it.key() == "battery") {
        if (analogInput.battery != analogValue) {
            analogInput.battery = analogValue;
            emit batteryChanged(analogInput.battery);
        }
    } else if (it.key() == "temperature") {
        if (analogInput.temperature != analogValue) {
            analogInput.temperature = analogValue;
            emit temperatureChanged(analogInput.temperature);
        }
    } else if (it.key() == "humidity") {
        if (analogInput.humidity != analogValue) {
            analogInput.humidity = analogValue;
            emit humidityChanged(analogInput.humidity);
        }
    }
    // Encoder
    else if (it.key() == "encoder") {
        if (analogInput.encoder != analogValue) {
            analogInput.encoder = analogValue;
            // Convert ADC (0-4095) back to speed (0-250)
            int calculatedSpeed = (analogInput.encoder * 250) / 4095;
            emit encoderSpeedChanged(calculatedSpeed);
        }
    }
}


            memcpy(&prevInput, &digInput, sizeof(digInSignal));
        }
    }
    close(m_socket);
}

//TIMER 10MS
DataProcessing::DataProcessing() {
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DataProcessing::DataProcessingTask);
    timer->start(10); // 10ms
}

void DataProcessing::DataProcessingTask() {
    tick500ms = softTimer / 50; // 500ms / 10ms = 50
    softTimer++;
}

CanHandler::CanHandler(QObject *parent)
    : QObject(parent)
{
    m_txThread = new CanTxThread(this);
    m_rxThread = new CanRxThread(this);
    m_dataProcessing = new DataProcessing();

    connect(m_txThread, &CanTxThread::leftLightChanged, this, &CanHandler::leftLightChanged);
    connect(m_txThread, &CanTxThread::rightLightChanged, this, &CanHandler::rightLightChanged);
    connect(m_txThread, &CanTxThread::hazardLightsChanged, this, &CanHandler::hazardLightsChanged);
    connect(m_rxThread, &CanRxThread::highBeamChanged, this, &CanHandler::highBeamChanged);
    connect(m_rxThread, &CanRxThread::lowBeamChanged, this, &CanHandler::lowBeamChanged);
    connect(m_rxThread, &CanRxThread::parkingLightsChanged, this, &CanHandler::parkingLightsChanged);
    connect(m_rxThread, &CanRxThread::speedChanged, this, &CanHandler::speedChanged);
    connect(m_rxThread, &CanRxThread::batteryChanged, this, &CanHandler::batteryChanged);
    connect(m_rxThread, &CanRxThread::temperatureChanged, this, &CanHandler::temperatureChanged);
    connect(m_rxThread, &CanRxThread::humidityChanged, this, &CanHandler::humidityChanged);
    connect(m_rxThread, &CanRxThread::encoderSpeedChanged, this, &CanHandler::encoderSpeedChanged);
    connect(m_rxThread, &CanRxThread::buttonStateChanged, 
            this, &CanHandler::buttonStateChanged);
    connect(m_rxThread, &CanRxThread::specificButtonPressed,
            this, &CanHandler::specificButtonPressed);
    connect(m_rxThread, &CanRxThread::carLightsToggled,
                this, &CanHandler::carLightsToggled);
    connect(m_rxThread, &CanRxThread::sendDigitalOutputRequest,
            m_txThread, &CanTxThread::sendDigitalOutputCommand);


    m_txThread->start();
    m_rxThread->start();
}

CanHandler::~CanHandler() {
    m_txThread->stop();
    m_rxThread->stop();
}
