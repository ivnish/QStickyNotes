#include "test_traymanager.h"
#include "../src/traymanager.h"
#include <QApplication>
#include <QMenu>
#include <QTimer>
#include <QtTest>

void TrayManagerTest::testTrayIconCreated() {
  TrayManager tray;

  QVERIFY2(tray.getTrayIcon() != nullptr, "Tray icon was not created");

  QVERIFY2(tray.getTrayIcon()->isVisible(), "Tray icon is not visible");

  QVERIFY2(!tray.getTrayIcon()->icon().isNull(), "Tray icon image is null");
}

void TrayManagerTest::testMenuActionCount() {
  TrayManager tray;
  QMenu *menu = tray.getTrayIcon()->contextMenu();

  QVERIFY2(menu != nullptr, "Context menu is null");
  QCOMPARE(menu->actions().size(), 4);
}

void TrayManagerTest::testMenuActionTexts() {
  TrayManager tray;
  QList<QAction *> actions = tray.getTrayIcon()->contextMenu()->actions();

  QCOMPARE(actions[0]->text(), QString("New note"));
  QCOMPARE(actions[1]->text(), QString("Settings"));
  QCOMPARE(actions[2]->text(), QString("About"));
  QCOMPARE(actions[3]->text(), QString("Quit"));
}

void TrayManagerTest::testMenuActionNewNoteCreatesWidget() {
  TrayManager tray;
  int initialCount = QApplication::topLevelWidgets().count();

  QList<QAction *> actions = tray.getTrayIcon()->contextMenu()->actions();
  actions[0]->trigger();

  QCOMPARE(QApplication::topLevelWidgets().count(), initialCount + 1);
}

void TrayManagerTest::testMenuActionSettingsOpensDialog() {
  TrayManager tray;
  bool dialogOpened = false;

  QList<QAction *> actions = tray.getTrayIcon()->contextMenu()->actions();

  QTimer::singleShot(500, [&dialogOpened]() {
    QWidget *dialog = QApplication::activeModalWidget();
    if (dialog) {
      dialogOpened = true;
      dialog->close();
    }
  });

  actions[1]->trigger();
  QVERIFY(dialogOpened);
}

void TrayManagerTest::testMenuActionAboutOpensDialog() {
  TrayManager tray;
  bool dialogOpened = false;

  QList<QAction *> actions = tray.getTrayIcon()->contextMenu()->actions();

  QTimer::singleShot(500, [&dialogOpened]() {
    QWidget *dialog = QApplication::activeModalWidget();
    if (dialog) {
      dialogOpened = true;
      dialog->close();
    }
  });

  actions[2]->trigger();
  QVERIFY(dialogOpened);
}
