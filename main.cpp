#include "main.hpp"
#include "QOrderedMap.h"
#include "batterymanager.hpp"
#include "powerprofile.hpp"
#include <csignal>
#include <qnamespace.h>
static QCoreApplication *g_app = nullptr;
void termSignalHandler(int signal) {
  if ((signal == SIGTERM || signal == SIGQUIT || signal == SIGINT) &&
      g_app != nullptr) {
    QMetaObject::invokeMethod(g_app, "quit", Qt::QueuedConnection);
  }
}
static const OrderedMap<QString, int16_t> timeouts = {
    {"2 minutes", 120},  {"5 minutes", 300},   {"10 minutes", 600},
    {"15 minutes", 900}, {"30 minutes", 1800}, {"1 hour", 3600},
    {"Never", -1},
};

static const OrderedMap<QString, QString> logindActions = {
    {"Sleep", "suspend"},
    {"PowerOff", "poweroff"},
    {"Reboot", "reboot"},
    {"Do nothing", "ignore"},
};

Worker::Worker(QObject *parent, QString native_path, bool has_battery)
    : QObject(parent), native_path(native_path), has_battery(has_battery) {
  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &Worker::doWork);
  timer->setInterval(5000);
  this->batteryManager = new BatteryManager(native_path);
  this->swayIdleManager = new SwayIdleManager(this);
  onBattery = !batteryManager->readPowerSupplyStatus();
}

void Worker::initialize() {
  timer->start();
  swayIdleManager->parseConfig();
  emit workerFinished();
}

Worker::~Worker() {
  timer->stop();
  delete batteryManager;
}

void Worker::applyPowerSettings() {
  QString settingsPath =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" +
      QCoreApplication::applicationName() + "/settings.ini";
  QSettings settings(settingsPath, QSettings::IniFormat);
  QString lockScreenKey = onBattery ? "LockScreenBattery" : "LockScreenPlugged";
  QString displayKey = onBattery ? "DisplayBattery" : "DisplayPlugged";
  QString sleepKey = onBattery ? "SleepBattery" : "SleepPlugged";
  auto lidActionBattery =
      logindActions.value(settings.value("LidCloseBattery").toString());
  auto lidActionPlugged =
      logindActions.value(settings.value("LidClosePlugged").toString());
  QString powerKeyKey = onBattery ? "PowerKeyBattery" : "PowerKeyPlugged";
  auto powerKeyAction =
      logindActions.value(settings.value(powerKeyKey).toString());
  QString powerProfileKey =
      onBattery ? "PowerProfileBattery" : "PowerProfilePlugged";
  int16_t lockTimeout =
      timeouts.value(settings.value(lockScreenKey).toString());
  int16_t screenTimeout = timeouts.value(settings.value(displayKey).toString());
  int16_t suspendTimeout = timeouts.value(settings.value(sleepKey).toString());

  QString powerProfile = settings.value(powerProfileKey).toString();
  profileManager.applyPowerProfile(powerProfile);
  swayIdleManager->applyConfig(lockTimeout, screenTimeout, suspendTimeout);
  logindManager.applyConfig(lidActionBattery, lidActionPlugged, powerKeyAction);
}

void Worker::doWork() {
  bool isOnline = batteryManager->readPowerSupplyStatus();
  bool newOnBattery = !isOnline;

  if (newOnBattery != onBattery) {
    onBattery = newOnBattery;
    emit onBatteryChanged(onBattery);
  }
}

void Worker::cleanup() { swayIdleManager->handleSigTerm(SIGTERM); }

