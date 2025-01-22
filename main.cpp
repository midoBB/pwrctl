#include "main.hpp"

Application::Application(int &argc, char **argv) : QApplication(argc, argv) {
  setQuitOnLastWindowClosed(false);
  mainWindow = new QMainWindow();
  this->displayPlugged = new QComboBox();
  this->displayBattery = new QComboBox();
  this->sleepPlugged = new QComboBox();
  this->sleepBattery = new QComboBox();
  this->lidClosePlugged = new QComboBox();
  this->lidCloseBattery = new QComboBox();
  mainWindow->setWindowTitle("Power Settings");
  QWidget *centralWidget = new QWidget();
  QGridLayout *mainLayout = new QGridLayout(centralWidget);
  mainLayout->addWidget(new QLabel(), 0, 0); // Empty corner
  // Header
  // Create header widgets with icons
  QWidget *headerRow = new QWidget();
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

  // Buttons
  QPushButton *saveBtn = new QPushButton("Save changes");
  QPushButton *cancelBtn = new QPushButton("Cancel");
  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnLayout->addStretch();
  btnLayout->addWidget(saveBtn);
  btnLayout->addWidget(cancelBtn);
  mainLayout->addLayout(btnLayout, 4, 0, 1, 3);

  // Connections
  connect(cancelBtn, &QPushButton::clicked, mainWindow, &QMainWindow::hide);
  // Add your save logic here

  centralWidget->setLayout(mainLayout);
  mainWindow->setCentralWidget(centralWidget);
  mainWindow->resize(400, 200);
  mainWindow->hide();

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
}

void Application::handleCancel() {
  mainWindow->hide();
}
void Application::handleSave() {
    qDebug() << "Display Plugged Index:" << displayPlugged->currentIndex();
    qDebug() << "Display Battery Index:" << displayBattery->currentIndex();
    qDebug() << "Sleep Plugged Index:" << sleepPlugged->currentIndex();
    qDebug() << "Sleep Battery Index:" << sleepBattery->currentIndex();
    qDebug() << "Lid Close Plugged Index:" << lidClosePlugged->currentIndex();
    qDebug() << "Lid Close Battery Index:" << lidCloseBattery->currentIndex();

    // Optional: Also show the text values
    qDebug() << "Values:";
    qDebug() << " - Plugged display:" << displayPlugged->currentText();
    qDebug() << " - Battery display:" << displayBattery->currentText();
    qDebug() << " - Plugged sleep:" << sleepPlugged->currentText();
    qDebug() << " - Battery sleep:" << sleepBattery->currentText();
    qDebug() << " - Lid action Plugged:" << lidClosePlugged->currentText();
    qDebug() << " - Lid action Battery:" << lidCloseBattery->currentText();

    mainWindow->hide();
}
int main(int argc, char *argv[]) {
  Application app(argc, argv);
  return app.exec();
}
