/*
 * Control PiccoRobo via V-Sido like Serial Command
 * by Daisuke IMAI <hine.gdw@gmail.com> 2016/04/22
 * 
 *
 */

#include <Servo.h>
#include <FlexiTimer2.h>
#include <Adafruit_NeoPixel.h>

// デバグ用LEDの定義
#define LED_PIN 13

// シリアル通信のための設定
#define SERIAL_BAUDRATE 115200

// サーボ関連定義
#define SERVO_NUM 20 // 接続可能なサーボの数の上限値(Piccoroboの場合20)

#define SERVO_BODY 4 // 身体のサーボのピン番号
#define SERVO_RIGHT_FOOT 5 // 右足のサーボのピン番号
#define SERVO_LEFT_FOOT 6 // 左足のサーボのピン番号
#define SERVO_NECK 7 // // 首のサーボのピン番号

#define SERVO_BODY_TRIM 4.5 // 身体のサーボのトリム調整
#define SERVO_RIGHT_FOOT_TRIM 6 // 右足のサーボのトリム調整
#define SERVO_LEFT_FOOT_TRIM -12 // 左足のサーボのトリム調整
#define SERVO_NECK_TRIM 0 // // 首のサーボのトリム調整

#define SERVO_BODY_VSIDO_SID 2 // 身体のサーボのV-Sidoにおけるsid
#define SERVO_RIGHT_FOOT_VSIDO_SID 9 // 右足のサーボのV-Sidoにおけるsid
#define SERVO_LEFT_FOOT_VSIDO_SID 15 // 左足のサーボのV-Sidoにおけるsid
#define SERVO_NECK_VSIDO_SID 1 // 首のサーボのV-Sidoにおけるsid

#define SERVO_PWM_CENTER 1500 // 0度を何uSで表現するかの調整値
#define SERVO_PWM_RANGE 2000 // 180度幅を何uSで表現するかの調整値

// 目のLEDのための定義
#define NEOPIXEL_PIN 3
#define NEOPIXEL_NUM 2
#define NEOPIXEL_RIGHT_EYE 0
#define NEOPIXEL_LEFT_EYE 1

// ロボットの状態変化のFPS
#define ROBOT_FPS 100

// V-Sidoコマンド関連定義
#define VSIDO_ST 0xff
#define VSIDO_OP_ACK 0x21
#define VSIDO_OP_ANGLE 0x6f
#define VSIDO_OP_GET_VID_VALUE 0x67
#define VSIDO_OP_EXTRA_COMMAND 0x78

#define VSIDO_SID_MAX 254

// V-Sido拡張コマンド(勝手仕様)を使用するため、このロボットの拡張IDを仮に200、コマンド番号は0番と設定
#define VSIDO_EXTRA_ID 200
#define VSIDO_EXTRA_LED 0

// シリアル通信のためのバッファ用意
byte received_buffer [256];
byte response_buffer [256];
int received_length = 0;

// サーボのための配列
boolean servo_use [SERVO_NUM];
float servo_trim_angle [SERVO_NUM];
float servo_target_angle [SERVO_NUM];
float servo_current_angle [SERVO_NUM];
int servo_remaining_count [SERVO_NUM];

// sidとサーボのピン番号との変換テーブル
int sid_pin_translation [VSIDO_SID_MAX + 1];

// サーボのインスタンス
Servo servo[SERVO_NUM];

// LEDのための配列
byte led_value [NEOPIXEL_NUM][3];
boolean led_changed [NEOPIXEL_NUM];

// 目のLEDのインスタンス
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEOPIXEL_NUM, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // シリアル準備
  Serial.begin(115200);

  // 出力ピンの設定
  pinMode(LED_PIN, OUTPUT);

  // 目のLED初期化
  init_eyes();

  // 目を緑に光らせる
  set_eye_color(NEOPIXEL_RIGHT_EYE, 80, 120, 20);
  set_eye_color(NEOPIXEL_LEFT_EYE, 80, 120, 20);

  // サーボ初期化
  init_servo();

  FlexiTimer2::set(1000 / ROBOT_FPS, change_robot_state);
  FlexiTimer2::start();

}

void loop() {
  receive_command();
  delay(1);
}

void change_robot_state(void) {
  change_led_state();
  change_servo_state();
}

void init_eyes(void) {
  pixels.begin();
  for (int i = 0; i < NEOPIXEL_NUM; i++) {
    set_eye_color(i, 0, 0, 0);
  }
}

void set_eye_color(unsigned int led_num, byte r, byte g, byte b) {
  if (led_num < NEOPIXEL_NUM) {
    led_value[led_num][0] = r;
    led_value[led_num][1] = g;
    led_value[led_num][2] = b;
    led_changed[led_num] = true;
  }
}

