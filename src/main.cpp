#include "settings.h"
#include "traymanager.h"

#include <QApplication>
#include <QDir>
#include <QLocale>
#include <QStandardPaths>
#include <QTranslator>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  QApplication::setApplicationName("QStickyNotes");

  QApplication::setQuitOnLastWindowClosed(false);

  // Config directory.
  QString configDirPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/QStickyNotes";
  QDir configDir(configDirPath);
  if (!configDir.exists()) {
    configDir.mkpath(".");
  }
  QString configPath = configDirPath + "/config.ini";
  gSettings = new QSettings(configPath, QSettings::IniFormat);
  // Write default settings if config not exists.
  if (!QFile::exists(configPath)) {
    gSettings->setValue("settings/noteColor", "#FFFBB2");
    gSettings->setValue("settings/textColor", "#000000");
    QFont defaultFont("Noto Sans", 10);
    defaultFont.setStyleName("Regular");
    gSettings->setValue("settings/font", defaultFont);
    gSettings->sync();
  }

  // Notes directory.
  QString notesDirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir notesDir(notesDirPath);
  if (!notesDir.exists()) {
    notesDir.mkpath(".");
  }

  QTranslator *translator = new QTranslator(&a);
  QString baseName = "qstickynotes_" + QLocale::system().name();
  QString path = "/usr/share/QStickyNotes/translations";
  if (translator->load(baseName, path)) {
    a.installTranslator(translator);
  }

  TrayManager tray;

  return a.exec();
}
