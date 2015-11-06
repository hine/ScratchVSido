# ScratchVSido
## これは何？
[アスラテック株式会社](http://www.asratec.co.jp/ "アスラテック株式会社")のロボット制御マイコンボード「[V-Sido CONNECT RC](http://www.asratec.co.jp/product/connect/rc/ "V-Sido CONNECT RC")」をScratchからコントロールするためのRemote Sensor ConnectionのRubyによるサンプルコードです。  
[V-Sido Developerサイトの技術資料](https://v-sido-developer.com/learning/connect/connect-rc/ "V-Sido Developerサイトの技術資料")に公開されている情報を元に、個人が作成したもので、アスラテック社公式のツールではありません。  
Scratch1.4からRemote Sensor Protocolを経由し、シリアル接続からV-Sido CONNECTをコントロールすることができます。
  
なお、現在は[Python版](https://github.com/hine/ScratchVSido/tree/python "Python版")を主にメンテナンスしています。Python版はこのRuby版とは違う使い方で、Web画面からScratchからのコマンドを作れるように指定ます。

## 誰が作ったの？
アスラテック株式会社に勤務する今井大介(Daisuke IMAI)が個人として作成しました。

## どうして作ったの？
だってScratchからロボットが動かせたら楽しいじゃないですか！  

## 動作環境
Windows、OS X、Ubuntuなど上のScratch1.4ならびにRubyで動作するのではないかと思います。  
動作確認済み環境は、  
* OS X 10.10.3(Yosemite) + Ruby 2.1.5p273  

Scratchとの接続に、[logiblocs](https://scratch.mit.edu/users/logiblocs/ "logiblocs")氏のコードを利用しています。  
[ここ](http://scratchforums.blob8108.net/forums/viewtopic.php?id=113658 "Sample Code")から、scratchrsc.rbを使わせていただいています。

## 使い方
まずScratch1.4を立ち上げてから、同じマシンで、  
$ ruby scratch2vsido.rb [シリアルポートデバイス] [Scratchが起動しているマシンのIPアドレス]  
で起動してください。  
通常は、IPアドレスは省略可能です。

## Scratchでの使い方
[Scratchのサンプル](https://scratch.mit.edu/projects/63929748/#editor "Scratch Sample")を参照してください。  

* サーボ角度のコントロール  
まずはreset_anglesのメッセージを送り、その後、変数sidにサーボモーターのIDを、変数angleに目的角度を設定、変数timeに変化にかかる時間を設定。
その後set_angleメッセージを送ります。すぐにsidなどを変更すると誤動作することがあるので、0秒の待つを入れてください。
まとめて変化させたいサーボを全て登録し、その後、execute_anglesのメッセージを送れば実際にサーボモーターに命令が送られます。  

* IK(Inverse kinematicsによる逆運動制御)のコントロール  
まずはreset_iksのメッセージを送り、その後、変数kidにIK制御のIDを、変数ikx、iky、ikzに目的位置を設定。
その後set_ikメッセージを送ります。すぐにkidなどを変更すると誤動作することがあるので、0秒の待つを入れてください。
まとめて変化させたいIK制御を全て登録し、その後、execute_iksのメッセージを送れば実際にサーボモーターに命令が送られます。  

* 歩行のコントロール  
変数speedに前後方向の移動量を、変数turnに左右の旋回量を設定。
その後walkメッセージを送れば実際にサーボモーターに命令が送られます。  

詳しくは[V-Sido Developerサイトの技術資料](https://v-sido-developer.com/learning/connect/connect-rc/ "V-Sido Developerサイトの技術資料")を見てください。

## 免責事項
一応。  

このサンプルコードを利用して発生したいかなる損害についても、アスラテック株式会社ならびに今井大介は責任を負いません。自己責任での利用をお願いします。
