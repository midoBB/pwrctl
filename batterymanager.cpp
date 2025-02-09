#include "batterymanager.hpp"

BatteryManager::BatteryManager() {}

bool BatteryManager::readPowerSupplyStatus() {
    QFile inputFile(POWER_SUPPLY_PATH);
    if (inputFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&inputFile);
        QString line = in.readLine();
        inputFile.close();
        bool isOnline = (line == "1");
        return isOnline;
    } else {
        qWarning() << "Could not open power supply status file:"
                   << POWER_SUPPLY_PATH;
        return false; // Assume plugged in if file can't be read
    }
}
