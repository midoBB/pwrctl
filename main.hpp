#pragma once
#include "batterymanager.hpp"
#include "logindmanager.hpp"
#include "powerprofile.hpp"
#include "swayidlemanager.hpp"
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <qglobal.h>
#include <qnamespace.h>

class Worker : public QObject {
  Q_OBJECT
public:
  Worker(QObject *parent = nullptr, QString native_path = "",
         bool has_battery = false);
  ~Worker() override;
  QTimer *timer;
  BatteryManager *batteryManager;
  QString native_path;
  bool has_battery;

public slots:
  void doWork();
  void initialize();
  void applyPowerSettings();
  void cleanup();

signals:
  void onBatteryChanged(bool onBattery);
  void workerFinished();

private:
  bool readPowerSupplyStatus();
  SwayIdleManager *swayIdleManager;
  PowerProfileManager profileManager;
  LogindManager logindManager;
  bool onBattery = false;
};

class Application : public QApplication {
  Q_OBJECT
public:
  Application(int &argc, char **argv);
  ~Application() override;

signals:
  void powerSourceChanged(bool onBattery);
  void appLoaded();
  void powerProfilesChanged(const QHash<QString, QString> &profiles,
                            const QPair<QString, QString> &activeProfile);

public slots:
  void updatePowerProfiles(const QHash<QString, QString> &profiles,
                           const QPair<QString, QString> &activeProfile);
  void onAppLoaded();
  void initFromDbus();

private slots:
  void handleSave();
  void handleCancel();

private:
  void startWorker();
  void loadSettings();
  QString getSettingsPath();
  short loadingStep = 0;
  bool m_onBattery = false;
  bool has_battery = false;
  QString power_native_path;
  QMainWindow *mainWindow;
  QSystemTrayIcon *trayIcon;
  QComboBox *lockScreenPlugged;
  QComboBox *lockScreenBattery;
  QComboBox *displayPlugged;
  QComboBox *displayBattery;
  QComboBox *sleepPlugged;
  QComboBox *sleepBattery;
  QComboBox *lidClosePlugged;
  QComboBox *lidCloseBattery;
  QComboBox *powerProfilePlugged;
  QComboBox *powerProfileBattery;
  QComboBox *powerKeyPlugged;
  QComboBox *powerKeyBattery;
  QThread *workerThread;
  QLabel *pluggedText;
  QLabel *batteryText;
  QLabel *batteryIcon;
  Worker *worker;
  QStringList powerProfiles;
  PowerProfileManager profileManager;
};
