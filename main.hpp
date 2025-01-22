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
#include <qnamespace.h>
class Application : public QApplication {
  Q_OBJECT
public:
  Application(int &argc, char **argv);

private slots:
  void handleSave();
  void handleCancel();

private:
  QMainWindow *mainWindow;
  QSystemTrayIcon *trayIcon;
  QComboBox *displayPlugged;
  QComboBox *displayBattery;
  QComboBox *sleepPlugged;
  QComboBox *sleepBattery;
  QComboBox *lidClosePlugged;
  QComboBox *lidCloseBattery;
};
