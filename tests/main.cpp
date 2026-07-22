#include <QApplication>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QtTest>
#include <unistd.h>

#include "../src/settings.h"
#include "test_settingsdialog.h"
#include "test_stickynote.h"
#include "test_traymanager.h"

int main(int argc, char *argv[]) {
  QString testDir =
      QString("/tmp/qstickynotes_test_%1").arg(QString::number(getuid()));
  QDir().mkpath(testDir + "/data");
  QDir().mkpath(testDir + "/config");
  qputenv("XDG_DATA_HOME", (testDir + "/data").toUtf8());
  qputenv("XDG_CONFIG_HOME", (testDir + "/config").toUtf8());

  QApplication app(argc, argv);
  QApplication::setQuitOnLastWindowClosed(false);

  QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

  gSettings = new QSettings(testDir + "/config/config.ini", QSettings::IniFormat);

  int status = 0;

  {
    TrayManagerTest tc;
    status |= QTest::qExec(&tc, argc, argv);
  }
  {
    SettingsDialogTest tc;
    status |= QTest::qExec(&tc, argc, argv);
  }
  {
    StickyNoteTest tc;
    status |= QTest::qExec(&tc, argc, argv);
  }

  delete gSettings;
  return status;
}
