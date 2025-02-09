#include "powerprofile.hpp"
#include <QProcess>
#include <QRegularExpression>

PowerProfileManager::PowerProfileManager(QObject *parent) : QObject(parent) {}

QHash<QString, QString> PowerProfileManager::getPowerProfiles() {
  QProcess *process = new QProcess(this);
  process->setProgram("powerprofilesctl");
  process->setArguments(QStringList() << "list");
  process->start();
  process->waitForFinished();

  QString output = process->readAllStandardOutput();
  parsePowerProfiles(output);
  return profiles;
}

QPair<QString, QString> PowerProfileManager::getActivePowerProfile() {
  return activeProfile;
}

QString capitalise_each_word(const QString &sentence) {
  QStringList words = sentence.split(" ", Qt::SkipEmptyParts);
  for (QString &word : words)
    word.front() = word.front().toUpper();

  return words.join(" ");
}

void PowerProfileManager::parsePowerProfiles(const QString &output) {
  profiles.clear();
  QStringList lines = output.split("\n");
  QRegularExpression profileRegex("^\\s*(\\*?\\s*\\w[\\w\\-]*):$");

  for (const QString &line : lines) {
    QRegularExpressionMatch match = profileRegex.match(line);
    if (match.hasMatch()) {
      QString profileName = match.captured(1).trimmed();
      bool isActive = profileName.startsWith("*");
      QString commandLineName = profileName;
      if (isActive) {
        profileName = profileName.mid(1).trimmed();
        commandLineName = commandLineName.mid(1).trimmed();
        activeProfile =
            qMakePair(commandLineName,
                      capitalise_each_word(profileName.replace("-", " ")));
      }
      QString displayName = capitalise_each_word(profileName.replace("-", " "));

      profiles.insert(commandLineName, displayName);
    }
  }
}

QString PowerProfileManager::getCommandNameForProfile(
    const QString &profileName) const {
  return profiles.key(profileName);
}
QString PowerProfileManager::getDisplayNameForProfile(
    const QString &profileName) const {
  return profiles.value(profileName);
}
