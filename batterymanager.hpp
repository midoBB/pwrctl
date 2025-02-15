#pragma once

#include <QFile>
#include <QIODevice>
#include <QString>
#include <QTextStream>
#include <QDebug>

class BatteryManager {
public:
    BatteryManager(QString native_path);
    bool readPowerSupplyStatus();

private:
    const QString POWER_SUPPLY_PATH = "/sys/class/power_supply/%1/online";
    QString getPowerSupplyPath();
    QString native_path;
};
