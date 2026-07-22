#pragma once

#include <stddef.h>
#include <stdint.h>

#define HIGH 0x1
#define LOW 0x0
#define OUTPUT 0x1

unsigned long micros();
unsigned long millis();
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t value);
void delayMicroseconds(unsigned int us);

class Print {
public:
    size_t print(const char* value);
    size_t print(int32_t value);
    size_t print(float value);
    size_t println(const char* value);
};

class Stream : public Print {
public:
    int available();
    int read();
};
