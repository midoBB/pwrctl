#pragma once

#include <QObject>
#include <QString>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTextStream>
#include <QProcess>
#include <cstdint>

class SwayIdleManager : public QObject {
  Q_OBJECT
public:
  SwayIdleManager(QObject *parent = nullptr);

  void parseConfig();
  void applyConfig(int16_t lockTimeout, int16_t screenTimeout, int16_t suspendTimeout);
  QString getConfigPath();

private:
  int screenLockTimeoutSeconds = 0;
  int displayPowerOffTimeoutSeconds = 0;
  int suspendTimeoutSeconds = 0;
  QString configPath;

  int extractTimeout(const QString &line);
};

