#include "traymanager.h"
#include "settings.h"
#include "settingsdialog.h"
#include "stickynotewidget.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QStandardPaths>

void TrayManager::loadNotes() {
  QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir(dirPath);
  QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files);
  for (const QString &fileName : files) {
    QString filePath = dirPath + "/" + fileName;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
      continue;
    }
    QByteArray data = file.readAll();
    file.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();
    QString id = obj["id"].toString();

    StickyNoteWidget *note = new StickyNoteWidget(id);
    note->loadFromJson(obj);
    note->show();
    note->m_isLoading = false;
    connect(note, &StickyNoteWidget::requestNewNote, this, &TrayManager::createNote);
  }
}

TrayManager::TrayManager(QObject *parent) : QObject(parent), trayIcon(nullptr), trayMenu(nullptr) {
  // Create menu.
  trayMenu = new QMenu();
  newNoteAction = new QAction(tr("New note"), this);
  settingsAction = new QAction(tr("Settings"), this);
  quitAction = new QAction(tr("Quit"), this);
  aboutAction = new QAction(tr("About"), this);
  connect(newNoteAction, &QAction::triggered, this, &TrayManager::onNewNote);
  connect(settingsAction, &QAction::triggered, this, &TrayManager::onSettings);
  connect(quitAction, &QAction::triggered, this, &TrayManager::onQuit);
  connect(aboutAction, &QAction::triggered, this, &TrayManager::onAbout);
  trayMenu->addAction(newNoteAction);
  trayMenu->addAction(settingsAction);
  trayMenu->addAction(aboutAction);
  trayMenu->addAction(quitAction);

  // Create tray icon.
  trayIcon = new QSystemTrayIcon(this);
  QString path = QString(APP_INSTALL_PREFIX) + "/share/QStickyNotes/Icons/QStickyNotes.png";
  if (QFile::exists(path)) {
    trayIcon->setIcon(QIcon(path));
  } else {
    trayIcon->setIcon(QIcon::fromTheme("notes"));
  }
  trayIcon->setContextMenu(trayMenu);
  trayIcon->setToolTip("QStickyNotes");
  trayIcon->show();

  loadNotes();
}

TrayManager::~TrayManager() {
  if (trayIcon) {
    trayIcon->hide();
  }
}

void TrayManager::createNote() {
  StickyNoteWidget *note = new StickyNoteWidget();
  note->move(0, 0);
  note->show();
  connect(note, &StickyNoteWidget::requestNewNote, this, &TrayManager::createNote);
}

void TrayManager::onNewNote() { createNote(); }

void TrayManager::onSettings() {
  SettingsDialog dialog;

  if (dialog.exec() == QDialog::Accepted) {
    gSettings->setValue("settings/noteColor", dialog.noteColor().name());
    gSettings->setValue("settings/textColor", dialog.textColor().name());
    gSettings->setValue("settings/font", dialog.noteFont());
    gSettings->sync();
  }
}

void TrayManager::onQuit() { QApplication::quit(); }

void TrayManager::onAbout() {
  QString title = "QStickyNotes 0.1";
  QString desc = tr("Lightweight sticky notes application inspired by indicator-stickynotes.");
  QString built = tr("Built with Qt") + " " + QString(qVersion());
  QString projectPage = tr("Project page") + ": <a href=\"https://github.com/ivnish/QStickyNotes\">GitHub</a></p>";
  QString copyright = "Copyright 2026 Andrei 'ivnish' Ivnitskii";

  QString text = QString("<p><b>%1</b></p>"
                         "<p>%2</p>"
                         "<p>%3</p>"
                         "<p>%4</p>"
                         "<p>%5</p>")
                     .arg(title, desc, built, projectPage, copyright);

  QMessageBox::about(nullptr, tr("About"), text);
}

QSystemTrayIcon *TrayManager::getTrayIcon() const { return trayIcon; }
