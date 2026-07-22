#include "test_stickynote.h"
#include "../src/settingsdialog.h"
#include "../src/stickynotewidget.h"
#include "../src/traymanager.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFocusEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QPushButton>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextEdit>
#include <QtTest>

static void cleanupNotes() {
  for (QWidget *w : QApplication::topLevelWidgets()) {
    if (qobject_cast<StickyNoteWidget *>(w))
      delete w;
  }
  QApplication::processEvents();
}

void StickyNoteTest::testNewNoteSavesToFile() {
  cleanupNotes();

  QString notesDir =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir(notesDir);

  // Clean leftover files from previous runs.
  QStringList oldFiles = dir.entryList(QStringList() << "*.json", QDir::Files);
  for (const QString &f : oldFiles)
    QFile::remove(dir.filePath(f));

  TrayManager tray;
  int initialCount =
      dir.entryList(QStringList() << "*.json", QDir::Files).count();

  QList<QAction *> actions = tray.getTrayIcon()->contextMenu()->actions();
  actions[0]->trigger();

  StickyNoteWidget *note = nullptr;
  for (QWidget *w : QApplication::topLevelWidgets()) {
    note = qobject_cast<StickyNoteWidget *>(w);
    if (note)
      break;
  }
  QVERIFY(note != nullptr);

  QTextEdit *textEdit = note->findChild<QTextEdit *>();
  QVERIFY(textEdit != nullptr);

  note->m_isLoading = false;
  textEdit->setPlainText("Test note text");
  QApplication::processEvents();

  QFocusEvent focusOut(QEvent::FocusOut);
  QApplication::sendEvent(textEdit, &focusOut);
  QApplication::processEvents();

  QCOMPARE(dir.entryList(QStringList() << "*.json", QDir::Files).count(),
           initialCount + 1);

  QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files);
  bool found = false;
  for (const QString &fileName : files) {
    QFile f(notesDir + "/" + fileName);
    f.open(QIODevice::ReadOnly);
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (doc.object()["text"].toString() == "Test note text") {
      found = true;
      break;
    }
  }
  QVERIFY(found);
}

void StickyNoteTest::testNoteSettingsSavesToFile() {
  cleanupNotes();

  QString notesDir =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir(notesDir);

  QStringList oldFiles = dir.entryList(QStringList() << "*.json", QDir::Files);
  for (const QString &f : oldFiles)
    QFile::remove(dir.filePath(f));

  TrayManager tray;

  QList<QAction *> actions = tray.getTrayIcon()->contextMenu()->actions();
  actions[0]->trigger();

  StickyNoteWidget *note = nullptr;
  for (QWidget *w : QApplication::topLevelWidgets()) {
    note = qobject_cast<StickyNoteWidget *>(w);
    if (note)
      break;
  }
  QVERIFY(note != nullptr);

  QTextEdit *textEdit = note->findChild<QTextEdit *>();
  QVERIFY(textEdit != nullptr);

  note->m_isLoading = false;
  textEdit->setPlainText("Settings test note");
  QApplication::processEvents();

  QFocusEvent focusOut(QEvent::FocusOut);
  QApplication::sendEvent(textEdit, &focusOut);
  QApplication::processEvents();

  QPushButton *settingsButton = nullptr;
  for (QPushButton *btn : note->findChildren<QPushButton *>()) {
    bool wasSettingsDialog = false;
    QTimer::singleShot(100, [&wasSettingsDialog]() {
      QWidget *w = QApplication::activeModalWidget();
      if (qobject_cast<SettingsDialog *>(w))
        wasSettingsDialog = true;
      if (w)
        w->close();
    });
    btn->click();
    QApplication::processEvents();
    if (wasSettingsDialog) {
      settingsButton = btn;
      break;
    }
  }
  QVERIFY(settingsButton != nullptr);

  QColor testNoteColor("#00FF00");
  QColor testTextColor("#FF00FF");
  QFont testFont("Courier", 12);

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

  settingsButton->click();
  QApplication::processEvents();

  QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files);
  QCOMPARE(files.count(), 1);

  QFile f(notesDir + "/" + files.first());
  f.open(QIODevice::ReadOnly);
  QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
  f.close();

  QJsonObject obj = doc.object();
  QCOMPARE(obj["noteColor"].toString(), testNoteColor.name());
  QCOMPARE(obj["textColor"].toString(), testTextColor.name());
  QCOMPARE(obj["text"].toString(), QString("Settings test note"));

  QFont savedFont;
  savedFont.fromString(obj["font"].toString());
  QCOMPARE(savedFont.family(), testFont.family());
  QCOMPARE(savedFont.pointSize(), testFont.pointSize());
}

