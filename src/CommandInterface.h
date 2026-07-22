#pragma once

#include "AltAzTracker.h"
#include "TargetCatalog.h"

class CommandOutput {
public:
    virtual ~CommandOutput() {}
    virtual void print(const char* text) = 0;
    virtual void printInt(int32_t value) = 0;
    virtual void printFloat(float value) = 0;
    virtual void println(const char* text) = 0;
};

class CommandInterface {
public:
    CommandInterface(AltAzTracker& tracker,
                     AxisController& altAxis,
                     AxisController& azAxis,
                     CommandOutput& out);

    void handleLine(char* line);

private:
    void printHelp();
    void printStatus();
    void selectTarget(const char* arg);
    int8_t parseTargetIndex(const char* arg) const;

private:
    AltAzTracker& _tracker;
    AxisController& _altAxis;
    AxisController& _azAxis;
    CommandOutput& _out;
    int8_t _targetIndex = -1;
};
