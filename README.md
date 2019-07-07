# esp-idf-irAEHA
M5Stick and M5StickC as a remote control transmitter.   
This is AEHA IR Format exclusive use.   
AEHA IR Format is used only in Japan.   

ESP-IDFには[こちら](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/rmt_nec_tx_rx)に赤外線送受信のサンプルが付属していますが、NECフォーマットしか対応していません。   

我が家のテレビは家製協(AEHA)フォーマットなので、家製協(AEHA)フォーマットに対応したアプリケーションを作りました。   
M5Stick M5StickCをリモコンの送信機として使うことができます。   

---

# リモコンコード解析
リモコンコードの解析には[rmt_aeha_rx](https://github.com/nopnop2002/esp-idf-irAEHA/tree/master/rmt_aeha_rx)を使います。   

```
git clone https://github.com/nopnop2002/esp-idf-irAEHA
cd esp-idf-irAEHA/rmt_aeha_rx
make menuconfig
make flash monitor
```

赤外線受信モジュールをGPIO19に接続します。
赤外線送信機は使用しません。
GPIO19を変更したい場合は、以下を変更することで任意のGPIOを使用することができます。

```
#define RMT_RX_GPIO_NUM  19     /*!< GPIO number for receiver */
```


ファームをビルドしてボードに書き込んだら、モニターツールを実行してシリアル出力を表示します。
リモコンのボタンを押すと、受信した赤外線コードが表示されます。
こちらはテレビの電源ボタンを押したときの表示です。   

```
I (23850) AEHA: RMT RCV --- customer: 0x2002 parity: 0x00 index: 4
I (23850) AEHA: RMT RCV --- data[0]: 0x08
I (23860) AEHA: RMT RCV --- data[1]: 0x00
I (23860) AEHA: RMT RCV --- data[2]: 0x3d
I (23870) AEHA: RMT RCV --- data[3]: 0xbd
```

# 赤外線コードの登録
このようにして解析した赤外線コードをfontディレクトリのDisplay.defに登録します。   
Display.defの書式は以下の通りです。   

```
表示するテキスト,customerコード,parityコード,dataコードの数,data0,data1,data2......;
```

*AEHAフォーマットのパリティコードは、本来、カスタマーコードを4ビット単位でXORをとったものですが、
調べてみるとパリティではなく、普通の1番目のデータとして使っているリモコンもあるようです。   
そこで、送信時のパリティコードはライブラリ内部で自動計算せずに、外部から与えるようにしています。



```
#This is define file for isp-idf-irSend
#Text,cmd,addr;
Power,0x2002,0x00,4,0x08,0x00,0x3d,0xbd;	TV on/off
Ch 1,0x2002,0x00,4,0x08,0x09,0x40,0xc9;		Channel 1
Ch 2,0x2002,0x00,4,0x08,0x09,0x41,0xc8;		Channel 2
Ch 3,0x2002,0x00,4,0x08,0x09,0x42,0xcb;		Channel 3
Ch 4,0x2002,0x00,4,0x08,0x09,0x43,0xca;		Channel 4
Ch 5,0x2002,0x00,4,0x08,0x09,0x44,0xcd;		Channel 5
Ch 6,0x2002,0x00,4,0x08,0x09,0x45,0xcc;		Channel 6
```


