/*
訓練校 IoT 課題：Raspberry Pi の GPIO を利用して 7セグメント LED に数字をカウントダウンするプログラムを作成せよ。

参考 : Raspberry Pi の GPIO 利用手順
①exprot : GPIO18を使用可能にする
echo 18 > /sys/class/gpio/export
➁direction : GPIO18を出力モードにする
echo out > /sys/class/gpio/gpio18/direction

③LED点灯
value : echo 1 > /sys/class/gpio/gpio18/value
④LED消灯
value : echo 0 > /sys/class/gpio/gpio18/value

⑤GPIO18の使用を終了する
unexport : echo 18 > /sys/class/gpio/unexport
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "gpio.h"

#define LEDS 8

void LightNum(int num);
unsigned char ledpattern(char num);
int getGpID(char num);
void write_gpio_value(int gpio_pin, char value);
char* get_gpio_value_path(int gpio_pin);
void unexport_all(void);
void unexport(int gpio_pin);
void standby_all(void);
void standby(int gpio_pin);
void unexportGpio(int gpio_pin);

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
    char l = ledpt;
    for (int i = 0; i < LEDS; i++) {
        //最下位ビットから取り出していく。
        int value = l & 0x01;
        l = l >> 1;

        //取り出した value 値を送る。
        write_gpio_value(getGpID(i), value + '0');
    }
}

// LightNum のコンソール表示によるシミュレーション (作りかけ)
/*
void PrintNum(int num) {
    int period;
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
    unsigned char ledpt = ledpattern(num);
    if (period) ledpt = ledpt | 0x80;
    char l = ledpt;
    char digitchr[3][4] = {
        {'|','~','|',' '},
        {'|','-','|',' '},
        {'|','~','|','.'}
    };
    char digitflag[3][4] = {
        {7,0,1,10},
        {6,8,2,10},
        {5,4,3,9}
    };
    char digitput[3][4] = { 0 };
    for (int i = 0; i < LEDS; i++) {
        //最下位ビットから取り出していく。
        int value = l & 0x01;
        l = l >> 1;

        //取り出した value 値を送る。
        for (int j = 0; j < 3; j++) {
            digitput[j][k] = 0;
            for (int k = 0; k < 4; k++) {
                if (digitflag[j][k] == i) {
                    digitput[j][k] = value;
                }
            }
        }
    }
    //表示する。
    printf("\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            if (digitput[i][j]) {}
        }
    }
}
*/

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

/*
num = 0-7 ならダイレクトに 0-7 を示し、
num = ABCDEFPabcdefp であればそれを 0-7 に変換し、対応するGPIOの数値を返す。
あってはならない数なら -1 を返す。
*/
int getGpID(char num) {
    int gpios[LEDS] = {5,6,22,27,17,25,24,23};
    int gp = 0;
    if (num >= 0 && num <= 7) {
        gp = num;
    }
    else {
        num = num | 0x20; //小文字に統一して判断する。
        if (num >= 'A' && num <= 'G') {
            gp = num - 'A';
        }
        else if (num == 'P') {
            gp = 7;
        }
        else return -1;
    }
    if (gp < 0 || gp > 7) {
        printf("Unexpected error.\n");
        return -1;
    }
    return gpios[gp];
 /*
        0:A:5
        1 : B : 6
        2 : C : 22
        3 : D : 27
        4 : E : 17
        5 : F : 25
        6 : G : 24
        7 : P : 23
*/
}


