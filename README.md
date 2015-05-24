# ScratchVSido
## これは何？
[アスラテック株式会社](http://www.asratec.co.jp/ "アスラテック株式会社")のロボット制御マイコンボード「[V-Sido CONNECT RC](http://www.asratec.co.jp/product/connect/rc/ "V-Sido CONNECT RC")」をScratchからコントロールするためのRemote Sensor ConnectionのRubyによるサンプルコードです。  
[V-Sido Developerサイトの技術資料](https://v-sido-developer.com/learning/connect/connect-rc/ "V-Sido Developerサイトの技術資料")に公開されている情報を元に、個人が作成したもので、アスラテック社公式のツールではありません。  
Scratch1.4からRemote Sensor Protocolを経由し、シリアル接続からV-Sido CONNECTをコントロールすることができます。

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

## 免責事項
一応。  

このサンプルコードを利用して発生したいかなる損害についても、アスラテック株式会社ならびに今井大介は責任を負いません。自己責任での利用をお願いします。