void Application::initFromDbus() {
  QDBusConnection systemBus = QDBusConnection::systemBus();
  QDBusInterface upower_iface("org.freedesktop.UPower",
                              "/org/freedesktop/UPower",
                              "org.freedesktop.UPower", systemBus);
  QDBusReply<QList<QDBusObjectPath>> reply =
      upower_iface.call("EnumerateDevices");
  if (reply.isValid()) {
    QList<QDBusObjectPath> devicePaths = reply.value();
    for (const QDBusObjectPath &devicePath : devicePaths) {
      try {
        QDBusInterface device_iface("org.freedesktop.UPower", devicePath.path(),
                                    "org.freedesktop.DBus.Properties",
                                    systemBus);
        QDBusReply<QVariantMap> propsReply =
            device_iface.call("GetAll", "org.freedesktop.UPower.Device");

        if (propsReply.isValid()) {
          QVariantMap properties = propsReply.value();
          if (properties.contains("Type")) {
            if (properties["Type"].toUInt() == 1) {
              if (power_native_path.isEmpty()) {
                power_native_path = properties["NativePath"].toString();
              }
            }
            if (properties["Type"].toUInt() == 2) {
              has_battery = true;
            }
          }
        }
      } catch (const std::exception &e) {
        qWarning() << "  Error: " << e.what();
      }
    }
  }
}

