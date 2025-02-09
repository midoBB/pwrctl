#pragma once

#include <QString>
#include <QStringList>
#include <QObject>

class PowerProfileManager : public QObject {
  Q_OBJECT
public:
    PowerProfileManager(QObject *parent = nullptr);

    QStringList getPowerProfiles();
    QString getActivePowerProfile();

private:
    void parsePowerProfiles(const QString &output);
    QStringList powerProfiles;
    QString activeProfile;
};
