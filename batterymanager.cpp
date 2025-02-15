#include "batterymanager.hpp"

BatteryManager::BatteryManager(QString native_path) : QObject(nullptr) {
    this->native_path = native_path;
}

QString BatteryManager::getPowerSupplyPath() {
    return POWER_SUPPLY_PATH.arg(native_path);
}
bool BatteryManager::readPowerSupplyStatus() {
  QFile inputFile(getPowerSupplyPath());
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
