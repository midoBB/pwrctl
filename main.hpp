#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <qnamespace.h>

class Worker : public QObject {
  Q_OBJECT
public:
  Worker(QObject *parent = nullptr);
  ~Worker() override;
  QTimer *timer;

public slots:
  void doWork();

signals:
  void onBatteryChanged(bool onBattery);

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
  QThread *workerThread;
  Worker *worker;
};
