#pragma once

#include <QHash>
#include <QObject>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QHash>

class PowerProfileManager : public QObject {
  Q_OBJECT
public:
  PowerProfileManager(QObject *parent = nullptr);

  QHash<QString, QString> getPowerProfiles();
  QPair<QString, QString> getActivePowerProfile();
  QString getCommandNameForProfile(const QString &profileDisplayName) const;
  QString getDisplayNameForProfile(const QString &profileName) const;

private:
  void parsePowerProfiles(const QString &output);
  QPair<QString, QString> activeProfile;
  QHash<QString, QString> profiles;
};
