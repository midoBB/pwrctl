#pragma once
#include "powerprofile.hpp"
#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDateTime>
#include <QFile>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QSystemTrayIcon>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <qglobal.h>
#include <qnamespace.h>
#include <QGraphicsDropShadowEffect>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>

class Worker : public QObject {
  Q_OBJECT
public:
  Worker(QObject *parent = nullptr);
  ~Worker() override;
  QTimer *timer;

public slots:
  void doWork();
  void initialize();
  void applyPowerSettings();

signals:
  void onBatteryChanged(bool onBattery);
  void workerFinished();

private:
  bool readPowerSupplyStatus();
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

private slots:
  void handleSave();
  void handleCancel();

private:
  void startWorker();
  void loadSettings();
  void loadComboSetting(QComboBox* combo, const QString& key);
  QString getSettingsPath();
  short loadingStep = 0;
  bool m_onBattery = false;
  QMainWindow *mainWindow;
  QSystemTrayIcon *trayIcon;
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
  Worker *worker;
  QStringList powerProfiles;
  PowerProfileManager  profileManager;
};
