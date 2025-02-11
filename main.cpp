#include "main.hpp"
#include "QOrderedMap.h"
#include "batterymanager.hpp"
#include "powerprofile.hpp"
#include <cstdint>

static const OrderedMap<QString, int16_t> timeouts = {
    {"2 minutes", 120},  {"5 minutes", 300},   {"10 minutes", 600},
    {"15 minutes", 900}, {"30 minutes", 1800}, {"1 hour", 3600},
    {"Never", -1},
};

Worker::Worker(QObject *parent) : QObject(parent) {
  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &Worker::doWork);
  timer->setInterval(5000);
  BatteryManager batteryManager;
  onBattery = !batteryManager.readPowerSupplyStatus();
}

void Worker::initialize() {
  timer->start();
  swayIdleManager.parseConfig();
  emit workerFinished();
}

Worker::~Worker() { timer->stop(); }

void Worker::applyPowerSettings() {
  qDebug() << "applyPowerSettings called, onBattery:" << onBattery;
  QString settingsPath =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" +
      QCoreApplication::applicationName() + "/settings.ini";
  QSettings settings(settingsPath, QSettings::IniFormat);
  QString lockScreenKey = onBattery ? "LockScreenBattery" : "LockScreenPlugged";
  QString displayKey = onBattery ? "DisplayBattery" : "DisplayPlugged";
  QString sleepKey = onBattery ? "SleepBattery" : "SleepPlugged";
  QString lidCloseKey = onBattery ? "LidCloseBattery" : "LidClosePlugged";
  QString powerKeyKey = onBattery ? "PowerKeyBattery" : "PowerKeyPlugged";
  QString powerProfileKey =
      onBattery ? "PowerProfileBattery" : "PowerProfilePlugged";
  int16_t lockTimeout =
      timeouts.value(settings.value(lockScreenKey).toString());
  int16_t screenTimeout = timeouts.value(settings.value(displayKey).toString());
  qDebug() << sleepKey << ": " << settings.value(sleepKey).toString();
  qDebug() << lidCloseKey << ": " << settings.value(lidCloseKey).toString();
  qDebug() << powerKeyKey << ": " << settings.value(powerKeyKey).toString();

  QString powerProfile = settings.value(powerProfileKey).toString();
  PowerProfileManager profileManager;
  profileManager.applyPowerProfile(powerProfile);
  swayIdleManager.applyConfig(lockTimeout, screenTimeout);
}

void Worker::doWork() {
  BatteryManager batteryManager;
  bool isOnline = batteryManager.readPowerSupplyStatus();
  bool newOnBattery = !isOnline;

  if (newOnBattery != onBattery) {
    onBattery = newOnBattery;
    emit onBatteryChanged(onBattery);
  }
}

