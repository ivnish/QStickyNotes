#ifndef TEST_STICKYNOTE_H
#define TEST_STICKYNOTE_H

#include <QObject>

class StickyNoteTest : public QObject {
  Q_OBJECT

private slots:
  void testNewNoteSavesToFile();
  void testNoteSettingsSavesToFile();
  void testNoteSavesOnFocusOut();
  void testNoteSavesOnMove();
  void testNoteSavesOnResize();
  void testNoteLoadsFromFile();
};

#endif
