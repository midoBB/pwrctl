#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QDateTime>
#include <qnamespace.h>

class Application : public QApplication {
  Q_OBJECT
public:
  Application(int &argc, char **argv);
signals:
  void powerSourceChanged(bool onBattery);

private slots:
  void handleSave();
  void handleCancel();
  void handleDBusSignal(QDBusMessage message);

private:
  void setupPowerMonitoring();
  bool m_onBattery = false;
  QMainWindow *mainWindow;
  QSystemTrayIcon *trayIcon;
  QComboBox *displayPlugged;
  QComboBox *displayBattery;
  QComboBox *sleepPlugged;
  QComboBox *sleepBattery;
  QComboBox *lidClosePlugged;
  QComboBox *lidCloseBattery;
};
