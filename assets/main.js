var ws = new WebSocket("ws://localhost:8888/ws");

ws.onopen = function() {
};

ws.onmessage = function (evt) {
  // WebSocketでメッセージを受け取った時の処理をまとめて
  try {
    var obj = JSON.parse(evt.data);
    console.log(obj["message"])
    if (obj["message"] == "robot_connected") {
      document.getElementById('serial_port').disabled = true;
      ButtonDisable(document.getElementById('robot_connect'));
      ButtonEnable(document.getElementById('robot_disconnect'));
      alert("ロボットに接続されました。");
    }
    if (obj["message"] == "robot_cannot_connect") {
      alert("ロボットに接続出来ませんでした。シリアルポートの確認をしてください。");
    }
    if (obj["message"] == "robot_disconnected") {
      document.getElementById('serial_port').disabled = false;
      ButtonEnable(document.getElementById('robot_connect'));
      ButtonDisable(document.getElementById('robot_disconnect'));
    }
    if (obj["message"] == "scratch_command") {
      document.getElementById('json_data').value = JSON.stringify(obj["json_data"], null, "  ");
      ButtonDisable(document.getElementById('save_json'));
    }
    if (obj["message"] == "scratch_connected") {
      ButtonDisable(document.getElementById('scratch_connect'));
      ButtonEnable(document.getElementById('scratch_disconnect'));
      alert("Scratchに接続されました");
    }
    if (obj["message"] == "scratch_cannot_connect") {
      alert("Scratchに接続出来ませんでした。Scratchの起動と遠隔センサーの設定の確認をしてください。");
    }
    if (obj["message"] == "scratch_disconnected") {
      ButtonEnable(document.getElementById('scratch_connect'));
      ButtonDisable(document.getElementById('scratch_disconnect'));
    }
  } catch(e) {
  }
};

function OnButtonClick(button) {
  // ボタンが押された時の処理をまとめて
  if (button.id == "robot_connect") {
    if (document.getElementById('serial_port').value == "") {
      alert("シリアルポートを指定してください。");
    } else {
      ws.send(JSON.stringify({command: "robot_connect", port: document.getElementById('serial_port').value}));
    }
  }
  if (button.id == "robot_disconnect") {
    ws.send(JSON.stringify({command: "robot_disconnect"}));
  }
  if (button.id == "save_json") {
    try {
      json_data = JSON.parse(document.getElementById('json_data').value);
      ws.send(JSON.stringify({command: "set_scratch_command", json_data: json_data}));
    } catch(e) {
      alert("JSONの構文が間違っています")
    }
  }
  if (button.id == "scratch_connect") {
    ws.send(JSON.stringify({command: "scratch_connect"}));
  }
  if (button.id == "scratch_disconnect") {
    ws.send(JSON.stringify({command: "scratch_disconnect"}));
  }
  console.log(button.id)
}

function OnJsonChange() {
  ButtonEnable(document.getElementById('save_json'));
}

function ButtonEnable(button) {
  button.disabled = false;
  button.className = "button";
}

function ButtonDisable(button) {
  button.disabled = true;
  button.className = "button_disable";
}