Application::Application(int &argc, char **argv) : QApplication(argc, argv) {
  g_app = this;
  struct sigaction sa;
  sa.sa_handler = termSignalHandler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGTERM, &sa, nullptr) == -1) {
    qDebug() << "Failed to set up SIGTERM handler";
    return;
  }
  if (sigaction(SIGQUIT, &sa, nullptr) == -1) {
    qDebug() << "Failed to set up SIGQUIT handler";
    return;
  }
  if (sigaction(SIGINT, &sa, nullptr) == -1) {
    qDebug() << "Failed to set up SIGINT handler";
    return;
  }
  setQuitOnLastWindowClosed(false);
  connect(this, &Application::appLoaded, this, &Application::onAppLoaded);
  initFromDbus();
  workerThread = new QThread(this);
  worker = new Worker(nullptr, power_native_path, has_battery);
  worker->moveToThread(workerThread);
  connect(workerThread, &QThread::started, worker, &Worker::initialize);
  connect(worker, &Worker::onBatteryChanged, this,
          &Application::powerSourceChanged);
  connect(worker, &Worker::onBatteryChanged, worker,
          &Worker::applyPowerSettings);
  connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);
  connect(worker, &Worker::workerFinished, [this]() {
    // NOTE: Loading steps are: 1. worker initialized, 2. gui loaded
    loadingStep++;
    if (loadingStep == 2) {
      emit appLoaded();
    }
  });
  workerThread->start();

  mainWindow = new QMainWindow(nullptr, Qt::FramelessWindowHint |
                                            Qt::WindowStaysOnTopHint);
  mainWindow->setStyleSheet(
      "QMainWindow { background-color: rgba(18, 18, 18, 192); }");
  this->lockScreenPlugged = new QComboBox();
  this->lockScreenBattery = new QComboBox();
  this->displayPlugged = new QComboBox();
  this->displayBattery = new QComboBox();
  this->sleepPlugged = new QComboBox();
  this->sleepBattery = new QComboBox();
  this->lidClosePlugged = new QComboBox();
  this->lidCloseBattery = new QComboBox();
  this->powerProfilePlugged = new QComboBox();
  this->powerProfileBattery = new QComboBox();
  this->powerKeyPlugged = new QComboBox();
  this->powerKeyBattery = new QComboBox();
  powerProfilePlugged->setEnabled(false);
  powerProfileBattery->setEnabled(false);
  mainWindow->setWindowTitle("Power Settings");
  QWidget *centralWidget = new QWidget();
  centralWidget->setContentsMargins(10, 10, 10, 10);

  QGridLayout *mainLayout = new QGridLayout(centralWidget);
  mainLayout->addWidget(new QLabel(), 0, 0); // Empty corner

  // Header
  // Create header widgets with icons
  QWidget *headerRow = new QWidget();
  headerRow->setContentsMargins(0, 20, 0, 0);
  QHBoxLayout *headerLayout = new QHBoxLayout(headerRow);

  // Plugged in header
  QLabel *pluggedIcon = new QLabel();
  pluggedIcon->setPixmap(
      QPixmap(":/cable.png")
          .scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  pluggedText = new QLabel("Plugged in");
  QHBoxLayout *pluggedHeader = new QHBoxLayout();
  pluggedHeader->setAlignment(Qt::AlignCenter);
  pluggedHeader->addWidget(pluggedText);
  pluggedHeader->addWidget(pluggedIcon);
  pluggedHeader->setSpacing(5);
  headerLayout->addLayout(pluggedHeader);

  batteryIcon = nullptr;
  batteryText = nullptr;
  QHBoxLayout *batteryHeader = nullptr;

  if (has_battery) {
    // Battery header
    batteryIcon = new QLabel();
    batteryIcon->setPixmap(
        QPixmap(":/battery.png")
            .scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    batteryText = new QLabel("On battery");
    batteryHeader = new QHBoxLayout();
    batteryHeader->setAlignment(Qt::AlignCenter);
    batteryHeader->addWidget(batteryText);
    batteryHeader->addWidget(batteryIcon);
    batteryHeader->setSpacing(5);
    headerLayout->addLayout(batteryHeader);
  }

  // Add header row to main layout
  mainLayout->addWidget(headerRow, 0, 1, 1, has_battery ? 2 : 1);

  // Lock screen
  mainLayout->addWidget(new QLabel("Lock screen:"), 1, 0, 1, 1);
  for (const auto &min : timeouts.keys()) {
    lockScreenPlugged->addItem(min);
    if (has_battery) {
      lockScreenBattery->addItem(min);
    }
  }
  mainLayout->addWidget(lockScreenPlugged, 1, 1, 1, 1);
  if (has_battery) {
    mainLayout->addWidget(lockScreenBattery, 1, 2, 1, 1);
  } else {
    mainLayout->addWidget(lockScreenPlugged, 1, 1, 1, 2);
  }

  // Turn off display
  mainLayout->addWidget(new QLabel("Turn off the display:"), 2, 0, 1, 1);
  for (const auto &min : timeouts.keys()) {
    displayPlugged->addItem(min);
    if (has_battery) {
      displayBattery->addItem(min);
    }
  }
  mainLayout->addWidget(displayPlugged, 2, 1, 1, 1);
  if (has_battery) {
    mainLayout->addWidget(displayBattery, 2, 2, 1, 1);
  } else {
    mainLayout->addWidget(displayPlugged, 2, 1, 1, 2);
  }

  // Put to sleep
  mainLayout->addWidget(new QLabel("Put the computer to sleep:"), 3, 0, 1, 1);
  for (const auto &min : timeouts.keys()) {
    sleepPlugged->addItem(min);
    if (has_battery) {
      sleepBattery->addItem(min);
    }
  }
  mainLayout->addWidget(sleepPlugged, 3, 1, 1, 1);
  if (has_battery) {
    mainLayout->addWidget(sleepBattery, 3, 2, 1, 1);
  } else {
    mainLayout->addWidget(sleepPlugged, 3, 1, 1, 2);
  }

  // Lid close action
  mainLayout->addWidget(new QLabel("When I close the lid:"), 4, 0, 1, 1);
  for (const auto &min : logindActions.keys()) {
    lidClosePlugged->addItem(min);
    if (has_battery) {
      lidCloseBattery->addItem(min);
    }
  }
  mainLayout->addWidget(lidClosePlugged, 4, 1, 1, 1);
  if (has_battery) {
    mainLayout->addWidget(lidCloseBattery, 4, 2, 1, 1);
  } else {
    mainLayout->addWidget(lidClosePlugged, 4, 1, 1, 2);
  }

  // Power Key Action
  mainLayout->addWidget(new QLabel("Power Key Action:"), 5, 0, 1, 1);
  for (const auto &action : logindActions.keys()) {
    powerKeyPlugged->addItem(action);
    if (has_battery) {
      powerKeyBattery->addItem(action);
    }
  }
  mainLayout->addWidget(powerKeyPlugged, 5, 1, 1, 1);
  if (has_battery) {
    mainLayout->addWidget(powerKeyBattery, 5, 2, 1, 1);
  } else {
    mainLayout->addWidget(powerKeyPlugged, 5, 1, 1, 2);
  }

  // Power Profiles
  mainLayout->addWidget(new QLabel("Power Profile:"), 6, 0, 1, 1);
  mainLayout->addWidget(powerProfilePlugged, 6, 1, 1, 1);
  if (has_battery) {
    mainLayout->addWidget(powerProfileBattery, 6, 2, 1, 1);
  } else {
    mainLayout->addWidget(powerProfilePlugged, 6, 1, 1, 2);
  }

  // Buttons
  QPushButton *saveBtn = new QPushButton("Save changes");
  QPushButton *cancelBtn = new QPushButton("Cancel");
  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();
  btnLayout->addWidget(saveBtn);
  btnLayout->addWidget(cancelBtn);
  mainLayout->addLayout(btnLayout, 7, 0, 1, 3);

  // Connections
  connect(cancelBtn, &QPushButton::clicked, mainWindow, &QMainWindow::hide);
  // Add your save logic here

  centralWidget->setLayout(mainLayout);
  mainWindow->setCentralWidget(centralWidget);
  mainWindow->resize(400, 200);

  // Create tray icon
  trayIcon = new QSystemTrayIcon(QIcon(":/icon.png"), this);

  // Create context menu
  QMenu *menu = new QMenu();
  QAction *quitAction = new QAction("Exit", this);
  connect(quitAction, &QAction::triggered, this, &QApplication::quit);
  menu->addAction(quitAction);

  trayIcon->setContextMenu(menu);

  connect(trayIcon, &QSystemTrayIcon::activated, this,
          [this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger) {
              mainWindow->setVisible(!mainWindow->isVisible());
            }
          });

  trayIcon->show();
  connect(saveBtn, &QPushButton::clicked, this, &Application::handleSave);
  connect(cancelBtn, &QPushButton::clicked, this, &Application::handleCancel);
  connect(this, &QCoreApplication::aboutToQuit, worker, &Worker::cleanup);
  connect(this, &Application::powerSourceChanged, [this](bool onBattery) {
    QFont font;
    font.setBold(true);
    pluggedText->setFont(onBattery ? QFont() : font);
    if (has_battery) {
      batteryText->setFont(onBattery ? font : QFont());
    }
    mainWindow->setWindowTitle(
        QString("Power Settings - %1").arg(onBattery ? "Battery" : "AC Power"));
  });
  connect(this, &Application::powerProfilesChanged, this,
          &Application::updatePowerProfiles);

  auto profiles = profileManager.getPowerProfiles();
  auto activeProfile = profileManager.getActivePowerProfile();

  emit powerProfilesChanged(profiles, activeProfile);

  loadSettings();
  loadingStep++;
  if (loadingStep == 2) {
    emit appLoaded();
  }
}

