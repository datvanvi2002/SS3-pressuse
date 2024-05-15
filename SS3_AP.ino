// #include <Scheduler.h>
#include <Arduino.h>
#include <DueTimer.h>

#include <Wire.h>

#include <Adafruit_GFX.h>

#include <Adafruit_SH1106.h>

// #define OLED_SDA 21
// #define OLED_SCL 22
// Adafruit_SH1106 display(21, 22);
// void displayMode() {

//   display.clearDisplay();
//   display.setTextColor(WHITE);
//   display.setTextSize(1);
//   display.setCursor(0, 0);
//   display.print("Set point : ");
//   display.print(setPointPessuse);
//   display.setCursor(0, 15);
//   display.print("Pressure : ");
//   display.print(pressure);
//   display.setCursor(0, 30);
//   display.print("Speed : ");
//   display.print(frequency);
//   display.display();
// }
// Khai bao bien PID
int status = 0;
const float kp = 2;
const float ki = 3;
const float kd = 0;
float last_error = 0;
float error = 0;
const int outputMax = 10000;  //fmax
const int outputMin = 100;    //fmin
float output;
float pressure = 0;
int frequency = 0;
float P, I;
//DRIVER STEP MOTTOR
const int pin_pulse = 2;      //pin control pulse,speed
const int pin_direct = 3;     //pin control direct
volatile bool state = false;  // Trạng thái của chân xung
// khai bao bien luu gia tri ap suat
String incomingChar = "";
String delimiter = " ";  // Ký tự phân cách
float setPointPressuse = 0;
//Khai bao chan Valve
const int val1 = 6;
const int val2 = 5;
const int deltaP = 30;
// Ham cap nhat trang thai chan pulse
void togglePin() {
  state = !state;                  // Đảo trạng thái
  digitalWrite(pin_pulse, state);  // Cập nhật trạng thái chân
}
void setpoint() {
  if (Serial.available() > 0) {                      // Kiểm tra xem có dữ liệu được gửi từ Seria Monitor hay không
    String setPoint = Serial.readStringUntil('\n');  // Đọc dữ liệu từ Serial Monitor đến khi gặp ký tự '\n' (ký tự xuống dòng)
    setPointPressuse = setPoint.toFloat();           // Chuyển đổi chuỗi sang số dạng float
    if (setPointPressuse > 100) {
      setPointPressuse = 100;
    }
    Serial.print("\n");
    Serial.print("Set Point Pressuse: ");
    Serial.println(setPointPressuse);  // In số float đã nhận được
  }
}
// Hàm cập nhật áp suất theo thời gian
void callPressure() {
  setpoint();
  //Gửi yêu cầu tới thiết bị
  Serial.println("Request: ");
  Serial1.write("#*?");
  Serial1.write(13);
  String incomingChar = "";
  delay(8);                       // thời gian nhỏ nhất 8ms
  if (Serial1.available() > 0) {  // Kiểm tra xem có dữ liệu nào được gửi đến không
    incomingChar = Serial1.readString();
    Serial.println(incomingChar);
    String data = "       ";
    for (int i = 0; i < incomingChar.length(); i++) {
      data[i] = incomingChar[i + 4];
    }
    pressure = data.toFloat();
    Serial.println("Pressure: ");
    Serial.println(pressure, 4);
    error = setPointPressuse - pressure;
    Serial.println(error);
  }
}
// Hàm điều khiển van

//PID hàm điều khiển động cơ
void PID() {
  callPressure();
  error = setPointPressuse - pressure;
  Serial.println("error");
  Serial.println(error);
  if (error > 0) {
    digitalWrite(pin_direct, 0);  // hướng quay thuận
  } else {
    digitalWrite(pin_direct, 1);  // hướng quay nghịch
  }
  error = abs(error);
  P = error * kp;
  I += error * ki;
  output = P + I;
  last_error = error;
  Serial.println(output);
  if (I > outputMax) I = outputMax;
  if (output > outputMax) output = outputMax;
  if (output < outputMin) output = outputMin;
  frequency = output;
  if (error == 0) {  //STOP
    I = 0;
    output = 0;
    frequency = 1;
    digitalWrite(pin_pulse, LOW);
    Timer3.stop();
  } else {
    //tone(pin_pulse, frequency);
    Timer3.attachInterrupt(togglePin).setFrequency(output).start();
  }
}
void setup() {
  Serial.begin(9600, SERIAL_8N1);
  Serial1.begin(9600, SERIAL_8N1);  // Khởi tạo giao tiếp UART với baud rate là 9600, 8 bit dữ liệu, no parity, 1 stop bit
  pinMode(pin_pulse, OUTPUT);
  pinMode(pin_direct, OUTPUT);
  // khai bao chan valve
  pinMode(val1, OUTPUT);
  pinMode(val2, OUTPUT);
  // 2 van mở , trong vòng Xs đẩy hết khí thì 2 van đóng
  digitalWrite(val1, 1);
  digitalWrite(val2, 1);
  delay(10000);
  while (setPointPressuse == 0) {
    setpoint();
  }
  digitalWrite(val2, 0);
//  display.clearDisplay();
  //display.begin(SH1106_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D
  //display.display();
}
//void updateSetpoint() {}
void loop() {
  //displayMode();
  Serial.println("loop");
  setpoint();
  if (pressure == setPointPressuse) {
    callPressure();
    digitalWrite(val1, 0);  // ngat bom AC
    digitalWrite(val2, 0);
  }
  if (deltaP < setPointPressuse - pressure) {
    while (1) {
      callPressure();
      //displayMode();
      digitalWrite(val1, 1);  // ngat bom AC
      digitalWrite(val2, 0);
      Serial.println("AC on");
      if (pressure == setPointPressuse) {
        Serial.println("AC off");
        digitalWrite(val1, 0);  // ngat bom AC
        break;
      }
    }
  } else {
    while (1) {
      //displayMode();
      if (pressure > setPointPressuse) {
        digitalWrite(val2, 1);
        callPressure();
        if (int(pressure) == int(setPointPressuse)) {
          digitalWrite(val2, 0);
          break;
        }
      } else {
        PID();
        digitalWrite(val2, 0);  //khoas van
        if (pressure == setPointPressuse) {
          break;
        }
      }
    }
  }
}
