# esp-idf-irAEHA
M5Stick and M5StickC as a remote control transmitter.   
This is AEHA IR Format exclusive use.   
AEHA IR Format is used only in Japan.   

ESP-IDFには[こちら](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/rmt/ir_nec_transceiver)に赤外線送受信のサンプルが付属していますが、
NECフォーマットしか対応していません。   

我が家のテレビは家製協(AEHA)フォーマットなので、家製協(AEHA)フォーマットに対応したアプリケーションを作りました。   
M5Stick M5StickC(+)をリモコンの送信機として使うことができます。   


# Software requirements
ESP-IDF V4.4.   
ESP-IDF V5 では RMT ドライバーの仕様が大幅に変更されました。    
ESP-IDF V5 では、ビルド時に以下の警告が表示されますが、機能します。   
```
#warning "The legacy RMT driver is deprecated, please use driver/rmt_tx.h and/or driver/rmt_rx.h"
```


# リモコンコード解析
リモコンコードの解析には esp-idf-irAEHA-analysis を使います。   

```
git clone https://github.com/nopnop2002/esp-idf-irAEHA
cd esp-idf-irAEHA/esp-idf-irAEHA-analysis
idf.py menuconfig
idf.py flash monitor
```

赤外線受信モジュールをGPIO34に接続します。
赤外線送信機は使用しません。
GPIO34を変更したい場合は、menuconfig で任意のGPIOに変更することができます。

![ir_recv](https://github.com/nopnop2002/esp-idf-irAEHA/assets/6020549/4833ac0a-f042-4157-ab2d-03b1ba2c8a58)

ファームをビルドしてボードに書き込んだら、モニターツールを実行してシリアル出力を表示します。   
リモコンのボタンを押すと、受信した赤外線コードが表示されます。   
こちらはテレビの電源ボタンを押したときの表示です。   
なぜか、３回同じコードが発射されていることが分かります。   

```
I (6770) AEHA: Scan success
I (6770) AEHA: Scan Data[0]: 0x02
I (6770) AEHA: Scan Data[1]: 0x20
I (6770) AEHA: Scan Data[2]: 0x80
I (6780) AEHA: Scan Data[3]: 0x00
I (6780) AEHA: Scan Data[4]: 0x3d
I (6780) AEHA: Scan Data[5]: 0xbd
I (6900) AEHA: Scan success
I (6910) AEHA: Scan Data[0]: 0x02
I (6910) AEHA: Scan Data[1]: 0x20
I (6910) AEHA: Scan Data[2]: 0x80
I (6910) AEHA: Scan Data[3]: 0x00
I (6910) AEHA: Scan Data[4]: 0x3d
I (6920) AEHA: Scan Data[5]: 0xbd
I (7040) AEHA: Scan success
I (7040) AEHA: Scan Data[0]: 0x02
I (7040) AEHA: Scan Data[1]: 0x20
I (7040) AEHA: Scan Data[2]: 0x80
I (7040) AEHA: Scan Data[3]: 0x00
I (7050) AEHA: Scan Data[4]: 0x3d
I (7050) AEHA: Scan Data[5]: 0xbd
```

# 赤外線コードの登録
このようにして解析した赤外線コードをfontディレクトリのDisplay.defに登録します。   
Display.defの書式は以下の通りです。   
行の終端はセミコロンです。   

```
表示するテキスト,発射する回数,data0,data1,data2......;Comment
```

data0とdata1はCustomer Codeと呼ばれています。   
data2はParity Codeと呼ばれています。   
Parity Codeは、本来、Customer Codeを4ビット単位でXORを取った値ですが、
調べてみるとそのルールには従わず、普通の1番目のデータとして使っているリモコンもあるようです。   
そこで、Parity Codeはライブラリ内部で自動計算せずに、外部から与えるようにしています。



```
#This is define file for isp-idf-irAEHA
#Text,number of fire,data0,data1,data2.....;
TV Power ON/OFF,3,0x02,0x20,0x80,0x00,0x3d,0xbd; TV on/off
TV Channel 1,3,0x02,0x20,0x80,0x09,0x40,0xc9; Channel 1
TV Channel 2,3,0x02,0x20,0x80,0x09,0x41,0xc8; Channel 2
TV Channel 3,3,0x02,0x20,0x80,0x09,0x42,0xcb; Channel 3
TV Channel 4,3,0x02,0x20,0x80,0x09,0x43,0xca; Channel 4
TV Channel 5,3,0x02,0x20,0x80,0x09,0x44,0xcd; Channel 5
TV Channel 6,3,0x02,0x20,0x80,0x09,0x45,0xcc; Channel 6
TV Channel 7,3,0x02,0x20,0x80,0x09,0x46,0xcf; Channel 7
TV Channel 8,3,0x02,0x20,0x80,0x09,0x47,0xce; Channel 8
TV Channel 9,3,0x02,0x20,0x80,0x09,0x48,0xc1; Channel 9
TV Channel 10,3,0x02,0x20,0x80,0x09,0x49,0xc0; Channel 10
TV Channel 11,3,0x02,0x20,0x80,0x09,0x4a,0xc3; Channel 11
TV Channel 12,3,0x02,0x20,0x80,0x09,0x4b,0xc2; Channel 12
```


# 家製協(AEHA)フォーマット

![ir_aeha](https://user-images.githubusercontent.com/6020549/184791479-c666a21d-5a3e-407b-9dec-a9c9e0df34ac.png)
