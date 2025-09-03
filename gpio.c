/*
Raspberry Pi �� GPIO �𐧌�E���p����֐��Q
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "gpio.h"

#define GPIOS 8 // �g�p���� Raspberry Pi ��GPIO(7�Z�O�����gLED�p)�̐�

/**
 * @brief �w��C���f�N�X�ɑΉ����� GPIO �� ID ��Ԃ��֐��B
 * @param num 0-7:�v�fID A-FPa-fp:7�Z�O�����g��LED�ʒu�������A���t�@�x�b�g�B0-7�ɕϊ������B
           0 : A : 5
           1 : B : 6
           2 : C : 22
           3 : D : 27
           4 : E : 17
           5 : F : 25
           6 : G : 24
           7 : P : 23
 * @return -1:���͒l�����������������ꍇ�B����ȊO�́AGPIO�̐��l�B
*/
int getGpID(char num) {
    int gpios[GPIOS] = { 5,6,22,27,17,25,24,23 }; // 7�Z�O�����gA?P(=Dp)�ɑΉ�����Raspberry Pi ��GPIO�ԍ�
    int gp = 0;
    if (num >= 0 && num <= 7) {
        gp = num;
    }
    else {
        num = num | 0x20; //�������ɓ��ꂵ�Ĕ��f����B
        if (num >= 'A' && num <= 'G') {
            gp = num - 'A';
        }
        else if (num == 'P') {
            gp = 7;
        }
        else return -1;
    }
    if (gp < 0 || gp > 7) {
        printf("getGpID : Unexpected error.\n");
        return -1;
    }
    return gpios[gp];
}

