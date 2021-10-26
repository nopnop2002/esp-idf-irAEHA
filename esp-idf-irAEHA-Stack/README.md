# External IR Transmiter
M5StackはIRトランスミッターを内蔵していません。
IRユニットをGROVE-Bポートに取り付ける必要があります。
非常に安定して動きます。

![Stack-1](https://user-images.githubusercontent.com/6020549/59958246-574faa00-94de-11e9-95f1-24871f8c8f20.JPG)

---

# How to build

```
git clone https://github.com/nopnop2002/esp-idf-irAEHA
cd esp-idf-irAEHA/esp-idf-irAEHA-Stack
idf.py menuconfig
idf.py flash
```

\*このアプリ特有のメニュー項目はありません。

--- 

# How to use

ボタンB / C（中央/右ボタン）を押してIRコードを選択します。
ButtonA（左ボタン）を押してIRコードを発射します。

![Stack](https://user-images.githubusercontent.com/6020549/60800362-0b745480-a1b0-11e9-82df-4bb29542a463.JPG)

