# What's pocketStrSynth?
This is a small-size synthesizer using an Arduino.
The original idea is from [dspsynth.eu by Jan Ostman](https://web.archive.org/web/20180817050935/http://blog.dspsynth.eu/how-to-build-a-string-synth/).
Now this website had already closed, but there is [the related article in Hackaday.com wrote by Elliot Williams](https://hackaday.com/2016/02/23/a-slew-of-open-source-synthesizers/), we can also refer this.  
  
Arduinoを使った小型のシンセサイザーです。  
音源部は[dspsynth.eu](https://web.archive.org/web/20180817050935/http://blog.dspsynth.eu/how-to-build-a-string-synth/)というサイトのソースコードを流用しています。  
現在このWebサイトは閉鎖していますが、[Hackaday.com](https://hackaday.com/2016/02/23/a-slew-of-open-source-synthesizers/)に同じコードを用いて別の方が執筆した記事がありますので、よろしければそちらもご参考ください。  
  
![Appearance](https://github.com/moppii-hub/pocketStrSynth/blob/master/others/appearance.jpg?raw=true)   


# リポジトリの内容
このリポジトリには以下の内容が含まれます。
 - Arduinoプログラムのソースコード
 - 外装設計データ
 - 基板設計データ（回路図、KiCadプロジェクト一式、及び基板発注に使うGerberデータ）
 - その他（写真・演奏例）


# 機能説明
このシンセサイザーは以下の機能を備えています。
 - 8個の鍵盤…演奏に使用します。6キー以上を同時押しし続けると設定モード/演奏モードを切り替えます。
 - 5つのDIPスイッチ…音階が変わります。（キーマトリクスの回路接続の変更スイッチです）
 - 3つのツマミ…それぞれ以下の機能があります。
   - 左　：アタック/リリースタイムの調節（設定モード時は、8つのキーの音階範囲が変わります）
   - 中央：デチューン量の調節（設定モード時は、スケールを変更します。メジャーとペンタの2種）
   - 右　：オクターブシフト４段階（設定モード時は、調声が変わります。Cから+15段階。）
 - ボタン電池（CR2032 3V）2個で動作します。


# 注意点
 - 基板上のコネクタ（ピンヘッダ）は、拡張または実験用です。演奏には直接必要はありません。
 - 電源スイッチON状態でUSBケーブルを接続すると、電池側またはUSB側に逆電圧が掛かります。


# その他写真
![howto](https://github.com/moppii-hub/pocketStrSynth/blob/master/others/how_to_use.jpg?raw=true)   
![inside](https://github.com/moppii-hub/pocketStrSynth/blob/master/others/inside.jpg?raw=true)   

