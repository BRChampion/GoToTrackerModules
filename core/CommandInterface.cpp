#include "CommandInterface.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static char* nextToken(char*& cursor) {
    while (*cursor == ' ' || *cursor == '\t' || *cursor == '\r' || *cursor == '\n') {
        ++cursor;
    }
    if (*cursor == '\0') return 0;

    char* token = cursor;
    while (*cursor != '\0' &&
           *cursor != ' ' &&
           *cursor != '\t' &&
           *cursor != '\r' &&
           *cursor != '\n') {
        ++cursor;
    }

    if (*cursor != '\0') {
        *cursor = '\0';
        ++cursor;
    }
    return token;
}

CommandInterface::CommandInterface(AltAzTracker& tracker,
                                   AxisController& altAxis,
                                   AxisController& azAxis,
                                   CommandOutput& out)
    : _tracker(tracker), _altAxis(altAxis), _azAxis(azAxis), _out(out) {}

void CommandInterface::handleLine(char* line) {
    char* cursor = line;
    char* cmd = nextToken(cursor);
    if (cmd == 0) return;

    if (strcmp(cmd, "help") == 0) {
        printHelp();
    } else if (strcmp(cmd, "status") == 0) {
        printStatus();
    } else if (strcmp(cmd, "target") == 0) {
        selectTarget(nextToken(cursor));
    } else if (strcmp(cmd, "goto") == 0) {
        if (!_tracker.hasTarget()) {
            _out.println("ERR no target");
            return;
        }
        _tracker.startGoto();
        _out.println("OK goto");
    } else if (strcmp(cmd, "track") == 0) {
        const char* arg = nextToken(cursor);
        if (arg != 0 && strcmp(arg, "on") == 0) {
            if (!_tracker.hasTarget()) {
                _out.println("ERR no target");
                return;
            }
            _tracker.setTracking(true);
            _out.println("OK track on");
        } else if (arg != 0 && strcmp(arg, "off") == 0) {
            _tracker.setTracking(false);
            _out.println("OK track off");
        } else {
            _out.println("ERR usage: track on|off");
        }
    } else if (strcmp(cmd, "stop") == 0) {
        _tracker.stop();
        _out.println("OK stop");
    } else {
        _out.println("ERR unknown command");
    }
}

void CommandInterface::printHelp() {
    _out.println("Commands:");
    _out.println("help");
    _out.println("status");
    _out.println("target <index|name>");
    _out.println("goto");
    _out.println("track on|off");
    _out.println("stop");
}

void CommandInterface::printStatus() {
    const SkyMath::HorizontalCoord hz = _tracker.currentHorizontal();

    _out.print("target=");
    if (_targetIndex >= 0) {
        _out.print(TargetCatalog::get((uint8_t)_targetIndex).name);
    } else {
        _out.print("none");
    }
    _out.print(" tracking=");
    _out.print(_tracker.tracking() ? "on" : "off");
    _out.print(" altSteps=");
    _out.printInt(_altAxis.posSteps());
    _out.print(" azSteps=");
    _out.printInt(_azAxis.posSteps());
    _out.print(" altTarget=");
    _out.printInt(_tracker.altTargetSteps());
    _out.print(" azTarget=");
    _out.printInt(_tracker.azTargetSteps());
    _out.print(" altDeg=");
    _out.printFloat(hz.altDeg);
    _out.print(" azDeg=");
    _out.printFloat(hz.azDeg);
    _out.println("");
}

void CommandInterface::selectTarget(const char* arg) {
    const int8_t index = parseTargetIndex(arg);
    if (index < 0) {
        _out.println("ERR target not found");
        return;
    }

    const TargetEq& target = TargetCatalog::get((uint8_t)index);
    SkyMath::EquatorialCoord eq = {target.raHours, target.decDeg};
    _tracker.setTarget(eq);
    _targetIndex = index;

    _out.print("OK target ");
    _out.println(target.name);
}

int8_t CommandInterface::parseTargetIndex(const char* arg) const {
    if (arg == 0 || *arg == '\0') return -1;

    if (isdigit((unsigned char)arg[0])) {
        const int index = atoi(arg);
        return (index >= 0 && index < TargetCatalog::count()) ? (int8_t)index : -1;
    }

    for (uint8_t i = 0; i < TargetCatalog::count(); ++i) {
        if (strcmp(arg, TargetCatalog::get(i).name) == 0) return (int8_t)i;
    }
    return -1;
}
