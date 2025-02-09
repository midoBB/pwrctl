#include "powerprofile.hpp"
#include <QProcess>
#include <QRegularExpression>

PowerProfileManager::PowerProfileManager(QObject *parent) : QObject(parent) {}

QStringList PowerProfileManager::getPowerProfiles() {
    QProcess *process = new QProcess(this);
    process->start("powerprofilesctl", QStringList() << "list");
    process->waitForFinished();

    QString output = process->readAllStandardOutput();
    parsePowerProfiles(output);
    return powerProfiles;
}

QString PowerProfileManager::getActivePowerProfile() {
    return activeProfile;
}

QString capitalise_each_word(const QString &sentence) {
  QStringList words = sentence.split(" ", Qt::SkipEmptyParts);
  for (QString &word : words)
    word.front() = word.front().toUpper();

  return words.join(" ");
}

void PowerProfileManager::parsePowerProfiles(const QString &output) {
    powerProfiles.clear();
    activeProfile.clear();
    QStringList lines = output.split("\n");
    QRegularExpression profileRegex("^\\s*(\\*?\\s*\\w[\\w\\-]*):$");

    for (const QString &line : lines) {
        QRegularExpressionMatch match = profileRegex.match(line);
        if (match.hasMatch()) {
            QString profileName = match.captured(1).trimmed();
            bool isActive = profileName.startsWith("*");
            if (isActive) {
                profileName = profileName.mid(1).trimmed();
                activeProfile = capitalise_each_word(profileName.replace("-", " "));
            }
            powerProfiles.append(capitalise_each_word(profileName.replace("-", " ")));
        }
    }
}
