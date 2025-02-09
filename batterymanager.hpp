#pragma once

#include <QFile>
#include <QIODevice>
#include <QString>
#include <QTextStream>
#include <QDebug>

class BatteryManager {
public:
    BatteryManager();
    bool readPowerSupplyStatus();

private:
    const QString POWER_SUPPLY_PATH = "/sys/class/power_supply/ADP1/online";
};
