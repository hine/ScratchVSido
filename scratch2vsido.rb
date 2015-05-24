# -*- coding: utf-8 -*-
require 'rubygems'

# シリアルライブラリの読み込み
require 'rubyserial'
# gem install rubyserial

# ScratchのRemote Sensor Connectionsプロトコルのライブラリ読み込み
require_relative "scratchrsc"
# http://scratchforums.blob8108.net/forums/viewtopic.php?id=113658

# シリアル等の外部とのデータのやりとりについて1バイトで行う
Encoding.default_external = 'ASCII-8BIT'

# シリアルポートの設定（引数から）
if ARGV[0].nil?
  serial_port = "/dev/tty.SBDBT-0009dd40dc14-SPP"
else
  serial_port = ARGV[0]
end

# Scratch hostの設定（引数）
if ARGV[1].nil?
  scratch_host = "127.0.0.1"
else
  scratch_host = ARGV[1]
end

# Scratch Remote Connectionのための具体クラス
class VsidoRSC < RSCWatcher

  # V-Sidoコマンド
  COMMAND_ST = 0xff
  COMMAND_OP_ANGLE = 0x6f
  COMMAND_OP_IK = 0x6b
  COMMAND_OP_WALK = 0x74

  # シリアル接続のボーレートを設定
  SERIAL_BAUTRATE = 115200

  # ブロードキャストメッセージ定義
  BROADCAST = [
    "reset_angles",
    "set_angle",
    "execute_angles",
    "reset_iks",
    "set_ik",
    "execute_iks",
    "walk",
    "stop"
  ]

  # センサー定義
  SENSOR = [
    "sid",
    "angle",
    "time",
    "kid",
    "ikx",
    "iky",
    "ikz",
    "speed",
    "turn"
  ]

  # 初期化の引数はシリアルポートとホストのIPアドレスを指定
  def initialize(serial_port, scratch_host)

    # シリアル接続
    print "Opening serial port..."
    begin
      @sp = Serial.new(serial_port, SERIAL_BAUTRATE)
    rescue => e
      STDERR.puts "Cannot open serial port."
      STDERR.puts e.to_s
      exit 1
    end
    puts "opened."

    # シリアル受信のためのスレッドを立ち上げる
    Thread.new do
      buffer = []
      loop do
        recieve = @sp.getbyte
        if !recieve.nil?
          if recieve.to_s(16) == "ff" # Todo: データ中にffがある場合も初期化されてしまう
            buffer = []
          end
          buffer.push(recieve.to_s(16))
          if buffer.length > 3
            if buffer.length == buffer[2].to_i
              print "V-Sido: < ", buffer.join(" "), "\n"
            end
          end
        end
      end
    end

    # Scratchに接続
    print "Connecting Scratch host..."
    begin
      # 元のライブラリのinitializeメソッドの呼び出し
      super(scratch_host)
    rescue
      STDERR.puts "cannot connect Scratch host."
      STDERR.puts e.to_s
      @sp.close
      exit 1
    end
    puts "connected."

    # インスタンス変数初期化（センサーと一致）
    @sid = 0
    @angle = 0
    @time = 2
    @angle_data = Hash.new()
    @kid = 0
    @ikx = 0
    @iky = 0
    @ikz = 0
    @ik_data = Hash.new()
    @speed = 0
    @turn = 0

    # Scratchに対応ブロードキャストを伝えるために一度ブロードキャストを送信
    BROADCAST.each do |command|
      broadcast command
    end
  end

  # センサー値(グローバル変数)の変更があった場合の処理
  def on_sensor_update(name, value) # when a variable or sensor is updated
    if SENSOR.include? name
      value = value.to_i
      if name == "sid"
        if (1 .. 254).include? value
          @sid = value
        end
      elsif name == "angle"
        if value < -145
          @angle = -145
        elsif value > 145
          @angle = 145
        else
          @angle = value
        end
      elsif name == "time"
        if value > 100
          @time = 100
        elsif value < 0
          @time = 0
        else
          @time = value
        end
      elsif name == "kid"
        if (0 .. 4).include?(value)
          @kid = value
        end
      elsif name == "ikx"
        if value < -100
          @ikx = -100
        elsif value > 100
          @ikx = 100
        else
          @ikx = value
        end
      elsif name == "iky"
        if value < -100
          @iky = -100
        elsif value > 100
          @iky = 100
        else
          @iky = value
        end
      elsif name == "ikz"
        if value < -100
          @ikz = -100
        elsif value > 100
          @ikz = 100
        else
          @ikz = value
        end
      elsif name == "speed"
        if value < -100
          @speed = -100
        elsif value > 100
          @speed = 100
        else
          @speed = value
        end
      elsif name == "turn"
        if value < -100
          @turn = -100
        elsif value > 100
          @turn = 100
        else
          @turn = value
        end
      end
      puts "S2V: #{name} assigned #{value}"
    elsif
      "Scratch: #{name} assigned #{value}"
    end
  end

  # 以下ブロードキャストメッセージに対する処理
  # ブロードキャスト: reset_angles
  def broadcast_reset_angles
    @angle_data.clear
    puts "S2V: Reseted angles."
  end

  # ブロードキャスト: set_angle
  def broadcast_set_angle
    @angle_data[@sid] = @angle;
    puts "S2V: Set angle #{@sid}:#{@angle}"
  end

  # ブロードキャスト: execute_angles
  def broadcast_execute_angles
    _send_command(_make_multi_servo_angle_command(@angle_data, @time))
    @angle_data.clear
  end

  # ブロードキャスト: reset_iks
  def broadcast_reset_iks
    @ik_data.clear
    puts "S2V: Reseted IKs."
  end

  # ブロードキャスト: set_ik
  def broadcast_set_ik
    @ik_data[@kid] = {x: @ikx, y: @iky, z: @ikz};
    puts "S2V: Set IK #{@kid}:#{@ikx},#{@iky},#{@ikz}"
  end

  # ブロードキャスト: execute_iks
  def broadcast_execute_iks
    _send_command(_make_multi_ik_command(@ik_data))
    @ik_data.clear
  end

  # ブロードキャスト: walk
  def broadcast_walk
    _send_command(_make_walk_command(@speed, @turn))
  end

  # ブロードキャスト: stop
  def broadcast_stop
    # 未実装
  end

  # ブロードキャストメッセージが来た場合の処理
  def on_broadcast(name)
    # 本来はライブラリの__on_broadcastで行う処理だが、通ってないみたいなのでここで処理
    method = "broadcast_#{name}"
    if self.respond_to? method
      self.send method
    else
      puts "Scratch: broadcast #{name}"
    end
  end

  private

  # V-Sidoのサーボの角度変更コマンド生成
  def _make_multi_servo_angle_command(angle_data, time)
    command_data = []
    command_data.push COMMAND_ST
    command_data.push COMMAND_OP_ANGLE
    command_data.push 0x00 #LN
    command_data.push time #CYCここでは固定値
    angle_data.each{|sid, angle|
      command_data.push sid.to_i

      # 角度は2バイトなので、ビットシフト処理などを行う(コマンドリファレンス参照)
      deg = (angle * 10).round
      command_data.push (deg << 1) & 0x00ff #DEG_L
      command_data.push (((deg << 1) >> 8) << 1) & 0x00ff #DEG_H
    }
    command_data.push 0x00 #sum

    return _adjust_ln_sum(command_data)
  end

  # V-SidoのIKコマンド生成
  def _make_multi_ik_command(ik_data)
    command_data = []
    command_data.push COMMAND_ST
    command_data.push COMMAND_OP_IK
    command_data.push 0x00 #LN
    command_data.push 0x01 #IKF
    ik_data.each{|kid, data|
      command_data.push kid.to_i

      command_data.push data[:x] + 100 #KDT_X
      command_data.push data[:y] + 100 #KDT_X
      command_data.push data[:z] + 100 #KDT_X
    }
    command_data.push 0x00 #sum

    return _adjust_ln_sum(command_data)
  end

  # V-Sidoの歩行コマンド生成
  def _make_walk_command(speed, turn)
    command_data = []
    command_data.push COMMAND_ST
    command_data.push COMMAND_OP_WALK
    command_data.push 0x00 #LN
    command_data.push 0x00 #WAD(Utilityでは0で固定)
    command_data.push 0x02 #WLN(現在2で固定)
    command_data.push speed + 100 #
    command_data.push turn + 100 #
    command_data.push 0x00 #sum

    return _adjust_ln_sum(command_data)
  end

  # V-SidoのコマンドデータのLNとSUMを調整する
  def _adjust_ln_sum(command_data)
    if command_data.length > 3
      command_data[2] = command_data.length #このプログラムではLNの位置は2で固定(※パススルーコマンドだと1)
    end

    sum = 0;
    for data in command_data
      sum ^= data
    end
    command_data[command_data.length - 1] = sum

    return command_data
  end

  # V-Sidoのコマンド送信（データの内容チェックなし）
  def _send_command(command_data)
    data_str = ""
    data_array =[]
    for data in command_data
      data_str << data.chr
      data_array.push data.to_s(16)
    end
    @sp.write data_str
    print "V-Sido: > ", data_array.join(" "), "\n"
  end
end

# 初期化
s2v = VsidoRSC.new(serial_port, scratch_host)
s2v.sensor_update "Scratch connected", "1"

#メインループ
loop do
  # データを受け取り処理する
  s2v.handle_command
end
