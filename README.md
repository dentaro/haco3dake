# haco3dake
haco3 on ichigo dyhook

esp32で動き、lua5.3でプログラミングできる開発環境Haco3を、ichigodyhookで動かすためにhaco3dakeを作っています。

エディタはkiloを改造したものです。

ichigo dyhookの本来の使用法からは外れますので、非破壊ですが、改造目的での自己責任でお願いします。

https://twitter.com/oRLF6nXrMai7KiK

基板も作って頒布する予定ですが、回路は結構簡単なのですぐに自作できると思います。

ichigodyhook側の３．３Vでもなぜか動きますが、WiFiやESP‐NOWなどの通信時は5Vをesp32側から入れないと安定しないようです。

コンポジット映像端子と、PS２キーボードがあれば、ichigodyhookがなくても使えると思います。

ESP-WROOM-32 (D1 Mini)で動くようにplatformIOでメモリ使用量をチューニングしています。

回路図はwroverですが、ピンアサインさえ合わせれば大体どのようなesp32,M5Stackなどでも動くと思います。

R5,R6は4.7kオーム
R7,R8は120オームです

<img width="666" alt="haco3dake_circuit" src="https://github.com/dentaro/haco3dake/assets/77978725/7ce8ede0-fe56-4fcb-b7c6-ffab06beb675">

<img width="671" alt="haco3dake_image" src="https://github.com/dentaro/haco3dake/assets/77978725/30eccf07-4129-436c-832d-f2368a9155dc">


