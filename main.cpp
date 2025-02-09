#include "main.hpp"
#include "powerprofile.hpp"
#include "batterymanager.hpp"

Worker::Worker(QObject *parent) : QObject(parent) {
  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &Worker::doWork);
  timer->setInterval(5000);
  BatteryManager batteryManager;
  onBattery = !batteryManager.readPowerSupplyStatus();
}

void Worker::initialize() {
  PowerProfileManager profileManager;
  QStringList profiles = profileManager.getPowerProfiles();
  QString activeProfile = profileManager.getActivePowerProfile();

  emit powerProfilesChanged(profiles, activeProfile);
  timer->start();
  emit workerFinished();
}

Worker::~Worker() {
  timer->stop();
}

void Worker::applyPowerSettings() {
  qDebug() << "applyPowerSettings called, onBattery:" << onBattery;
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

  workerThread = new QThread(this);
  worker = new Worker();
  worker->moveToThread(workerThread);
  connect(workerThread, &QThread::started, worker, &Worker::initialize);
  connect(worker, &Worker::onBatteryChanged, this,
          &Application::powerSourceChanged);
  connect(worker, &Worker::onBatteryChanged, worker, &Worker::applyPowerSettings);
  connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);

  workerThread->start();

  mainWindow = new QMainWindow();
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

  // Turn off display
  mainLayout->addWidget(new QLabel("Turn off the display:"), 1, 0);
  for (const auto &min :
       {"2 minutes", "5 minutes", "10 minutes", "15 minutes"}) {
    displayPlugged->addItem(min);
    displayBattery->addItem(min);
  }
  mainLayout->addWidget(displayPlugged, 1, 1);
  mainLayout->addWidget(displayBattery, 1, 2);

  // Put to sleep
  mainLayout->addWidget(new QLabel("Put the computer to sleep:"), 2, 0);
  for (const auto &min :
       {"2 minutes", "5 minutes", "10 minutes", "15 minutes"}) {
    sleepPlugged->addItem(min);
    sleepBattery->addItem(min);
  }
  mainLayout->addWidget(sleepPlugged, 2, 1);
  mainLayout->addWidget(sleepBattery, 2, 2);

  // Lid close action
  mainLayout->addWidget(new QLabel("When I close the lid:"), 3, 0);
  for (const auto &min : {"Do nothing", "Sleep", "Shutdown"}) {
    lidClosePlugged->addItem(min);
    lidCloseBattery->addItem(min);
  }
  mainLayout->addWidget(lidClosePlugged, 3, 1);
  mainLayout->addWidget(lidCloseBattery, 3, 2);

  // Power Key Action
  mainLayout->addWidget(new QLabel("Power Key Action:"), 4, 0);
  QStringList powerKeyActions = {"Ignore",  "Poweroff",  "Reboot",
                                 "Suspend", "Hibernate", "Hybrid Sleep",
                                 "Lock"};
  for (const auto &action : powerKeyActions) {
    powerKeyPlugged->addItem(action);
    powerKeyBattery->addItem(action);
  }
  mainLayout->addWidget(powerKeyPlugged, 4, 1);
  mainLayout->addWidget(powerKeyBattery, 4, 2);

  // Power Profiles
  mainLayout->addWidget(new QLabel("Power Profile:"), 5, 0);
  mainLayout->addWidget(powerProfilePlugged, 5, 1);
  mainLayout->addWidget(powerProfileBattery, 5, 2);

  // Buttons
  QPushButton *saveBtn = new QPushButton("Save changes");
  QPushButton *cancelBtn = new QPushButton("Cancel");
  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();
  btnLayout->addWidget(saveBtn);
  btnLayout->addWidget(cancelBtn);
  mainLayout->addLayout(btnLayout, 6, 0, 1, 3);

  // Connections
  connect(cancelBtn, &QPushButton::clicked, mainWindow, &QMainWindow::hide);
  // Add your save logic here

  centralWidget->setLayout(mainLayout);
  mainWindow->setCentralWidget(centralWidget);
  mainWindow->resize(400, 200);
  // mainWindow->hide();

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
  loadSettings();
  connect(worker, &Worker::powerProfilesChanged, this,
          &Application::updatePowerProfiles);
  connect(worker, &Worker::workerFinished, [this]() {
    if (guiLoaded) {
      emit appLoaded();
    }
  });
  connect(this, &Application::appLoaded, this, &Application::onAppLoaded);
  guiLoaded = true;
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

