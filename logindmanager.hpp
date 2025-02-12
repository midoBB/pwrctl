#pragma once

#include <QObject>
#include <QString>

#include <QDebug>

#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QTextStream>

class LogindManager : public QObject {
  Q_OBJECT
public:
  LogindManager(QObject *parent = nullptr);
  void applyConfig(QString lidBatteryAction, QString lidPluggedAction,
                   QString powerButtonAction);


private:
  QString handleLidSwitch;
  QString handleLidSwitchExternalPower;
  QString handlePowerKey;
};