void change_led_state(void) {
  int change_num = 0;
  for (int i = 0; i < NEOPIXEL_NUM; i++) {
    if (led_changed[i]) {
      pixels.setPixelColor(i, pixels.Color(led_value[i][0], led_value[i][1], led_value[i][2]));
      led_changed[i] = false;
      change_num++;
    }
  }
  if (change_num > 0) {
    pixels.show();
  }
}
void init_servo(void) {
  //サーボ関連変数初期化
  for (int i = 0; i < SERVO_NUM; i++) {
    servo_use[i] = false;
    servo_trim_angle[i] = 0;
    servo_target_angle[i] = 0;
    servo_current_angle[i] = 0;
  }

  // サーボの利用可否を決める
  servo_use[SERVO_BODY] = true;
  servo_use[SERVO_RIGHT_FOOT] = true;
  servo_use[SERVO_LEFT_FOOT] = true;
  servo_use[SERVO_NECK] = true;

  // サーボのトリム値の設定
  servo_trim_angle[SERVO_BODY] = SERVO_BODY_TRIM;
  servo_trim_angle[SERVO_RIGHT_FOOT] = SERVO_RIGHT_FOOT_TRIM;
  servo_trim_angle[SERVO_LEFT_FOOT] = SERVO_LEFT_FOOT_TRIM;
  servo_trim_angle[SERVO_NECK] = SERVO_NECK_TRIM;

  // sidとの変換テーブル設定
  sid_pin_translation[SERVO_BODY_VSIDO_SID] = SERVO_BODY;
  sid_pin_translation[SERVO_RIGHT_FOOT_VSIDO_SID] = SERVO_RIGHT_FOOT;
  sid_pin_translation[SERVO_LEFT_FOOT_VSIDO_SID] = SERVO_LEFT_FOOT;
  sid_pin_translation[SERVO_NECK_VSIDO_SID] = SERVO_NECK;

  // サーボをアタッチする
  for (int i = 0; i < SERVO_NUM; i++) {
    if (servo_use[i]) {
      servo[i].attach(i);
    }
  }

  // サーボを初期位置に移動させる
  change_servo_angle(SERVO_BODY, 0, 1);
  change_servo_angle(SERVO_RIGHT_FOOT, 0, 1);
  change_servo_angle(SERVO_LEFT_FOOT, 0, 1);
  change_servo_angle(SERVO_NECK, 0, 1);
}

void change_servo_angle(int servo_id, float angle, int ms) {
  servo_target_angle[servo_id] = angle;
  servo_remaining_count[servo_id] = (int)(ms * ROBOT_FPS / 1000);
  if (servo_remaining_count[servo_id] == 0) {
    servo_remaining_count[servo_id] = 1;
  }
}

void change_servo_state(void) {
  float next_angle;
  float servo_angle;
  int angle_us;
  for (int servo_id = 0; servo_id < SERVO_NUM; servo_id++) {
    if (servo_use[servo_id] && (servo_remaining_count[servo_id] > 0)) {
      digitalWrite(LED_PIN, HIGH);
      next_angle = servo_current_angle[servo_id] + ((servo_target_angle[servo_id] - servo_current_angle[servo_id]) / servo_remaining_count[servo_id]);
      servo_angle = next_angle + servo_trim_angle[servo_id];
      angle_us = int(SERVO_PWM_CENTER + servo_angle * SERVO_PWM_RANGE / 180);
      servo[servo_id].writeMicroseconds(angle_us);
      servo_current_angle[servo_id] = next_angle;
      servo_remaining_count[servo_id]--;
      digitalWrite(LED_PIN, LOW);
    }
  }
}

void receive_command(void) {
  if (Serial.available() > 0) {
    received_length++;
    byte received_data = Serial.read();
    if (received_data == VSIDO_ST) {
      if (received_length > 1) {
        // 2バイト目以降にVSIDO_ST(=0xff)が来た場合
        if ((received_buffer[0] == 0x53) || (received_buffer[0] == 0x54) || (received_buffer[0] == 0x0c) || (received_buffer[0] == 0x0d)) {
          // パススルーコマンドの場合何もしない
        } else {
          // パススルーじゃないのに0xffが来たらおそらく取りそこねになってると思われるので、リセットする
          received_length = 1;
        }
      }
    }
    received_buffer[received_length - 1] = received_data;
    if (received_length >= 3) {
      int length = int(received_buffer[2]);
      if (received_length == length) {
        // 受信完了
        parse_command(received_buffer);
        received_length = 0;
      }
    }
  }
}

