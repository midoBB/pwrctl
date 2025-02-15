#pragma once

#include <QFile>
#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QString>
#include <QTextStream>
#include <QtGlobal>

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
