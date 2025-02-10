#pragma once

#include <QObject>
#include <QString>

class LogindManager : public QObject {
  Q_OBJECT
public:
  LogindManager(QObject *parent = nullptr);

  void parseConfig(const QString &configPath);
  void applyConfig();
};
