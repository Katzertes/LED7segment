/*
Raspberry Pi の GPIO を制御・利用する関数群
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "gpio.h"

#define GPIOS 8 // 使用する Raspberry Pi のGPIO(7セグメントLED用)の数

/**
 * @brief 指定インデクスに対応した GPIO の ID を返す関数。
 * @param num 0-7:要素ID A-FPa-fp:7セグメントのLED位置を示すアルファベット。0-7に変換される。
           0 : A : 5
           1 : B : 6
           2 : C : 22
           3 : D : 27
           4 : E : 17
           5 : F : 25
           6 : G : 24
           7 : P : 23
 * @return -1:入力値がおかししかった場合。それ以外は、GPIOの数値。
*/
int getGpID(char num) {
    int gpios[GPIOS] = { 5,6,22,27,17,25,24,23 }; // 7セグメントA?P(=Dp)に対応するRaspberry Pi のGPIO番号
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
        printf("getGpID : Unexpected error.¥n");
        return -1;
    }
    return gpios[gp];
}

/**
 * @brief valueファイルに値を書き込む。
 * @param gpion GPIOの番号
 * @param value 書き込む文字コード。例 '1' '0'
*/
void write_gpio_value(int gpion, char value) { // gpion を追加
    int fd;
    char path_buffer[64]; // ファイルパスを格納するためのバッファ

    // ファイルパス文字列 "/sys/class/gpio/gpio<gpion>/value" を生成
    // snprintf :バッファオーバーフロー対策
    snprintf(path_buffer, sizeof(path_buffer), "/sys/class/gpio/gpio%d/value", gpion);

    fd = open(path_buffer, O_WRONLY);
    if (fd == -1) {
        perror("Failed to open file.");
        exit(EXIT_FAILURE);
    }
    if (write(fd, &value, 1) == -1) {
        printf("[%c]¥n", value);
        perror("Failed to set the value to the file.");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

/**
 * @brief 指定されたGPIO番号の value ファイルへのディレクトリ含む文字列(パス文字列)を生成する関数
 * @param gpion GPIO番号
 * @return 生成されたパス文字列へのポインタ。失敗した場合は NULL
 * @warning 呼び出し元で free() すること
 * @note そのfree()し忘れリスクがあるので set_gpio_path を代替で後から作った。
 * 呼び出し側で文字列配列を用意しておけば set_gpio_path を使えば済むので、こちらは現状使わない。
 */
char* get_gpio_value_path(int gpion) {
    int pin_str_len = snprintf(NULL, 0, "%d", gpion);

    // パスの固定部分の長さ
    size_t fixed_len = strlen("/sys/class/gpio/gpio/value");

    // 必要な合計サイズ = 固定部分 + GPIO番号 + NULL終端文字
    size_t required_len = fixed_len + pin_str_len + 1;

    // ヒープにメモリを動的に確保
    char* path_buffer = (char*)malloc(required_len);
    if (path_buffer == NULL) {
        return NULL; // メモリ確保失敗
    }

    snprintf(path_buffer, required_len, "/sys/class/gpio/gpio%d/value", gpion);
    return path_buffer;
}

/**
 * @brief 指定されたGPIO番号の fname ファイルへのパス文字列を生成
 * @param ptr 代入する文字列配列へのポインタ
 * @param gpion GPIO番号
 * @param fname ファイル名。例 "value"
 * @note 呼び出し側で文字列配列を用意しておくこと。250バイトあれば十分。
 * メモリ解放し忘れリスクがあったので get_gpio_value_path の代替として用意した。
 */
void set_gpio_path(char *ptr, int gpion, char fname[]) {
    snprintf(ptr, 250, "/sys/class/gpio/gpio%d/%s", gpion, fname);
}

/**
 * @brief すべての gpio を unexport する。
 */
void unexport_all(void) {
    for (int i = 0; i < GPIOS; i++) {
        unexport((int)getGpID(i));
        // タイミング調整のため待機
        usleep(500000); // 50ms 待機
        printf("%d has been unexported.¥n", getGpID(i));
    }
}

/**
 * @brief 指定された GPIO を unexport する。
 * @param gpion GPIO番号
 */
void unexport(int gpion) {
    int fd;
    char pin_str[4];
    // Convert the integer GPIO number to a string
    snprintf(pin_str, sizeof(pin_str), "%d", gpion);

    // Unexport
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open unexport file");
        return;
    }
    write(fd, pin_str, strlen(pin_str));
    close(fd);
    // printf(">> GPIO %s was unexported. <<¥n", pin_str);
}

/**
 * @brief すべての gpio を standby (export + direction out) する。
 */
void standby_all(void) {
    for (int i = 0; i < GPIOS; i++) {
        standby(getGpID(i));
        usleep(500000); // 50ms 待機
    }
}

/**
 * @brief 指定された GPIO を standby (export + direction out) する。
 * @param gpion GPIO番号
 * @note export した後に direction を設定するまでに少し時間がかかるので、usleep() で待機を入れている。
 * もし失敗する場合は、待機時間を増やしてみること
 * 厳密には、export した後に direction ファイルができるまで待つべきだったが、制作時間に制限もあり簡略化した。
 */
void standby(int gpion) {
    int fd;
    char path_buffer[64];
    char pin_str[4];

    // GPIO番号を文字列に変換
    snprintf(pin_str, sizeof(pin_str), "%d", gpion);

    // GPIOをexport
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open export file");
        return;
    }
    write(fd, pin_str, strlen(pin_str));
    close(fd);
    printf("- GPIO %s exported.-¥n", pin_str);

    // タイミング調整のため待機
    usleep(500000); // 50ms 待機

    // direction ファイルパスを生成
    snprintf(path_buffer, sizeof(path_buffer), "/sys/class/gpio/gpio%s/direction", pin_str);

    // direction を "out" に設定
    fd = open(path_buffer, O_WRONLY);
    if (fd < 0) {
        printf("¥n< %s >", path_buffer);
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
    printf("- GPIO %s directioned.-¥n", pin_str);
}

/**
 * @note unexport として作り直してそのまま残している。
 * 現状使われていない。
 */
void unexportGpio(int gpion) {
    int fd;
    char pin_str[4];
    snprintf(pin_str, sizeof(pin_str), "%d", gpion);
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open unexport file");
        return;
    }
    write(fd, pin_str, strlen(pin_str));
    close(fd);
    printf(">> GPIO %s had been unexported. <<¥n", pin_str);
}