Application::~Application() {
  workerThread->quit();
  workerThread->wait();
  g_app = nullptr;
}

QString Application::getSettingsPath() {
  QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" +
      QCoreApplication::applicationName();
  QDir dir(path);
  if (!dir.exists()) {
    dir.mkpath(path);
  }
  QString fileName = path + "/settings.ini";
  return fileName;
}

void Application::updatePowerProfiles(
    const QHash<QString, QString> &profiles,
    const QPair<QString, QString> &activeProfile) {
  powerProfilePlugged->clear();
  powerProfileBattery->clear();

  // Add new profiles to the combo boxes
  for (const auto &profile : profiles) {
    powerProfilePlugged->addItem(profile);
    powerProfileBattery->addItem(profile);
  }

  // Set the active profile
  int index = powerProfilePlugged->findText(activeProfile.second);
  powerProfilePlugged->setCurrentIndex(index);
  powerProfileBattery->setCurrentIndex(index);

  // Enable the combo boxes
  powerProfilePlugged->setEnabled(true);
  powerProfileBattery->setEnabled(true);
}

void Application::handleCancel() { mainWindow->hide(); }
void Application::handleSave() {
  QSettings settings(getSettingsPath(), QSettings::IniFormat);
  settings.setValue("LockScreenPlugged", lockScreenPlugged->currentText());
  settings.setValue("LockScreenBattery", lockScreenBattery->currentText());
  settings.setValue("DisplayPlugged", displayPlugged->currentText());
  settings.setValue("DisplayBattery", displayBattery->currentText());
  settings.setValue("SleepPlugged", sleepPlugged->currentText());
  settings.setValue("SleepBattery", sleepBattery->currentText());
  settings.setValue("LidClosePlugged", lidClosePlugged->currentText());
  settings.setValue("LidCloseBattery", lidCloseBattery->currentText());
  settings.setValue("PowerKeyPlugged", powerKeyPlugged->currentText());
  settings.setValue("PowerKeyBattery", powerKeyBattery->currentText());

  QString profilePluggedDisplayName = powerProfilePlugged->currentText();
  QString profileBatteryDisplayName = powerProfileBattery->currentText();
  QString profilePluggedName =
      profileManager.getCommandNameForProfile(profilePluggedDisplayName);
  QString profileBatteryName =
      profileManager.getCommandNameForProfile(profileBatteryDisplayName);

  settings.setValue("PowerProfilePlugged", profilePluggedName);
  settings.setValue("PowerProfileBattery", profileBatteryName);
  QTimer::singleShot(0, worker, &Worker::applyPowerSettings);
  mainWindow->hide();
}

