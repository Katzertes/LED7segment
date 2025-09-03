/*
2025/8/29 訓練校における追加課題 by J.H.

訓練校要件
■任意の場所に新しく「input_num」というディレクトリを作成し、その中に以下の要件を満たすプログラムを作成
・ユーザーが入力した値を２秒間出力
・0~9の値が入力されている間は「数値入力→２秒間点灯」のサイクルを繰り返す
・0~9以外の値が入力された場合、「finish.」と出力し、プログラムを終了する
・エクスポートやアンエクスポート等の事前処理と事後処理はプログラム化してもしなくてもよい
実行例)
>>>5 (７セグメントLEDで"5"を2秒間点灯ののち消灯)
>>>2 (７セグメントLEDで"2"を2秒間点灯ののち消灯)
>>>10 (点灯なし)
finish.
*/

#include <stdio.h>
#include <unistd.h>
#include "gpio.h"
#include "7seg.h"
#include "time.h"

int main(void) {
    int num;
    int cont = 0;
    do {
        printf("表示したい数(0-9)を入れてください。それ以外は終了 :");
        scanf("%d", &num);
        if (num >= 0 && num <= 9) {
            cont = 1;
            LightNum(num);
            usleep(2000000);
            // Off7Seg();
        }
        else {
            printf("You input %d\n", num);
            cont = 0;
        }
    } while (cont);
    Off7Seg();

    // 基本課題のカウントダウン、9⇒0までのLED表示をする。
/*
    for (int i = 9; i >= 0; i--) {
        //iを表示する関数の呼び出し。
        LightNum(i);
        //一秒待つ。
        usleep(1000000);
    }
*/
    printf("finish.\n=== the Demonstration for the Exercise on 2025.8/29 Completed.\n===");
    return 0;
}


