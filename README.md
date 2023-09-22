# haco3dake
haco3 on ichigo dyhook

esp32で動き、lua5.3でプログラミングできる開発環境Haco3を、ichigodyhookで動かすためにhaco3dakeを作っています。
エディタはkiloを改造したものです。

https://twitter.com/oRLF6nXrMai7KiK

基板も作って頒布する予定ですが、回路は結構簡単なのですぐに自作できると思います。

コンポジット映像端子と、PS２キーボードがあれば、ichigodyhookがなくても使えると思います。

ESP-WROOM-32 (D1 Mini)で動くようにplatformIOでチューニングしていますが、ピンアサインさえ合わせれば大体どのようなesp32,M5Stackなどでも動くと思います。

R5,R6は4.7kオーム
R7,R8は120オームです

<img width="666" alt="haco3dake_circuit" src="https://github.com/dentaro/haco3dake/assets/77978725/7ce8ede0-fe56-4fcb-b7c6-ffab06beb675">

<img width="671" alt="haco3dake_image" src="https://github.com/dentaro/haco3dake/assets/77978725/30eccf07-4129-436c-832d-f2368a9155dc">


