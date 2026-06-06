#pragma once

#include "config.h"

#include <QAction>
#include <QMenu>
#include <QObject>
#include <QSystemTrayIcon>

class TrayManager : public QObject {
  Q_OBJECT

public:
  explicit TrayManager(QObject *parent = nullptr);
  ~TrayManager();
  QSystemTrayIcon *getTrayIcon() const;

private:
  void createNote();
  QSystemTrayIcon *trayIcon;
  QMenu *trayMenu;
  QAction *newNoteAction;
  QAction *settingsAction;
  QAction *quitAction;
  void loadNotes();
  QAction *aboutAction;

private slots:
  void onNewNote();
  void onSettings();
  void onQuit();
  void onAbout();
};
