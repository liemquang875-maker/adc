#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// --- THÔNG TIN WIFI ---
const char* ssid     = "Liem";
const char* password = "matkhautu1den8";

ESP8266WebServer server(80);

#define ANALOG_PIN A0

// API gửi dữ liệu JSON cho JavaScript
void handleData() {
  int analogValue = analogRead(ANALOG_PIN);
  int percent = map(analogValue, 0, 1023, 0, 100);
  float voltage = (analogValue / 1023.0) * 3.3;

  String json = "{";
  json += "\"raw\":" + String(analogValue) + ",";
  json += "\"percent\":" + String(percent) + ",";
  json += "\"voltage\":" + String(voltage, 2);
  json += "}";

  server.send(200, "application/json", json);
}

// Giao diện HTML + CSS 3D Tank
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP8266 Water Tank Monitor</title>
    <style>
        body {
            font-family: 'Segoe UI', Arial, sans-serif;
            background-color: #eef2f5;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            margin: 0;
        }
        .container {
            background: #ffffff;
            padding: 30px;
            border-radius: 20px;
            box-shadow: 0 10px 25px rgba(0,0,0,0.1);
            text-align: center;
            width: 320px;
        }
        .tank-title {
            font-size: 20px;
            font-weight: bold;
            color: #333;
            margin-bottom: 5px;
        }
        .tank-id {
            font-size: 14px;
            color: #777;
            margin-bottom: 25px;
        }
        
        /* Khung bình chứa dạng 3D */
        .tank-wrapper {
            position: relative;
            width: 160px;
            height: 240px;
            margin: 0 auto;
        }
        .tank {
            position: absolute;
            width: 100%;
            height: 100%;
            border: 3px solid #7f8c8d;
            border-radius: 80px / 20px;
            background: rgba(230, 238, 248, 0.4);
            overflow: hidden;
            box-sizing: border-box;
        }
        
        /* Vạch chia độ bên cạnh bình */
        .ruler {
            position: absolute;
            right: -15px;
            top: 15px;
            bottom: 15px;
            width: 8px;
            display: flex;
            flex-direction: column;
            justify-content: space-between;
        }
        .ruler-line {
            width: 100%;
            height: 2px;
            background-color: #95a5a6;
        }

        /* Nước bên trong bình */
        .water {
            position: absolute;
            bottom: 0;
            width: 100%;
            height: 0%; /* Cập nhật qua JS */
            background: linear-gradient(180deg, #3498db, #2980b9);
            transition: height 0.6s ease-in-out, background 0.4s ease;
        }
        /* Mặt nước hình elip 3D */
        .water-surface {
            position: absolute;
            top: -12px;
            left: 0;
            width: 100%;
            height: 24px;
            background: #5dede7;
            border-radius: 50%;
            opacity: 0.8;
        }

        /* Cảnh báo khi mức nước quá thấp (< 20%) */
        .water.low {
            background: linear-gradient(180deg, #e74c3c, #c0392b);
        }
        .water.low .water-surface {
            background: #ff7675;
        }

        /* Nhãn hiển thị phần trăm cạnh bình */
        .percent-label {
            position: absolute;
            right: -60px;
            font-weight: bold;
            font-size: 16px;
            color: #2980b9;
            transition: bottom 0.6s ease-in-out;
        }
        .percent-label.low {
            color: #c0392b;
        }

        .info-panel {
            margin-top: 30px;
            background: #f8f9fa;
            padding: 12px;
            border-radius: 10px;
            font-size: 14px;
            color: #555;
        }
        .info-value {
            font-weight: bold;
            color: #2c3e50;
        }
    </style>
</head>
<body>

<div class="container">
    <div class="tank-title">Tank 1</div>
    <div class="tank-id">(ID: tank_1)</div>

    <div class="tank-wrapper">
        <div class="tank">
            <div class="water" id="waterLevel">
                <div class="water-surface"></div>
            </div>
        </div>
        
        <!-- Vạch thước đo -->
        <div class="ruler">
            <div class="ruler-line"></div>
            <div class="ruler-line"></div>
            <div class="ruler-line"></div>
            <div class="ruler-line"></div>
            <div class="ruler-line"></div>
            <div class="ruler-line"></div>
            <div class="ruler-line"></div>
            <div class="ruler-line"></div>
            <div class="ruler-line"></div>
        </div>

        <!-- Nhãn phần trăm đi theo mực nước -->
        <div class="percent-label" id="percentText" style="bottom: 0%;">0%</div>
    </div>

    <div class="info-panel">
        <div>Điện áp A0: <span class="info-value" id="voltageVal">0.00 V</span></div>
        <div>Tín hiệu ADC: <span class="info-value" id="rawVal">0 / 1023</span></div>
    </div>
</div>

<script>
    // Hàm gọi AJAX cập nhật dữ liệu từ ESP8266 mỗi 1 giây
    function updateTankData() {
        fetch('/data')
            .then(response => response.json())
            .then(data => {
                let percent = data.percent;
                let water = document.getElementById('waterLevel');
                let percentText = document.getElementById('percentText');

                // Cập nhật chiều cao nước và vị trí nhãn %
                water.style.height = percent + '%';
                percentText.style.bottom = Math.max(percent - 5, 0) + '%';
                percentText.innerText = percent + '%';

                // Đổi màu thành đỏ nếu mức nước dưới 20%
                if (percent < 20) {
                    water.classList.add('low');
                    percentText.classList.add('low');
                } else {
                    water.classList.remove('low');
                    percentText.classList.remove('low');
                }

                // Cập nhật chi tiết số liệu
                document.getElementById('voltageVal').innerText = data.voltage + ' V';
                document.getElementById('rawVal').innerText = data.raw + ' / 1023';
            })
            .catch(error => console.error('Lỗi kết nối:', error));
    }

    // Tự động đọc dữ liệu mỗi 1000ms (1 giây)
    setInterval(updateTankData, 1000);
    updateTankData();
</script>

</body>
</html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nKết nối WiFi thành công!");
  Serial.print("Địa chỉ IP ESP8266: ");
  Serial.println(WiFi.localIP());

  // Định nghĩa các đường dẫn Server
  server.on("/", handleRoot);
  server.on("/data", handleData);

  server.begin();
  Serial.println("Web Server đã khởi chạy!");
}

void loop() {
  server.handleClient();
}