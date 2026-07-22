#include "test_settingsdialog.h"
#include "../src/settings.h"
#include "../src/settingsdialog.h"
#include "../src/traymanager.h"
#include <QApplication>
#include <QMenu>
#include <QTimer>
#include <QtTest>

void SettingsDialogTest::testSettingsSavesValues() {
  TrayManager tray;

  QColor testNoteColor("#FF0000");
  QColor testTextColor("#FFFFFF");
  QFont testFont("Arial", 14);

  QList<QAction *> actions = tray.getTrayIcon()->contextMenu()->actions();

  QTimer::singleShot(500, [testNoteColor, testTextColor, testFont]() {
    SettingsDialog *dialog =
        qobject_cast<SettingsDialog *>(QApplication::activeModalWidget());
    if (dialog) {
      dialog->setNoteColor(testNoteColor);
      dialog->setTextColor(testTextColor);
      dialog->setFont(testFont);
      dialog->accept();
    }
  });

  actions[1]->trigger();

  QCOMPARE(gSettings->value("settings/noteColor").toString(),
           testNoteColor.name());
  QCOMPARE(gSettings->value("settings/textColor").toString(),
           testTextColor.name());

  QFont savedFont = gSettings->value("settings/font").value<QFont>();
  QCOMPARE(savedFont.family(), testFont.family());
  QCOMPARE(savedFont.pointSize(), testFont.pointSize());
}
