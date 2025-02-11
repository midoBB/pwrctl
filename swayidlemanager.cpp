#include "swayidlemanager.hpp"
#include <qprocess.h>

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

void SwayIdleManager::applyConfig(int16_t lockTimeout, int16_t screenTimeout) {
  QFile file(configPath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "Could not open Swayidle config file for reading:"
               << configPath;
    return;
  }

  // Read all lines from the config file
  QStringList lines;
  while (!file.atEnd()) {
    lines << file.readLine().trimmed();
  }
  file.close();

  // Regular expression to match a timeout line.
  // It captures:
  //   group(1): any leading whitespace,
  //   group(2): an optional comment marker (“# ”) if present,
  //   group(3): the literal "timeout" plus following whitespace,
  //   group(4): the number (the timeout value),
  //   group(5): the rest of the line (the command and everything else)
  QRegularExpression re("^(\\s*)(#\\s*)?(timeout\\s+)(\\d+)(.*)$");

  // These booleans track which line we’ve seen (in order).
  bool foundLock = false;
  bool foundScreen = false;
  bool foundDiff = false;

  for (int i = 0; i < lines.size(); ++i) {
    QString line = lines[i];
    QRegularExpressionMatch match = re.match(line);
    if (match.hasMatch()) {
      QString indent = match.captured(1);
      QString comment =
          match.captured(2); // may be empty or "# " if already commented
      QString keyword = match.captured(3); // "timeout " (with trailing space)
      QString numberStr =
          match.captured(4);            // the current number (we ignore it)
      QString rest = match.captured(5); // the rest of the line (command etc.)

      // Decide which timeout this is by checking the command text.
      // (Order matters because sometimes the same command text appears more
      // than once.)
      if (rest.contains("swaylock")) {
        if (!foundLock) {
          // This is the first occurrence (the lock line)
          foundLock = true;
          if (lockTimeout == -1) {
            // Comment out if lock timeout is -1
            if (!line.trimmed().startsWith("#"))
              line = indent + "# " + keyword + numberStr + rest;
          } else {
            // Make sure the line is uncommented and update the number
            line = indent + keyword + QString::number(lockTimeout) + rest;
          }
        } else if (!foundDiff) {
          // This is the second occurrence with "swaylock" – our diff line.
          foundDiff = true;
          int diff = screenTimeout - lockTimeout;
          // If lockTimeout is -1 or the computed diff is negative, comment out
          // the line.
          if (lockTimeout == -1 || diff < 1) {
            if (!line.trimmed().startsWith("#"))
              line = indent + "# " + keyword + numberStr + rest;
          } else {
            line = indent + keyword + QString::number(diff) + rest;
          }
        }
      } else if (rest.contains("power off") && !foundScreen) {
        // This is the screen timeout line
        foundScreen = true;
        if (screenTimeout == -1) {
          if (!line.trimmed().startsWith("#"))
            line = indent + "# " + keyword + numberStr + rest;
        } else {
          line = indent + keyword + QString::number(screenTimeout) + rest;
        }
      }
    }
    lines[i] = line;
  }
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text |
                 QIODevice::Truncate)) {
    qWarning() << "Could not open Swayidle config file for writing:"
               << configPath;
    return;
  }
  QTextStream out(&file);
  for (const QString &l : lines) {
    out << l << "\n";
  }
  file.close();

  QProcess *killallProcess = new QProcess(this);
  killallProcess->start("killall", QStringList() << "swayidle");
  killallProcess->waitForFinished();
  delete killallProcess;

  QProcess *swayidleProcess = new QProcess(this);
  bool executed = swayidleProcess->startDetached("swayidle", QStringList() << "-w" << "-d");
  if (!executed) {
    qWarning() << "Could not start swayidle";
  }
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
