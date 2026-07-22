#ifndef TEST_TRAYMANAGER_H
#define TEST_TRAYMANAGER_H

#include <QObject>

class TrayManagerTest : public QObject {
  Q_OBJECT

private slots:
  void testTrayIconCreated();
  void testMenuActionCount();
  void testMenuActionTexts();
  void testMenuActionNewNoteCreatesWidget();
  void testMenuActionSettingsOpensDialog();
  void testMenuActionAboutOpensDialog();
};

#endif