void Application::updatePowerProfiles(const QStringList &profiles,
                                      const QString &activeProfile) {
  // Clear existing items
  powerProfilePlugged->clear();
  powerProfileBattery->clear();

  // Add new profiles to the combo boxes
  powerProfilePlugged->addItems(profiles);
  powerProfileBattery->addItems(profiles);

  // Set the active profile
  int index = profiles.indexOf(activeProfile);
  powerProfilePlugged->setCurrentIndex(index);
  powerProfileBattery->setCurrentIndex(index);

  // Enable the combo boxes
  powerProfilePlugged->setEnabled(true);
  powerProfileBattery->setEnabled(true);
  if (!m_savedPowerProfilePlugged.isEmpty()) {
    int idx = powerProfilePlugged->findText(m_savedPowerProfilePlugged);
    if (idx != -1)
      powerProfilePlugged->setCurrentIndex(idx);
    m_savedPowerProfilePlugged.clear();
  }

  if (!m_savedPowerProfileBattery.isEmpty()) {
    int idx = powerProfileBattery->findText(m_savedPowerProfileBattery);
    if (idx != -1)
      powerProfileBattery->setCurrentIndex(idx);
    m_savedPowerProfileBattery.clear();
  }
}

void Application::handleCancel() { mainWindow->hide(); }
void Application::handleSave() {
  QSettings settings(getSettingsPath(), QSettings::IniFormat);

  settings.setValue("DisplayPlugged", displayPlugged->currentText());
  settings.setValue("DisplayBattery", displayBattery->currentText());
  settings.setValue("SleepPlugged", sleepPlugged->currentText());
  settings.setValue("SleepBattery", sleepBattery->currentText());
  settings.setValue("LidClosePlugged", lidClosePlugged->currentText());
  settings.setValue("LidCloseBattery", lidCloseBattery->currentText());
  settings.setValue("PowerKeyPlugged", powerKeyPlugged->currentText());
  settings.setValue("PowerKeyBattery", powerKeyBattery->currentText());
  settings.setValue("PowerProfilePlugged", powerProfilePlugged->currentText());
  settings.setValue("PowerProfileBattery", powerProfileBattery->currentText());
  mainWindow->hide();
}

void Application::loadSettings() {
  QSettings settings(getSettingsPath(), QSettings::IniFormat);

  // Restore non-power-profile settings
  loadComboSetting(displayPlugged, "DisplayPlugged");
  loadComboSetting(displayBattery, "DisplayBattery");
  loadComboSetting(sleepPlugged, "SleepPlugged");
  loadComboSetting(sleepBattery, "SleepBattery");
  loadComboSetting(lidClosePlugged, "LidClosePlugged");
  loadComboSetting(lidCloseBattery, "LidCloseBattery");
  loadComboSetting(powerKeyPlugged, "PowerKeyPlugged");
  loadComboSetting(powerKeyBattery, "PowerKeyBattery");

  // Store power profile settings to apply later
  m_savedPowerProfilePlugged = settings.value("PowerProfilePlugged").toString();
  m_savedPowerProfileBattery = settings.value("PowerProfileBattery").toString();
}

void Application::onAppLoaded() {
  QTimer::singleShot(0, worker, &Worker::applyPowerSettings);
}

// Helper function to load a QComboBox setting
void Application::loadComboSetting(QComboBox *combo, const QString &key) {
  QSettings settings;
  QString value = settings.value(key).toString();
  if (!value.isEmpty()) {
    int index = combo->findText(value);
    if (index != -1)
      combo->setCurrentIndex(index);
  }
}
int main(int argc, char *argv[]) {
  Application app(argc, argv);
  return app.exec();
}
