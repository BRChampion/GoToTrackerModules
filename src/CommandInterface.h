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

// Minimal serial-monitor style command parser. The input line is modified in
// place while tokenizing, so callers should pass a writable char buffer.
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
    void nudgeAxis(const char* axisName, const char* stepText);
    int8_t parseTargetIndex(const char* arg) const;

private:
    AltAzTracker& _tracker;
    AxisController& _altAxis;
    AxisController& _azAxis;
    CommandOutput& _out;
    int8_t _targetIndex = -1;
};