void parse_command(byte *buffer) {
  int length = int(buffer[2]);
  if (length >= 4) {
    switch (buffer[1]) {
      case VSIDO_OP_ANGLE: {
        int sid_num;
        int sid;
        float angle;
        sid_num = (length - 5) / 3;
        for (int i = 0; i < sid_num; i++) {
          sid = buffer[4 + i * 3];
          if ((sid > 0) && (sid < VSIDO_SID_MAX) && (servo_use[sid_pin_translation[sid]])) {
            angle = parse_2bytes_data(buffer[5 + i * 3], buffer[6 + i * 3]) / 10.0;
            if (angle < -90) {
              angle = -90;
            }
            if (angle > 90) {
              angle = 90;
            }
            change_servo_angle(sid_pin_translation[sid], angle, buffer[length - 2]);
          }
        }
        send_ack();
        break;
      }
      case VSIDO_OP_GET_VID_VALUE: {
        // VID値取得
        if (buffer[3] == 0xfe) {
          // バージョン情報
          send_vid_version();
        }
        if ((buffer[3] == 0x06) && (buffer[4] == 0x07)) {
          // PWM周期
          send_pwm_period();
        }
        break;
      }
      case VSIDO_OP_EXTRA_COMMAND: {
        int extra_id = int(buffer[3]);
        if (extra_id == VSIDO_EXTRA_ID) {
          int extra_command = int(buffer[4]);
          switch (extra_command) {
            case VSIDO_EXTRA_LED: {
              int led_num;
              led_num = (length - 5) / 4;
              int led_id;
              int led_r;
              int led_g;
              int led_b;
              for (int i = 0; i < led_num; i++) {
                led_id = int(buffer[5 + i * 4]);
                led_r = int(buffer[6 + i * 4]);
                led_g = int(buffer[7 + i * 4]);
                led_b = int(buffer[8 + i * 4]);
                if ((led_id < NEOPIXEL_NUM) && (led_r <= 100) && (led_g <= 100) && (led_b <= 100)) {
                  set_eye_color(led_id, byte(led_r * 2.55), byte(led_g * 2.55), byte(led_b * 2.55));
                }
              }
              break;
            }
            default: {
              break;
            }
          }
        }
        send_ack();
        break;
      }
      default: {
        send_ack();
        break;
      }
    }
  }
}

void send_ack(void) {
  response_buffer[256] = {};
  response_buffer[0] = byte(VSIDO_ST);
  response_buffer[1] = byte(VSIDO_OP_ACK);
  response_buffer[2] = byte(4); // ACKのレスポンス長は4
  response_buffer[3] = make_checksum(response_buffer);
  send_response(response_buffer);
}

void send_vid_version(void) {
  response_buffer[256] = {};
  response_buffer[0] = byte(VSIDO_ST);
  response_buffer[1] = byte(VSIDO_OP_GET_VID_VALUE);
  response_buffer[2] = byte(5); // バージョン情報を返すレスポンス長は5
  response_buffer[3] = byte(0xfe); // バージョン情報はマジックナンバー
  response_buffer[4] = make_checksum(response_buffer);
  send_response(response_buffer);
}

void send_pwm_period(void) {
  response_buffer[256] = {};
  response_buffer[0] = byte(VSIDO_ST);
  response_buffer[1] = byte(VSIDO_OP_GET_VID_VALUE);
  response_buffer[2] = byte(6); // バージョン情報を返すレスポンス長は5
  response_buffer[3] = byte(0x00); // 1024の下位バイト
  response_buffer[3] = byte(0x04); // 1024の上位バイト
  response_buffer[4] = make_checksum(response_buffer);
  send_response(response_buffer);
}

void send_response(byte *buffer) {
  send_response(buffer, false);
}

void send_response(byte *buffer, boolean human_readable) {
  int length = int(buffer[2]);
  if (length >= 4) {
    if (human_readable) {
      for (int i = 0; i < length; i++) {
        Serial.print(int(response_buffer[i]), HEX);
        Serial.print(" ");
      }
      Serial.println("");
    } else {
      Serial.write(buffer, length);
    }
  }
}

byte make_checksum(byte *buffer) {
  int length = int(buffer[2]);
  if (length >= 4) {
    int sum = 0;
    for (int i = 0; i < length - 1; i++) {
      sum ^= int(buffer[i]);
    }
    return byte(sum);
  } else {
    return byte(0);
  }
}

int parse_2bytes_data(byte data1, byte data2) {
  // V-Sidoの2バイトデータ処理戻しロジック
  int code = (data2 & 0b10000000); //符号の保持
  int tmp_data_high = (data2 >> 1) | code; // 符号を保持して右に1bitシフト
  short tmp_data = (((tmp_data_high << 8) | data1) >> 1) | (code << 8); // 2バイトに戻して右に1bitシフト
  return tmp_data;
}

int convert_signed_to_unsigned(int value) {
  unsigned int return_value = value;
  return return_value;
}

