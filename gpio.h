#pragma once

int getGpID(char num);
void write_gpio_value(int gpio_pin, char value);
char* get_gpio_value_path(int gpio_pin);
void set_gpio_path(char* ptr, int gpion, char fname[]);
void unexport_all(void);
void unexport(int gpio_pin);
void standby_all(void);
void standby(int gpio_pin);
void unexportGpio(int gpio_pin);

