#pragma once

#include <Arduino.h>
#include "Clock.h"
#include "CommandInterface.h"
#include "Driver.h"

class ArduinoClock : public Clock {
public:
    TickMicros micros() const override { return ::micros(); }
    TickMicros millis() const override { return ::millis(); }
};

class StepDirDriver : public Driver {
public:
    static const uint8_t NoEnablePin = 0xFFU;

    StepDirDriver(uint8_t stepPin,
                  uint8_t directionPin,
                  uint8_t enablePin = NoEnablePin,
                  bool enableActiveLow = true,
                  uint16_t pulseWidthMicros = 3U)
        : _stepPin(stepPin),
          _directionPin(directionPin),
          _enablePin(enablePin),
          _enableActiveLow(enableActiveLow),
          _pulseWidthMicros(pulseWidthMicros) {}

    void begin() {
        pinMode(_stepPin, OUTPUT);
        pinMode(_directionPin, OUTPUT);
        digitalWrite(_stepPin, LOW);
        digitalWrite(_directionPin, LOW);

        if (_enablePin != NoEnablePin) {
            pinMode(_enablePin, OUTPUT);
            enable(false);
        }
    }

    void enable(bool on) override {
        if (_enablePin == NoEnablePin) return;
        digitalWrite(_enablePin, (on == _enableActiveLow) ? LOW : HIGH);
    }

    void step(StepDir direction) override {
        digitalWrite(_directionPin,
                     direction == StepDir::Forward ? HIGH : LOW);
        // TB6600-style STEP/DIR drivers need a short high pulse on STEP.
        // This is the only intentional blocking delay in the motion path.
        digitalWrite(_stepPin, HIGH);
        delayMicroseconds(_pulseWidthMicros);
        digitalWrite(_stepPin, LOW);
    }

private:
    uint8_t _stepPin;
    uint8_t _directionPin;
    uint8_t _enablePin;
    bool _enableActiveLow;
    uint16_t _pulseWidthMicros;
};

class StreamCommandOutput : public CommandOutput {
public:
    explicit StreamCommandOutput(Print& output) : _output(output) {}

    void print(const char* text) override { _output.print(text); }
    void printInt(int32_t value) override { _output.print(value); }
    void printFloat(float value) override { _output.print(value); }
    void println(const char* text) override { _output.println(text); }

private:
    Print& _output;
};

class StreamCommandInput {
public:
    StreamCommandInput(Stream& input, CommandInterface& commands)
        : _input(input), _commands(commands) {}

    void update() {
        // Drain all currently available serial bytes, but never wait for more.
        // Complete newline-terminated commands are passed to CommandInterface.
        while (_input.available() > 0) {
            const char ch = (char)_input.read();
            if (ch == '\r') continue;

            if (ch == '\n') {
                if (!_discarding && _length > 0U) {
                    _buffer[_length] = '\0';
                    _commands.handleLine(_buffer);
                }
                _length = 0U;
                _discarding = false;
                continue;
            }

            if (_discarding) continue;
            if (_length < BufferSize - 1U) {
                _buffer[_length++] = ch;
            } else {
                _length = 0U;
                _discarding = true;
            }
        }
    }

private:
    static const uint8_t BufferSize = 64U;
    Stream& _input;
    CommandInterface& _commands;
    char _buffer[BufferSize];
    uint8_t _length = 0U;
    bool _discarding = false;
};
