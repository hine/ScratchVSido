# ScratchVSido
## これは何？
[アスラテック株式会社](http://www.asratec.co.jp/ "アスラテック株式会社")のロボット制御マイコンボード「[V-Sido CONNECT RC](http://www.asratec.co.jp/product/connect/rc/ "V-Sido CONNECT RC")」をScratchからコントロールするためのRemote Sensor ConnectionのPython3によるサンプルコードです。  
[V-Sido Developerサイトの技術資料](https://v-sido-developer.com/learning/connect/connect-rc/ "V-Sido Developerサイトの技術資料")に公開されている情報を元に、個人が作成したもので、アスラテック社公式のツールではありません。  
Scratch1.4からRemote Sensor Protocolを経由し、シリアル接続からV-Sido CONNECTをコントロールすることができます。

元は[Ruby版](https://github.com/hine/ScratchVSido/tree/master "Ruby版")で書いていましたが、現在は全く仕様を変えてこのPython版のみメンテナンスしています。

## 誰が作ったの？
アスラテック株式会社に勤務する今井大介(Daisuke IMAI)が個人として作成しました。

## どうして作ったの？
だってScratchからロボットが動かせたら楽しいじゃないですか！  

## 動作環境
Windows、OS X、Ubuntuなど上のScratch1.4ならびにPython3で動作するのではないかと思います。  
動作確認済み環境は、  
* OS X 10.11.1(ElCapitan) + Python3.4.3
* Windows8.1/10 + Python3.4.3/3.5

です。  

まだ未確認ですが、RaspberryPiでも動かせるのではないかと思います。  

Scratchとの接続に自作の[pyscratchライブラリ](https://github.com/hine/pyscratch "pyscratchライブラリ")を、V-Sido CONNECT RCとの接続に同じく自作の[pyvsidoライブラリ](https://github.com/hine/pyvsido "pyvsidoライブラリ")を利用しています。  

JSONのビジュアル表示にDavid Durmanさんの[FlexiJsonEditor](https://github.com/DavidDurman/FlexiJsonEditor "FlexiJsonEditor")を利用しています。  

## 使い方
後日、まとめます。  

## Scratchでの使い方
後日、まとめます。  

## 免責事項
一応。  

このサンプルコードを利用して発生したいかなる損害についても、アスラテック株式会社ならびに今井大介は責任を負いません。自己責任での利用をお願いします。
