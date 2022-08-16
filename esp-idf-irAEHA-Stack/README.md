# External IR Transmiter
M5StackはIRトランスミッターを内蔵していません。   
IRユニットをGROVE-Aポートに取り付ける必要があります。   
非常に安定して動きます。   

![Stack-1](https://user-images.githubusercontent.com/6020549/184788974-11da243f-1018-4bd4-ac8a-3308408df635.JPG)


# How to build

```
git clone https://github.com/nopnop2002/esp-idf-irAEHA
cd esp-idf-irAEHA/esp-idf-irAEHA-Stack
idf.py menuconfig
idf.py flash
```

\*このアプリ特有のメニュー項目はありません。


# How to use

ボタンB / C（中央/右ボタン）を押してIRコードを選択します。   
ボタンBを２秒以上押すと、最後のページを表示します。   
ボタンCを２秒以上押すと、最初のページを表示します。   
ButtonA（左ボタン）を押すとIRコードを発射します。   

![Stack-2](https://user-images.githubusercontent.com/6020549/184789080-6c34dfd8-868b-4301-ba4e-74ab38d0a3c4.JPG)