void write_gpio_value(int gpio_pin, char value) { // gpio_pin を追加
    int fd;
    char path_buffer[64]; // ファイルパスを格納するためのバッファ

    // ファイルパス文字列 "/sys/class/gpio/gpio<gpio_pin>/value" を生成
    // snprintf を使用してバッファオーバーフローを安全に防ぐ
    snprintf(path_buffer, sizeof(path_buffer), "/sys/class/gpio/gpio%d/value", gpio_pin);

    fd = open(path_buffer, O_WRONLY); // 動的に生成したパスを使用
    if (fd == -1) {
        perror("Failed to open file.");
        exit(EXIT_FAILURE);
    }
    if (write(fd, &value, 1) == -1) {
        printf("[%c]\n", value);
        perror("Failed to set value to the file.");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

/**
 * @brief 指定されたGPIO番号の value ファイルへのパス文字列を生成
 * @param gpio_pin GPIO番号
 * @return 生成されたパス文字列へのポインタ。失敗した場合は NULL
 */
char* get_gpio_value_path(int gpio_pin) {
    int pin_str_len = snprintf(NULL, 0, "%d", gpio_pin);
    // ヒープにメモリを動的に確保
    size_t fixed_len = strlen("/sys/class/gpio/gpio/value");
    size_t required_len = fixed_len + pin_str_len + 1;
    char* path_buffer = (char*)malloc(required_len);
    if (path_buffer == NULL) {
        return NULL; // メモリ確保失敗
    }

    snprintf(path_buffer, required_len, "/sys/class/gpio/gpio%d/value", gpio_pin);
    return path_buffer;
}

/*
    
/**
 * @brief 指定されたGPIO番号の fname ファイルへのパス文字列を生成
 * @param ptr 代入する文字列配列へのポインタ
 * @param gpion GPIO番号
 * @param fname ファイル名。例 "value"
 */
void set_gpio_path(char* ptr, int gpion, char fname[]) {
    snprintf(ptr, 250, "/sys/class/gpio/gpio%d/%s", gpion, fname);
}


void unexport_all(void) {
    for (int i = 0; i < LEDS; i++) {
//        deleteFile(getGpID(i));
        unexport((int)getGpID(i));
        // タイミング調整のため待機
        usleep(500000); // 50ms 待機
        printf("%d has been unexported.\n",getGpID(i));
    }
}


void unexport(int gpio_pin) {
    int fd;
    char path_buffer[64];
    char pin_str[4]; // Buffer for pin number string (e.g., "18", "123")

    // Convert the integer pin number to a string
    snprintf(pin_str, sizeof(pin_str), "%d", gpio_pin);

    // Export
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open export file");
        return;
    }
    write(fd, pin_str, strlen(pin_str));
    close(fd);
}

//LED表示に必要なすべての gpio を準備する。
void standby_all(void) {
    for (int i = 0; i < LEDS; i++) {
        standby(getGpID(i));
        usleep(500000); // 50ms 待機
    }
}

//export して directory GPxxを作り、direction < out として準備する。
/*
void standby(int gpio_pin) {
    int fd;
    char path_buffer[64];
    char pin_str[4]; // Buffer for pin number string (e.g., "18", "123")

    // Convert the integer pin number to a string
    snprintf(pin_str, sizeof(pin_str), "%d", gpio_pin);

    // Export
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open export file");
        return;
    }
    write(fd, pin_str, strlen(pin_str));
    close(fd);

    // Set the GPIO direction to "out"
    snprintf(path_buffer, sizeof(path_buffer), "/sys/class/gpio/gpio%s/direction", pin_str);

    fd = open(path_buffer, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open direction file");
        return;
    }
    write(fd, "out", 3);
    close(fd);
}
*/

void standby(int gpio_pin) {
    int fd;
    char path_buffer[64];
    char pin_str[4];

    // GPIO番号を文字列に変換
    snprintf(pin_str, sizeof(pin_str), "%d", gpio_pin);

    // GPIOをexport
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open export file");
        return;
    }
    write(fd, pin_str, strlen(pin_str));
    close(fd);
    printf("- GPIO %s exported.-\n",pin_str);

    // タイミング調整のため待機
    usleep(500000); // 50ms 待機

    // direction ファイルパスを生成
    snprintf(path_buffer, sizeof(path_buffer), "/sys/class/gpio/gpio%s/direction", pin_str);

    // direction を "out" に設定
    fd = open(path_buffer, O_WRONLY);
    if (fd < 0) {
        printf("\n< %s >", path_buffer);
        perror("Failed to open direction file");
        // direction の設定に失敗しても、exportしたGPIOはunexportする
        fd = open("/sys/class/gpio/unexport", O_WRONLY);
        if (fd >= 0) {
            write(fd, pin_str, strlen(pin_str));
            close(fd);
        }
        return;
    }
    write(fd, "out", 3);
    close(fd);
    printf("- GPIO %s directioned.-\n",pin_str);
}


void unexportGpio(int gpio_pin) {
    int fd;
    char pin_str[4]; // Buffer for pin number string (e.g., "18", "123")

    // Convert the integer pin number to a string
    snprintf(pin_str, sizeof(pin_str), "%d", gpio_pin);

    // Open the unexport file
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open unexport file");
        return;
    }

    // Write the pin number string to unexport the GPIO
    write(fd, pin_str, strlen(pin_str));
    close(fd);
    printf(">> GPIO %s had been unexported. <<\n", pin_str);
}

int main(void) {
    // すべてのgpioを開いて、準備する。(export)
    standby_all();
    // カウントダウンしたいので、9⇒0までのLED表示をする。
     for(int i=9;i>=0;i--){
         //iを表示する関数の呼び出し。
         LightNum(i);
        //一秒待つ。
         usleep(1000000);
     }
    //すべてのgpioを閉じる。
    unexport_all();
    printf("=== Demonstration Completed.\n===");

    return 0;
}


