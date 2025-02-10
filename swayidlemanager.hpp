#pragma once

#include <QObject>
#include <QString>
#include <QDir>

class SwayIdleManager : public QObject {
  Q_OBJECT
public:
  SwayIdleManager(QObject *parent = nullptr);

  void parseConfig();
  void applyConfig();
  QString getConfigPath();

private:
  int screenLockTimeoutSeconds = 0;
  int displayPowerOffTimeoutSeconds = 0;
  QString configPath;

  int extractTimeout(const QString &line);
};