void Application::loadSettings() {
  QSettings settings(getSettingsPath(), QSettings::IniFormat);
  lockScreenPlugged->setCurrentIndex(lockScreenPlugged->findText(
      settings.value("LockScreenPlugged").toString()));
  lockScreenBattery->setCurrentIndex(lockScreenBattery->findText(
      settings.value("LockScreenBattery").toString()));
  displayPlugged->setCurrentIndex(
      displayPlugged->findText(settings.value("DisplayPlugged").toString()));
  displayBattery->setCurrentIndex(
      displayBattery->findText(settings.value("DisplayBattery").toString()));
  sleepPlugged->setCurrentIndex(
      sleepPlugged->findText(settings.value("SleepPlugged").toString()));
  sleepBattery->setCurrentIndex(
      sleepBattery->findText(settings.value("SleepBattery").toString()));
  lidClosePlugged->setCurrentIndex(
      lidClosePlugged->findText(settings.value("LidClosePlugged").toString()));
  lidCloseBattery->setCurrentIndex(
      lidCloseBattery->findText(settings.value("LidCloseBattery").toString()));
  powerKeyPlugged->setCurrentIndex(
      powerKeyPlugged->findText(settings.value("PowerKeyPlugged").toString()));
  powerKeyBattery->setCurrentIndex(
      powerKeyBattery->findText(settings.value("PowerKeyBattery").toString()));
  auto savedPlugged = profileManager.getDisplayNameForProfile(
      settings.value("PowerProfilePlugged").toString());
  auto savedBattery = profileManager.getDisplayNameForProfile(
      settings.value("PowerProfileBattery").toString());
  powerProfilePlugged->setCurrentIndex(
      powerProfilePlugged->findText(savedPlugged));
  powerProfileBattery->setCurrentIndex(
      powerProfileBattery->findText(savedBattery));
}

void Application::onAppLoaded() {
  QTimer::singleShot(0, worker, &Worker::applyPowerSettings);
  BatteryManager *batteryManager = new BatteryManager(power_native_path);
  bool onBattery = !batteryManager->readPowerSupplyStatus();
  delete batteryManager;
  QFont font;
  font.setBold(true);
  pluggedText->setFont(onBattery ? QFont() : font);
  if (has_battery) {
    batteryText->setFont(onBattery ? font : QFont());
  }
  mainWindow->setWindowTitle(
      QString("Power Settings - %1").arg(onBattery ? "Battery" : "AC Power"));
}

int main(int argc, char *argv[]) {
  Application app(argc, argv);
  return app.exec();
}