Application::Application(int &argc, char **argv) : QApplication(argc, argv) {
  setQuitOnLastWindowClosed(false);
  connect(this, &Application::appLoaded, this, &Application::onAppLoaded);
  workerThread = new QThread(this);
  worker = new Worker();
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

  mainWindow = new QMainWindow();
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
  centralWidget->setStyleSheet("background-color: transparent;");
  centralWidget->setContentsMargins(10, 10, 10, 10);

  QGraphicsDropShadowEffect *shadowEffect =
      new QGraphicsDropShadowEffect(centralWidget);
  shadowEffect->setBlurRadius(10);
  shadowEffect->setColor(QColor(0, 0, 0, 80));
  shadowEffect->setOffset(0);
  centralWidget->setGraphicsEffect(shadowEffect);

  QGridLayout *mainLayout = new QGridLayout(centralWidget);
  mainLayout->addWidget(new QLabel(), 0, 0); // Empty corner

  // Header
  // Create header widgets with icons
  QWidget *headerRow = new QWidget();
  headerRow->setContentsMargins(0, 20, 0, 0);
  QHBoxLayout *headerLayout = new QHBoxLayout(headerRow);

  // Empty spacer for first column
  QLabel *pluggedIcon = new QLabel();
  pluggedIcon->setPixmap(
      QPixmap(":/cable.png")
          .scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  QLabel *pluggedText = new QLabel("Plugged in");
  QHBoxLayout *pluggedHeader = new QHBoxLayout();
  pluggedHeader->setAlignment(Qt::AlignCenter);
  pluggedHeader->addWidget(pluggedText);
  pluggedHeader->addWidget(pluggedIcon);
  pluggedHeader->setSpacing(5);
  headerLayout->addLayout(pluggedHeader);

  // Battery header
  QLabel *batteryIcon = new QLabel();
  batteryIcon->setPixmap(
      QPixmap(":/battery.png")
          .scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  QLabel *batteryText = new QLabel("On battery");
  QHBoxLayout *batteryHeader = new QHBoxLayout();
  batteryHeader->setAlignment(Qt::AlignCenter);
  batteryHeader->addWidget(batteryText);
  batteryHeader->addWidget(batteryIcon);
  batteryHeader->setSpacing(5);
  headerLayout->addLayout(batteryHeader);

  // Add header row to main layout
  mainLayout->addWidget(headerRow, 0, 1, 1, 2); // Span all columns
  // Lock screen
  mainLayout->addWidget(new QLabel("Lock screen:"), 1, 0);
  for (const auto &min : timeouts.keys()) {
    lockScreenPlugged->addItem(min);
    lockScreenBattery->addItem(min);
  }
  mainLayout->addWidget(lockScreenPlugged, 1, 1);
  mainLayout->addWidget(lockScreenBattery, 1, 2);
  // Turn off display
  mainLayout->addWidget(new QLabel("Turn off the display:"), 2, 0);
  for (const auto &min : timeouts.keys()) {
    displayPlugged->addItem(min);
    displayBattery->addItem(min);
  }
  mainLayout->addWidget(displayPlugged, 2, 1);
  mainLayout->addWidget(displayBattery, 2, 2);

  // Put to sleep
  mainLayout->addWidget(new QLabel("Put the computer to sleep:"), 3, 0);
  for (const auto &min : timeouts.keys()) {
    sleepPlugged->addItem(min);
    sleepBattery->addItem(min);
  }
  mainLayout->addWidget(sleepPlugged, 3, 1);
  mainLayout->addWidget(sleepBattery, 3, 2);

  // Lid close action
  mainLayout->addWidget(new QLabel("When I close the lid:"), 4, 0);
  QStringList lidCloseActions = {"Do Nothing", "Sleep", "Poweroff"};
  for (const auto &min : lidCloseActions) {
    lidClosePlugged->addItem(min);
    lidCloseBattery->addItem(min);
  }
  mainLayout->addWidget(lidClosePlugged, 4, 1);
  mainLayout->addWidget(lidCloseBattery, 4, 2);

  // Power Key Action
  mainLayout->addWidget(new QLabel("Power Key Action:"), 5, 0);
  QStringList powerKeyActions = {"Ignore",  "Poweroff",  "Reboot",
                                 "Suspend", "Hibernate", "Hybrid Sleep",
                                 "Lock"};
  for (const auto &action : powerKeyActions) {
    powerKeyPlugged->addItem(action);
    powerKeyBattery->addItem(action);
  }
  mainLayout->addWidget(powerKeyPlugged, 5, 1);
  mainLayout->addWidget(powerKeyBattery, 5, 2);

  // Power Profiles
  mainLayout->addWidget(new QLabel("Power Profile:"), 6, 0);
  mainLayout->addWidget(powerProfilePlugged, 6, 1);
  mainLayout->addWidget(powerProfileBattery, 6, 2);

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
  connect(this, &Application::powerSourceChanged, [this](bool onBattery) {
    QFont font;
    font.setBold(true);

    displayPlugged->setFont(onBattery ? QFont() : font);
    displayBattery->setFont(onBattery ? font : QFont());

    // Or show current state in window title
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
}

int main(int argc, char *argv[]) {
  Application app(argc, argv);
  return app.exec();
}
