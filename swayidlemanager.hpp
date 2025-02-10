#pragma once

#include <QObject>
#include <QString>

class SwayIdleManager : public QObject {
  Q_OBJECT
public:
  SwayIdleManager(QObject *parent = nullptr);

  void parseConfig(const QString &configPath);
  void applyConfig();
};
