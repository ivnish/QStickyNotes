#pragma once

#include "config.h"
#include <QLabel>
#include <QPoint>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>
#include <QUuid>
#include <QWidget>

class StickyNoteWidget : public QWidget {
  Q_OBJECT

public:
  explicit StickyNoteWidget(QWidget *parent = nullptr);
  explicit StickyNoteWidget(const QString &id, QWidget *parent = nullptr);
  void loadFromJson(const QJsonObject &obj);
  bool m_isLoading;

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void moveEvent(QMoveEvent *event) override;

private:
  QPoint m_dragPosition;
  QTextEdit *m_textEdit;
  QWidget *m_header;
  QPushButton *m_addButton;
  QPushButton *m_closeButton;
  QPushButton *m_lockButton;
  bool m_locked;
  QLabel *m_resizer;
  bool m_resizing;
  QPointF m_resizeStartPos;
  QSize m_resizeStartSize;
  QString m_id;
  void updateResizerPosition();
  void saveToFile();
  int calculateTextHeight() const;
  void init();
  QPushButton *m_settingsButton;
  QColor m_noteColor;
  QColor m_textColor;
  QFont m_font;
  void applyStyle();
  bool m_isDirty;
  QTimer *m_saveTimer;

signals:
  void requestNewNote();
};
