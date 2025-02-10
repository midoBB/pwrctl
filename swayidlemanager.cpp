#include "swayidlemanager.hpp"
#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

SwayIdleManager::SwayIdleManager(QObject *parent) : QObject(parent) {}

void SwayIdleManager::parseConfig() {
  this->configPath = getConfigPath();
  QFile configFile(configPath);

  if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "Could not open Swayidle config file:" << configPath;
    return;
  }

  QTextStream in(&configFile);
  while (!in.atEnd()) {
    QString line = in.readLine();
    if (line.startsWith("timeout")) {
      if (line.contains("swaylock")) {
        screenLockTimeoutSeconds = extractTimeout(line);
      } else if (line.contains("output * power off")) {
        displayPowerOffTimeoutSeconds = extractTimeout(line);
      }
    }
  }

  configFile.close();

  qDebug() << "Screen lock timeout:" << screenLockTimeoutSeconds;
  qDebug() << "Display power off timeout:" << displayPowerOffTimeoutSeconds;
}

void SwayIdleManager::applyConfig() {
  qDebug() << "Applying Swayidle config";
  // TODO: Implement config applying logic
}

int SwayIdleManager::extractTimeout(const QString &line) {
  QRegularExpression re("timeout\\s+(\\d+)");
  QRegularExpressionMatch match = re.match(line);
  if (match.hasMatch()) {
    return match.captured(1).toInt();
  }
  return 0;
}

QString SwayIdleManager::getConfigPath() {
  return QDir::homePath() + "/.config/swayidle/config";
}