void StickyNoteTest::testNoteSavesOnFocusOut() {
  cleanupNotes();

  QString notesDir =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir(notesDir);
  for (const QString &f : dir.entryList(QStringList() << "*.json", QDir::Files))
    QFile::remove(dir.filePath(f));

  TrayManager tray;
  QList<QAction *> actions = tray.getTrayIcon()->contextMenu()->actions();
  actions[0]->trigger();

  StickyNoteWidget *note = nullptr;
  for (QWidget *w : QApplication::topLevelWidgets()) {
    note = qobject_cast<StickyNoteWidget *>(w);
    if (note)
      break;
  }
  QVERIFY(note != nullptr);

  QTextEdit *textEdit = note->findChild<QTextEdit *>();
  QVERIFY(textEdit != nullptr);
  note->m_isLoading = false;

  textEdit->setPlainText("Original");
  QApplication::processEvents();
  QFocusEvent focusOut1(QEvent::FocusOut);
  QApplication::sendEvent(textEdit, &focusOut1);
  QApplication::processEvents();

  QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files);
  QCOMPARE(files.count(), 1);
  QFile f1(notesDir + "/" + files.first());
  f1.open(QIODevice::ReadOnly);
  QJsonDocument doc1 = QJsonDocument::fromJson(f1.readAll());
  f1.close();
  QCOMPARE(doc1.object()["text"].toString(), QString("Original"));

  textEdit->setPlainText("Modified");
  QApplication::processEvents();
  QFocusEvent focusOut2(QEvent::FocusOut);
  QApplication::sendEvent(textEdit, &focusOut2);
  QApplication::processEvents();

  QFile f2(notesDir + "/" + files.first());
  f2.open(QIODevice::ReadOnly);
  QJsonDocument doc2 = QJsonDocument::fromJson(f2.readAll());
  f2.close();
  QCOMPARE(doc2.object()["text"].toString(), QString("Modified"));
}

void StickyNoteTest::testNoteSavesOnMove() {
  cleanupNotes();

  QString notesDir =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir(notesDir);
  for (const QString &f : dir.entryList(QStringList() << "*.json", QDir::Files))
    QFile::remove(dir.filePath(f));

  TrayManager tray;
  QList<QAction *> actions = tray.getTrayIcon()->contextMenu()->actions();
  actions[0]->trigger();

  StickyNoteWidget *note = nullptr;
  for (QWidget *w : QApplication::topLevelWidgets()) {
    note = qobject_cast<StickyNoteWidget *>(w);
    if (note)
      break;
  }
  QVERIFY(note != nullptr);

  QTextEdit *textEdit = note->findChild<QTextEdit *>();
  note->m_isLoading = false;
  textEdit->setPlainText("Move test");
  QApplication::processEvents();
  QFocusEvent focusOut(QEvent::FocusOut);
  QApplication::sendEvent(textEdit, &focusOut);
  QApplication::processEvents();

  QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files);
  QCOMPARE(files.count(), 1);
  QFile f1(notesDir + "/" + files.first());
  f1.open(QIODevice::ReadOnly);
  QJsonDocument doc1 = QJsonDocument::fromJson(f1.readAll());
  f1.close();
  QCOMPARE(doc1.object()["x"].toInt(), 0);
  QCOMPARE(doc1.object()["y"].toInt(), 0);

  note->move(100, 200);
  QTest::qWait(1500);

  QFile f2(notesDir + "/" + files.first());
  f2.open(QIODevice::ReadOnly);
  QJsonDocument doc2 = QJsonDocument::fromJson(f2.readAll());
  f2.close();
  QCOMPARE(doc2.object()["x"].toInt(), 100);
  QCOMPARE(doc2.object()["y"].toInt(), 200);
}

