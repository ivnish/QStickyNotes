#ifndef TEST_SETTINGSDIALOG_H
#define TEST_SETTINGSDIALOG_H

#include <QObject>

class SettingsDialogTest : public QObject {
  Q_OBJECT

private slots:
  void testSettingsSavesValues();
};

#endif