/**
 * @brief value�t�@�C���ɒl���������ށB
 * @param gpion GPIO�̔ԍ�
 * @param value �������ޕ����R�[�h�B�� '1' '0'
*/
void write_gpio_value(int gpion, char value) { // gpion ��ǉ�
    int fd;
    char path_buffer[64]; // �t�@�C���p�X���i�[���邽�߂̃o�b�t�@

    // �t�@�C���p�X������ "/sys/class/gpio/gpio<gpion>/value" �𐶐�
    // snprintf :�o�b�t�@�I�[�o�[�t���[�΍�
    snprintf(path_buffer, sizeof(path_buffer), "/sys/class/gpio/gpio%d/value", gpion);

    fd = open(path_buffer, O_WRONLY);
    if (fd == -1) {
        perror("Failed to open file.");
        exit(EXIT_FAILURE);
    }
    if (write(fd, &value, 1) == -1) {
        printf("[%c]\n", value);
        perror("Failed to set the value to the file.");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

/**
 * @brief �w�肳�ꂽGPIO�ԍ��� value �t�@�C���ւ̃f�B���N�g���܂ޕ�����(�p�X������)�𐶐�����֐�
 * @param gpion GPIO�ԍ�
 * @return �������ꂽ�p�X������ւ̃|�C���^�B���s�����ꍇ�� NULL
 * @warning �Ăяo������ free() ���邱��
 * @note ����free()���Y�ꃊ�X�N������̂� set_gpio_path ���ւŌォ�������B
 * �Ăяo�����ŕ�����z���p�ӂ��Ă����� set_gpio_path ���g���΍ςނ̂ŁA������͌���g��Ȃ��B
 */
char* get_gpio_value_path(int gpion) {
    int pin_str_len = snprintf(NULL, 0, "%d", gpion);

    // �p�X�̌Œ蕔���̒���
    size_t fixed_len = strlen("/sys/class/gpio/gpio/value");

    // �K�v�ȍ��v�T�C�Y = �Œ蕔�� + GPIO�ԍ� + NULL�I�[����
    size_t required_len = fixed_len + pin_str_len + 1;

    // �q�[�v�Ƀ������𓮓I�Ɋm��
    char* path_buffer = (char*)malloc(required_len);
    if (path_buffer == NULL) {
        return NULL; // �������m�ێ��s
    }

    snprintf(path_buffer, required_len, "/sys/class/gpio/gpio%d/value", gpion);
    return path_buffer;
}

/**
 * @brief �w�肳�ꂽGPIO�ԍ��� fname �t�@�C���ւ̃p�X������𐶐�
 * @param ptr ������镶����z��ւ̃|�C���^
 * @param gpion GPIO�ԍ�
 * @param fname �t�@�C�����B�� "value"
 * @note �Ăяo�����ŕ�����z���p�ӂ��Ă������ƁB250�o�C�g����Ώ\���B
 * ������������Y�ꃊ�X�N���������̂� get_gpio_value_path �̑�ւƂ��ėp�ӂ����B
 */
void set_gpio_path(char *ptr, int gpion, char fname[]) {
    snprintf(ptr, 250, "/sys/class/gpio/gpio%d/%s", gpion, fname);
}

/**
 * @brief ���ׂĂ� gpio �� unexport ����B
 */
void unexport_all(void) {
    for (int i = 0; i < GPIOS; i++) {
        unexport((int)getGpID(i));
        // �^�C�~���O�����̂��ߑҋ@
        usleep(500000); // 50ms �ҋ@
        printf("%d has been unexported.\n", getGpID(i));
    }
}

/**
 * @brief �w�肳�ꂽ GPIO �� unexport ����B
 * @param gpion GPIO�ԍ�
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
    // printf(">> GPIO %s was unexported. <<\n", pin_str);
}

/**
 * @brief ���ׂĂ� gpio �� standby (export + direction out) ����B
 */
void standby_all(void) {
    for (int i = 0; i < GPIOS; i++) {
        standby(getGpID(i));
        usleep(500000); // 50ms �ҋ@
    }
}

/**
 * @brief �w�肳�ꂽ GPIO �� standby (export + direction out) ����B
 * @param gpion GPIO�ԍ�
 * @note export ������� direction ��ݒ肷��܂łɏ������Ԃ�������̂ŁAusleep() �őҋ@�����Ă���B
 * �������s����ꍇ�́A�ҋ@���Ԃ𑝂₵�Ă݂邱��
 * �����ɂ́Aexport ������� direction �t�@�C�����ł���܂ő҂ׂ����������A���쎞�Ԃɐ���������ȗ��������B
 */
void standby(int gpion) {
    int fd;
    char path_buffer[64];
    char pin_str[4];

    // GPIO�ԍ��𕶎���ɕϊ�
    snprintf(pin_str, sizeof(pin_str), "%d", gpion);

    // GPIO��export
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        perror("Failed to open export file");
        return;
    }
    write(fd, pin_str, strlen(pin_str));
    close(fd);
    printf("- GPIO %s exported.-\n", pin_str);

    // �^�C�~���O�����̂��ߑҋ@
    usleep(500000); // 50ms �ҋ@

    // direction �t�@�C���p�X�𐶐�
    snprintf(path_buffer, sizeof(path_buffer), "/sys/class/gpio/gpio%s/direction", pin_str);

    // direction �� "out" �ɐݒ�
    fd = open(path_buffer, O_WRONLY);
    if (fd < 0) {
        printf("\n< %s >", path_buffer);
        perror("Failed to open direction file");
        // direction �̐ݒ�Ɏ��s���Ă��Aexport����GPIO��unexport����
        fd = open("/sys/class/gpio/unexport", O_WRONLY);
        if (fd >= 0) {
            write(fd, pin_str, strlen(pin_str));
            close(fd);
        }
        return;
    }
    write(fd, "out", 3);
    close(fd);
    printf("- GPIO %s directioned.-\n", pin_str);
}

/**
 * @note unexport �Ƃ��č�蒼���Ă��̂܂܎c���Ă���B
 * ����g���Ă��Ȃ��B
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
    printf(">> GPIO %s had been unexported. <<\n", pin_str);
}


