# coding:utf-8
'''
ScratchからV-Sidoでロボットを動かすサンプル

Copyright (c) 2015 Daisuke IMAI

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
'''
import os
import sys
import time
import json
import socket
import threading

import tornado.ioloop
import tornado.web
import tornado.websocket

import vsido


class MotionData(object):
    '''
    ロボットのモーションデータに関するデータの保持ならびにやりとりを行う
    '''
    def __init__(self):
        pass

    def read_json(self, json_path):
        ''' jsonの読み込み '''
        json_data = {}
        if not os.path.exists(json_path):
            # jsonが読み込めないので初期化と保存をする
            json_data = {'data_set_name': 'empty', 'data_set': []}
            self.write_json(json_path, json_data)
            return json_data
        else:
            with open(json_path, 'r', encoding='utf-8') as fp:
                try:
                    json_data = json.load(fp)
                except IOError:
                    print('JSON file I/O error')
                    raise
                else:
                    return json_data
                finally:
                    fp.close()

    def write_json(self, json_path, json_data):
        ''' jsonの保存 '''
        with open(json_path, 'w', encoding='utf-8') as fp:
            try:
                json.dump(json_data, fp, indent=2)
            except IOError:
                print('JSON file I/O error')
                raise
            finally:
                fp.close()


#ここからScratchとのソケット通信に関する定義
class ScratchRemoteSensor(object):
    '''
    '''

    SCRATCH_HOST = '127.0.0.1'
    SCRATCH_PORT = 42001

    def __init__(self, sock=None):
        if sock is None:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        else:
            self.sock = sock

        # 受信用のバッファ用意
        self._receive_buffer = b''

    def connect(self, host=None, port=None):
        if host is None:
            host = ScratchRemoteSensor.SCRATCH_HOST
        if port is None:
            port = ScratchRemoteSensor.SCRATCH_PORT
        try:
            self.sock.connect((host, port))
        except:
            raise
        self._connected = True
        self._start_receiver()

    def _start_receiver(self):
        ''' 受信スレッドの立ち上げ '''
        self._receiver_alive = True
        self._receiver_thread = threading.Thread(target=self._receiver)
        self._receiver_thread.setDaemon(True)
        self._receiver_thread.start()

    def _stop_receiver(self):
        ''' 受信スレッドの停止 '''
        self._receiver_alive = False
        self._receiver_thread.join()

    def _receiver(self):
        ''' 受信スレッドの処理 '''
        try:
            while self._receiver_alive:
                data = self.sock.recv(1)
                if len(data) > 0:
                    self._receive_buffer += data
                    #self._receive_buffer.append(data)
                    if len(self._receive_buffer) >= 4:
                        message_len = int.from_bytes(self._receive_buffer[:4], byteorder='big')
                        if len(self._receive_buffer) == 4 + message_len:
                            message = self._receive_buffer[4:].decode('utf-8')
                            self._receive_buffer = b''
                            if message.startswith('broadcast'):
                                print('broadcast:', message.replace('broadcast ', '', 1))
                            if message.startswith('sensor-update'):
                                print('sensor-update:', message.replace('sensor-update ', '', 1))
        except:
            raise

    def send_broadcast(self, message):
        message_string = 'broadcast "' + message + '"'
        self.sock.sendall(len(message_string).to_bytes(4, byteorder='big') + message_string.encode('utf-8'))

    def send_sensor_update(self, name, value):
        message_string = 'sensor-update "' + name + '" ' + str(value)
        print(message_string)
        self.sock.sendall(len(message_string).to_bytes(4, byteorder='big') + message_string.encode('utf-8'))


#ここからTornadeでのWeb/WebSocketサーバーに関する定義
class IndexHandler(tornado.web.RequestHandler):
    '''
    通常のHTTPリクエストで/が求められた時のハンドラ
    '''
    @tornado.web.asynchronous
    def get(self):
        self.render("index.html")


class WebSocketHandler(tornado.websocket.WebSocketHandler):
    '''
    WebSocketで/wsにアクセスが来た時のハンドラ

    on_message -> receive data
    write_message -> send data
    '''

    def open(self):
        self.i = 0
        self.callback = tornado.ioloop.PeriodicCallback(self._send_message, 50)
        self.callback.start()
        print('WebSocket opened')
        self.write_message(json.dumps({'message': 'scratch_command', 'json_data': md.read_json('scratch_command.json')}))

    def check_origin(self, origin):
        ''' アクセス元チェックをしないように上書き '''
        return True

    def on_message(self, message):
        received_data = json.loads(message)
        print('got message:', received_data['command'])
        if received_data['command'] == 'robot_connect':
            print('Connecting to V-Sido CONNECT via', received_data['port'], '...', end='')
            try:
                # V-Sido CONNECTに接続
                vc.connect(received_data['port'])
            except:
                self.write_message(json.dumps({'message': 'robot_cannot_connect'}))
                print('fail')
            else:
                self.write_message(json.dumps({'message': 'robot_connected'}))
                print('done')
        elif received_data['command'] == 'robot_disconnect':
            # V-Sido CONNECTに接続
            vc.disconnect()
            self.write_message(json.dumps({'message': 'robot_disconnected'}))
        elif received_data['command'] == 'set_scratch_command':
            # JSONデータを保存する
            json_data = received_data['json_data']
            md.write_json('scratch_command.json', json_data)
            self.write_message(json.dumps({'message': 'scratch_command', 'json_data': json_data}))

    def _send_message(self):
        pass
#        if len(vsidoconnect.message_buffer) > 0:
#            self.write_message(vsidoconnect.message_buffer.pop(0))

    def on_close(self):
        self.callback.stop()
        print('WebSocket closed')


# アプリケーション割り当て
web_application = tornado.web.Application([
    (r'/', IndexHandler),
    (r'/ws', WebSocketHandler),
    ],
    template_path=os.path.join(os.getcwd(),  'templates'),
    static_path=os.path.join(os.getcwd(),  'assets'),
)


if __name__ == '__main__':
    # V-Sido CONNECTのインスタンス生成
    vc = vsido.Connect()

    # モーションデータのインスタンス生成
    md = MotionData()
    #motion = md.read_json('scratch_command.json')
    #print(motion)
    #md.write_json('scratch_command.json', md.data)

    srs = ScratchRemoteSensor()
    srs.connect()
    srs.send_broadcast('hit')
    srs.send_sensor_update('test', 100)

    # Tornado起動
    print('Starting Web/WebSocket Server...', end='')
    web_application.listen(8888)
    print('done')

    print('Open http://localhost:8888/')
    print('')

    # Tornadoメインループ
    tornado.ioloop.IOLoop.instance().start()
