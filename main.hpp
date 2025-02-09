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
#include <qnamespace.h>

class Worker : public QObject {
  Q_OBJECT
public:
  Worker(QObject *parent = nullptr);
  ~Worker() override;
  QTimer *timer;

public slots:
  void doWork();
  void initialize();

signals:
  void onBatteryChanged(bool onBattery);
  void powerProfilesChanged(const QStringList &profiles,
                            const QString &activeProfile);

private:
  bool readPowerSupplyStatus();
  void parsePowerProfiles(const QString &output);
  bool onBattery = false;
};

class Application : public QApplication {
  Q_OBJECT
public:
  Application(int &argc, char **argv);
  ~Application() override;

signals:
  void powerSourceChanged(bool onBattery);

public slots:
  void updatePowerProfiles(const QStringList &profiles,
                           const QString &activeProfile);

private:
  void startWorker();

private slots:
  void handleSave();
  void handleCancel();

private:
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
  QThread *workerThread;
  Worker *worker;
  QStringList powerProfiles;
  QString activeProfile;
};
