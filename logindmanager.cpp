#include "logindmanager.hpp"

const QString LOGIND_CONFIG_PATH = "/etc/systemd/logind.conf";

LogindManager::LogindManager(QObject *parent) : QObject(parent) {}

void LogindManager::applyConfig(QString lidBatteryAction,
                                QString lidPluggedAction,
                                QString powerButtonAction) {
  QFile readFile(LOGIND_CONFIG_PATH);
  if (!readFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "Could not open logind config file for reading:"
               << LOGIND_CONFIG_PATH;
    return;
  }

  QTextStream in(&readFile);
  QString content = in.readAll();
  QStringList lines = content.split("\n");
  readFile.close();

  QRegularExpression lidSwitchRegex("^#?\\s*HandleLidSwitch\\s*=.*");
  QRegularExpression lidSwitchExternalPowerRegex(
      "^#?\\s*HandleLidSwitchExternalPower\\s*=.*");
  QRegularExpression lidSwitchDockedRegex(
      "^#?\\s*HandleLidSwitchDocked\\s*=.*");
  QRegularExpression powerKeyRegex("^#?\\s*HandlePowerKey\\s*=.*");

  for (QString &line : lines) {
    if (lidSwitchRegex.match(line).hasMatch()) {
      line = "HandleLidSwitch=" + lidBatteryAction;
    } else if (lidSwitchExternalPowerRegex.match(line).hasMatch()) {
      line = "HandleLidSwitchExternalPower=" + lidPluggedAction;
    } else if (lidSwitchDockedRegex.match(line).hasMatch()) {
      line = "HandleLidSwitchDocked=" + lidPluggedAction;
    } else if (powerKeyRegex.match(line).hasMatch()) {
      line = "HandlePowerKey=" + powerButtonAction;
    }
  }

  QString newContent = lines.join("\n") + "\n";

  // Use pkexec to write the modified content to the file
  QProcess *process = new QProcess();
  process->setProgram("pkexec");
  process->setArguments(QStringList() << "tee" << LOGIND_CONFIG_PATH);
  process->start();
  process->waitForStarted();
  process->write(newContent.toUtf8());
  process->closeWriteChannel();
  process->waitForFinished();

  if (process->exitCode() != 0) {
    qWarning() << "Failed to apply Logind config:" << process->errorString();
  }
  // Restart logind service
  QStringList restartArguments;
  restartArguments << "systemctl" << "reload" << "systemd-logind.service";
  QProcess *restartProcess = new QProcess();
  restartProcess->setProgram("pkexec");
  restartProcess->setArguments(restartArguments);
  restartProcess->start();
  restartProcess->waitForFinished();

  if (restartProcess->exitCode() != 0) {
    qWarning() << "Failed to restart logind:" << restartProcess->errorString();
  }
}
