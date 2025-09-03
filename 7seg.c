/*
Raspberry Pi 7segment LED 制御関数
2025.8 制作 by J.H.
VsCode, Vi
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "gpio.h"
#include "7seg.h"


// 入力された数字(0-10)を LED 上に表示する。
// 0-9: 数字
// 10 : ピリオドのみ。
// 20-29 : 数字とピリオド両方を表示する。
void LightNum(int num) {
    int period;
    //引数により数字を調整する。
    if (num <= 9 && num >= 0) {
        period = 0;
    }
    else if (num >= 20 && num <= 29) {
        period = 1;
        num -= 20;
    }
    else if (num == 10) {
        period = 1;
    }
    else {
        return;
    }

    //ledpattern から二進数で表現された LED 発行パターンを読み込む。
    unsigned char ledpt = ledpattern(num);
    if (period) ledpt = ledpt | 0x80;

    //ledpt で示されたビットにより、それぞれのLEDを光らせる。
    Light7Seg(ledpt);
}

/**
 * @brief 7segment LED の各セグメントを点灯/消灯する。
 * @param l 7segment LED の表示パターンを二進数8bitで表す char 値。
 *          最下位ビットが a セグメント、次が b セグメント、...、最上位ビットが小数点セグメントを表す。
 *          なお 0 が表示を意味するため反転して渡す。すなわちbit 0なら非表示、1なら表示となる。 
 */
void Light7Seg(char l) {
    for (int i = 0; i < LEDS; i++) {
        //最下位ビットから取り出していく。
        int value = l & 0x01;
        l = l >> 1;
        //取り出した LED-On/Off フラグを GPIO valueに送り点灯/消灯させる
        write_gpio_value(getGpID(i), value + '0');
    }
}

/**
 * @brief 7segment LED の全セグメントを消灯する
 */
void Off7Seg(void) {
    Light7Seg(0xFF);
}

/**
* @brief 7segment LED の表示パターンを二進数8bitで表す char 値を返す。
* @param num 表現したい数字。0-9:数字 10:小数点部
* @return LED表示パターンを表す char 値。なお 0 が表示を意味するため反転して返す。すなわちbit 0なら非表示、1なら表示となる。
*/
unsigned char ledpattern(char num) {
    unsigned char led[] = {
    0x3F, // 0b00111111  - 数字 0
    0x06, // 0b00000110  - 数字 1
    0x5B, // 0b01011011  - 数字 2
    0x4F, // 0b01001111  - 数字 3
    0x66, // 0b01100110  - 数字 4
    0x6D, // 0b01101101  - 数字 5
    0x7D, // 0b01111101  - 数字 6
    0x27, // 0b00100111  - 数字 7
    0x7F, // 0b01111111  - 数字 8
    0x6F, // 0b01101111  - 数字 9
    0x80  // 0b10000000  - 小数点 (P)
    };
    return led[num] ^ 0xFF; //反転させて返す。なぜなら 0 が表示を意味するから。
}


