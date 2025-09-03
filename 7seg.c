/*
Raspberry Pi 7segment LED ����֐�
2025.8 ���� by J.H.
VsCode, Vi
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "gpio.h"
#include "7seg.h"


// ���͂��ꂽ����(0-10)�� LED ��ɕ\������B
// 0-9: ����
// 10 : �s���I�h�̂݁B
// 20-29 : �����ƃs���I�h������\������B
void LightNum(int num) {
    int period;
    //�����ɂ�萔���𒲐�����B
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

    //ledpattern �����i���ŕ\�����ꂽ LED ���s�p�^�[����ǂݍ��ށB
    unsigned char ledpt = ledpattern(num);
    if (period) ledpt = ledpt | 0x80;

    //ledpt �Ŏ����ꂽ�r�b�g�ɂ��A���ꂼ���LED�����点��B
    Light7Seg(ledpt);
}

/**
 * @brief 7segment LED �̊e�Z�O�����g��_��/��������B
 * @param l 7segment LED �̕\���p�^�[�����i��8bit�ŕ\�� char �l�B
 *          �ŉ��ʃr�b�g�� a �Z�O�����g�A���� b �Z�O�����g�A...�A�ŏ�ʃr�b�g�������_�Z�O�����g��\���B
 *          �Ȃ� 0 ���\�����Ӗ����邽�ߔ��]���ēn���B���Ȃ킿bit 0�Ȃ��\���A1�Ȃ�\���ƂȂ�B 
 */
void Light7Seg(char l) {
    for (int i = 0; i < LEDS; i++) {
        //�ŉ��ʃr�b�g������o���Ă����B
        int value = l & 0x01;
        l = l >> 1;
        //���o���� LED-On/Off �t���O�� GPIO value�ɑ���_��/����������
        write_gpio_value(getGpID(i), value + '0');
    }
}

/**
 * @brief 7segment LED �̑S�Z�O�����g����������
 */
void Off7Seg(void) {
    Light7Seg(0xFF);
}

/**
* @brief 7segment LED �̕\���p�^�[�����i��8bit�ŕ\�� char �l��Ԃ��B
* @param num �\�������������B0-9:���� 10:�����_��
* @return LED�\���p�^�[����\�� char �l�B�Ȃ� 0 ���\�����Ӗ����邽�ߔ��]���ĕԂ��B���Ȃ킿bit 0�Ȃ��\���A1�Ȃ�\���ƂȂ�B
*/
unsigned char ledpattern(char num) {
    unsigned char led[] = {
    0x3F, // 0b00111111  - ���� 0
    0x06, // 0b00000110  - ���� 1
    0x5B, // 0b01011011  - ���� 2
    0x4F, // 0b01001111  - ���� 3
    0x66, // 0b01100110  - ���� 4
    0x6D, // 0b01101101  - ���� 5
    0x7D, // 0b01111101  - ���� 6
    0x27, // 0b00100111  - ���� 7
    0x7F, // 0b01111111  - ���� 8
    0x6F, // 0b01101111  - ���� 9
    0x80  // 0b10000000  - �����_ (P)
    };
    return led[num] ^ 0xFF; //���]�����ĕԂ��B�Ȃ��Ȃ� 0 ���\�����Ӗ����邩��B
}


