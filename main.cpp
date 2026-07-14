#include <iostream>
#include "core/SimClock.h"
#include "core/FakeDriver.h"
#include "core/AxisController.h"
#include "core/MountModel.h"
#include "core/AltAzTracker.h"
#include "core/CommandInterface.h"

class ConsoleOutput : public CommandOutput {
public:
    void print(const char* text) override { std::cout << text; }
    void printInt(int32_t value) override { std::cout << value; }
    void printFloat(float value) override { std::cout << value; }
    void println(const char* text) override { std::cout << text << "\n"; }
};

static void runCommand(CommandInterface& commands, const char* text) {
    char buffer[32];
    uint8_t i = 0;
    while (text[i] != '\0' && i < sizeof(buffer) - 1) {
        buffer[i] = text[i];
        ++i;
    }
    buffer[i] = '\0';

    std::cout << "> " << buffer << "\n";
    commands.handleLine(buffer);
}

int main() {
    SimClock clk;
    FakeDriver altDriver;
    FakeDriver azDriver;
    AxisController altAxis(clk, altDriver);
    AxisController azAxis(clk, azDriver);

    MountModel altModel(200.0f, 16.0f);
    MountModel azModel(200.0f, 16.0f);
    AltAzTracker tracker(clk, altAxis, azAxis, altModel, azModel);
    ConsoleOutput out;
    CommandInterface commands(tracker, altAxis, azAxis, out);

    altAxis.begin();
    azAxis.begin();
    altAxis.enable(true);
    azAxis.enable(true);

    tracker.begin();
    tracker.setObserver(47.6f, -52.7f);
    tracker.setTime(1783353600UL);

    runCommand(commands, "help");
    runCommand(commands, "target Vega");
    runCommand(commands, "goto");

    for (uint16_t i = 0; i < 2000; ++i) {
        clk.advanceMicros(1000UL);
        tracker.update();
    }

    runCommand(commands, "status");
    runCommand(commands, "track on");

    for (uint16_t i = 0; i < 3000; ++i) {
        clk.advanceMicros(1000UL);
        tracker.update();
    }

    runCommand(commands, "status");
    runCommand(commands, "stop");
    runCommand(commands, "status");
}
