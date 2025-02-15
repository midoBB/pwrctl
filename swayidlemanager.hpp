#pragma once

#include <QDir>
#include <QFile>
#include <QObject>
#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>
#include <QTextStream>
#include <cstdint>

class SwayIdleManager : public QObject {
  Q_OBJECT
public:
  SwayIdleManager(QObject *parent = nullptr);

  void parseConfig();
  void applyConfig(int16_t lockTimeout, int16_t screenTimeout,
                   int16_t suspendTimeout);
  QString getConfigPath();

public slots:
  void handleSigTerm(int signal);

private:
  int screenLockTimeoutSeconds = 0;
  int displayPowerOffTimeoutSeconds = 0;
  int suspendTimeoutSeconds = 0;
  QString configPath;

  int extractTimeout(const QString &line);
};