void StickyNoteTest::testNoteSavesOnResize() {
  cleanupNotes();

  QString notesDir =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir(notesDir);
  for (const QString &f : dir.entryList(QStringList() << "*.json", QDir::Files))
    QFile::remove(dir.filePath(f));

  TrayManager tray;
  QList<QAction *> actions = tray.getTrayIcon()->contextMenu()->actions();
  actions[0]->trigger();

  StickyNoteWidget *note = nullptr;
  for (QWidget *w : QApplication::topLevelWidgets()) {
    note = qobject_cast<StickyNoteWidget *>(w);
    if (note)
      break;
  }
  QVERIFY(note != nullptr);

  QTextEdit *textEdit = note->findChild<QTextEdit *>();
  note->m_isLoading = false;
  textEdit->setPlainText("Resize test");
  QApplication::processEvents();
  QFocusEvent focusOut(QEvent::FocusOut);
  QApplication::sendEvent(textEdit, &focusOut);
  QApplication::processEvents();

  QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files);
  QCOMPARE(files.count(), 1);
  QFile f1(notesDir + "/" + files.first());
  f1.open(QIODevice::ReadOnly);
  QJsonDocument doc1 = QJsonDocument::fromJson(f1.readAll());
  f1.close();
  QCOMPARE(doc1.object()["width"].toInt(), 250);
  QCOMPARE(doc1.object()["height"].toInt(), 200);

  note->resize(400, 300);
  QTest::qWait(2500);

  QFile f2(notesDir + "/" + files.first());
  f2.open(QIODevice::ReadOnly);
  QJsonDocument doc2 = QJsonDocument::fromJson(f2.readAll());
  f2.close();
  QCOMPARE(doc2.object()["width"].toInt(), 400);
  QCOMPARE(doc2.object()["height"].toInt(), 300);
}

void StickyNoteTest::testNoteLoadsFromFile() {
  cleanupNotes();

  QString notesDir =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir dir(notesDir);
  for (const QString &f : dir.entryList(QStringList() << "*.json", QDir::Files))
    QFile::remove(dir.filePath(f));

  QJsonObject savedObj;
  {
    TrayManager tray;
    QList<QAction *> actions = tray.getTrayIcon()->contextMenu()->actions();
    actions[0]->trigger();

    StickyNoteWidget *note = nullptr;
    for (QWidget *w : QApplication::topLevelWidgets()) {
      note = qobject_cast<StickyNoteWidget *>(w);
      if (note)
        break;
    }
    QVERIFY(note != nullptr);

    QTextEdit *textEdit = note->findChild<QTextEdit *>();
    note->m_isLoading = false;
    textEdit->setPlainText("Load test note");
    note->move(150, 250);
    note->resize(350, 280);
    QApplication::processEvents();
    QFocusEvent focusOut(QEvent::FocusOut);
    QApplication::sendEvent(textEdit, &focusOut);
    QApplication::processEvents();

    QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files);
    QCOMPARE(files.count(), 1);
    QFile f(notesDir + "/" + files.first());
    f.open(QIODevice::ReadOnly);
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    savedObj = doc.object();

    delete note;
    QApplication::processEvents();
  }

  TrayManager tray;

  StickyNoteWidget *loaded = nullptr;
  for (QWidget *w : QApplication::topLevelWidgets()) {
    loaded = qobject_cast<StickyNoteWidget *>(w);
    if (loaded)
      break;
  }
  QVERIFY(loaded != nullptr);

  QTextEdit *loadedEdit = loaded->findChild<QTextEdit *>();
  QVERIFY(loadedEdit != nullptr);

  QCOMPARE(loadedEdit->toPlainText(), savedObj["text"].toString());
  QCOMPARE(loaded->x(), savedObj["x"].toInt());
  QCOMPARE(loaded->y(), savedObj["y"].toInt());
  QCOMPARE(loaded->width(), savedObj["width"].toInt());
  QCOMPARE(loaded->height(), savedObj["height"].toInt());
  QCOMPARE(loaded->palette().color(QPalette::Window).name(),
           QColor(savedObj["noteColor"].toString()).name());

  QString styleSheet = loadedEdit->styleSheet();
  QRegularExpression re(";\\s*color:\\s*(#[0-9a-fA-F]+)");
  QRegularExpressionMatch match = re.match(styleSheet);
  QVERIFY(match.hasMatch());
  QCOMPARE(QColor(match.captured(1)).name(),
           QColor(savedObj["textColor"].toString()).name());

  QFont loadedFont;
  loadedFont.fromString(savedObj["font"].toString());
  QCOMPARE(loadedEdit->font().family(), loadedFont.family());
  QCOMPARE(loadedEdit->font().pointSize(), loadedFont.pointSize());
}
