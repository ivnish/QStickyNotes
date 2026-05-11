#include "stickynotewidget.h"
#include "settings.h"
#include "settingsdialog.h"

#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFocusEvent>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QStandardPaths>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWindow>

void StickyNoteWidget::init() {
  setWindowTitle("Sticky Note");

  // No system frame. No taskbar entry.
  setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
  setAttribute(Qt::WA_X11NetWmWindowTypeUtility);

  resize(250, 200);

  // === HEADER ===
  m_header = new QWidget(this);
  m_header->setFixedHeight(28);
  m_header->installEventFilter(this);

  m_addButton = new QPushButton(this);
  m_closeButton = new QPushButton(this);
  m_lockButton = new QPushButton(this);
  m_settingsButton = new QPushButton(this);

  QString addButtonIcon = "/usr/share/QStickyNotes/Icons/add.png";
  m_addButton->setIcon(QIcon(addButtonIcon));
  QString closeButtonIcon = "/usr/share/QStickyNotes/Icons/close.png";
  m_closeButton->setIcon(QIcon(closeButtonIcon));
  m_locked = false;
  QString lockButtonIcon = "/usr/share/QStickyNotes/Icons/unlock.png";
  m_lockButton->setIcon(QIcon(lockButtonIcon));
  QString settingsButtonIcon = "/usr/share/QStickyNotes/Icons/settings.png";
  m_settingsButton->setIcon(QIcon(settingsButtonIcon));

  m_addButton->setFlat(true);
  m_closeButton->setFlat(true);
  m_lockButton->setFlat(true);
  m_settingsButton->setFlat(true);

  QHBoxLayout *headerLayout = new QHBoxLayout(m_header);
  headerLayout->setContentsMargins(5, 2, 5, 2);
  headerLayout->addWidget(m_closeButton);
  headerLayout->addStretch();
  headerLayout->addWidget(m_lockButton);
  headerLayout->addWidget(m_settingsButton);
  headerLayout->addWidget(m_addButton);

  // Load settings
  QString noteColorStr = gSettings->value("settings/noteColor", "#FFFBB2").toString();
  QString textColorStr = gSettings->value("settings/textColor", "#000000").toString();
  QColor noteColor(noteColorStr);
  QColor textColor(textColorStr);
  m_noteColor = QColor(noteColorStr);
  m_textColor = QColor(textColorStr);
  QFont defaultFont("Noto Sans", 10);
  defaultFont.setStyleName("Regular");
  QVariant fontVar = gSettings->value("settings/font");
  QFont font;
  if (fontVar.isValid()) {
    font = fontVar.value<QFont>();
    m_font = fontVar.value<QFont>();
  } else {
    font = defaultFont;
    m_font = defaultFont;
  }

  setAutoFillBackground(true);
  QPalette pal = palette();
  pal.setColor(QPalette::Window, noteColor);
  setPalette(pal);

  // === TEXT ===
  m_textEdit = new QTextEdit(this);
  m_textEdit->setFrameStyle(QFrame::NoFrame);
  m_textEdit->setFont(font);
  m_textEdit->setStyleSheet(QString("background-color: %1; color: %2;").arg(noteColor.name(), textColor.name()));
  m_textEdit->installEventFilter(this);
  m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
  m_textEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  m_isDirty = false;

  // === MAIN LAYOUT ===
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(2, 2, 2, 2);
  mainLayout->setSpacing(0);
  mainLayout->addWidget(m_header);
  mainLayout->addWidget(m_textEdit);

  m_resizing = false;
  m_resizer = new QLabel(this);
  m_resizer->setFixedSize(16, 16);
  m_resizer->setCursor(Qt::SizeFDiagCursor);
  QString resizerIcon = "/usr/share/QStickyNotes/Icons/resizer.png";
  m_resizer->setPixmap(QPixmap(resizerIcon));
  updateResizerPosition();
  m_resizer->installEventFilter(this);

  // === SIGNALS ===
  connect(m_closeButton, &QPushButton::clicked, this, [this]() {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Delete note"));
    msgBox.setText(tr("Are you sure you want to delete this note?"));

    QPushButton *cancelButton = msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *deleteButton = msgBox.addButton(tr("Delete"), QMessageBox::AcceptRole);

    // Cancel by default.
    msgBox.setDefaultButton(cancelButton);
    msgBox.setEscapeButton(cancelButton);
    msgBox.exec();

    if (msgBox.clickedButton() == deleteButton) {
      QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
      QString filePath = dirPath + "/" + m_id + ".json";
      if (QFile::exists(filePath)) {
        QFile::remove(filePath);
      }
      close();
    }
  });

  connect(m_lockButton, &QPushButton::clicked, [this]() {
    m_locked = !m_locked;
    if (!m_locked) {
      m_textEdit->setReadOnly(false);
      QString lockButtonIcon = "/usr/share/QStickyNotes/Icons/unlock.png";
      m_lockButton->setIcon(QIcon(lockButtonIcon));
    } else {
      m_textEdit->setReadOnly(true);
      QString lockButtonIcon = "/usr/share/QStickyNotes/Icons/lock.png";
      m_lockButton->setIcon(QIcon(lockButtonIcon));
    }
    saveToFile();
  });

  connect(m_addButton, &QPushButton::clicked, this, [this]() { emit requestNewNote(); });

  connect(m_textEdit, &QTextEdit::textChanged, this, [this]() {
    if (m_isLoading) {
      return;
    }
    int textHeight = calculateTextHeight();
    int headerHeight = m_header->height();
    int totalHeight = headerHeight + textHeight;
    if (height() < totalHeight) {
      resize(width(), totalHeight);
    }
    m_isDirty = true;
  });

  connect(m_settingsButton, &QPushButton::clicked, this, [this]() {
    SettingsDialog dialog;

    dialog.setNoteColor(m_noteColor);
    dialog.setTextColor(m_textColor);
    dialog.setFont(m_font);

    if (dialog.exec() == QDialog::Accepted) {
      m_noteColor = dialog.noteColor();
      m_textColor = dialog.textColor();
      m_font = dialog.noteFont();
      applyStyle();
      saveToFile();
    }
  });

  m_saveTimer = new QTimer(this);
  m_saveTimer->setSingleShot(true);
  connect(m_saveTimer, &QTimer::timeout, this, [this]() {
    if (!m_isLoading && m_isDirty) {
      saveToFile();
    }
  });
}

// New note constructor.
StickyNoteWidget::StickyNoteWidget(QWidget *parent) : QWidget(parent) {
  m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
  init();
}

// Load from file note constructor.
StickyNoteWidget::StickyNoteWidget(const QString &id, QWidget *parent) : QWidget(parent), m_id(id) {
  m_isLoading = true;
  init();
}

bool StickyNoteWidget::eventFilter(QObject *obj, QEvent *event) {
  if (obj == m_header) {
    if (event->type() == QEvent::MouseButtonPress) {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      if (mouseEvent->button() == Qt::LeftButton) {
        m_isDirty = true;
        QWindow *window = this->windowHandle();
        if (window) {
          window->startSystemMove();
        }
        return true;
      }
    }
  }

  if (obj == m_resizer) {
    if (event->type() == QEvent::MouseButtonPress) {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      if (mouseEvent->button() == Qt::LeftButton) {
        m_resizing = true;
        m_resizeStartPos = mouseEvent->globalPosition();
        m_resizeStartSize = size();
        return true;
      }
    }
    if (event->type() == QEvent::MouseMove) {
      if (m_resizing) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        QPointF delta = mouseEvent->globalPosition() - m_resizeStartPos;
        int newWidth = m_resizeStartSize.width() + delta.x();
        int newHeight = m_resizeStartSize.height() + delta.y();
        if (newWidth < 150) {
          newWidth = 150;
        }
        int minHeight = m_header->height() + calculateTextHeight();
        if (newHeight < minHeight) {
          newHeight = minHeight;
        }
        resize(newWidth, newHeight);
        return true;
      }
    }
    if (event->type() == QEvent::MouseButtonRelease) {
      m_resizing = false;
      return true;
    }
  }

  if (obj == m_textEdit) {
    if (event->type() == QEvent::FocusOut && m_isDirty && !m_isLoading) {
      saveToFile();
    }
  }

  return QWidget::eventFilter(obj, event);
}

void StickyNoteWidget::updateResizerPosition() { m_resizer->move(width() - m_resizer->width(), height() - m_resizer->height()); }

void StickyNoteWidget::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  updateResizerPosition();
  if (!m_isLoading) {
    m_isDirty = true;
    m_saveTimer->start(2000);
  }
}

void StickyNoteWidget::saveToFile() {
  QString dirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QString filePath = dirPath + "/" + m_id + ".json";

  QJsonObject obj;
  obj["id"] = m_id;
  obj["text"] = m_textEdit->toPlainText();
  obj["x"] = x();
  obj["y"] = y();
  obj["width"] = width();
  obj["height"] = height();
  obj["locked"] = m_locked;
  obj["noteColor"] = m_noteColor.name();
  obj["textColor"] = m_textColor.name();
  obj["font"] = m_font.toString();

  QJsonDocument doc(obj);

  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly)) {
    return;
  }

  file.write(doc.toJson());
  file.close();

  m_isDirty = false;

  qWarning() << "Save note to file";
}

int StickyNoteWidget::calculateTextHeight() const {
  int docHeight = static_cast<int>(m_textEdit->document()->size().height());
  int padding = 10;
  return docHeight + padding;
}

void StickyNoteWidget::loadFromJson(const QJsonObject &obj) {
  // Text.
  QString text = obj["text"].toString();
  m_textEdit->setPlainText(text);

  // Position.
  int posX = obj["x"].toInt(0);
  int posY = obj["y"].toInt(0);
  move(posX, posY);

  // Size.
  int w = obj["width"].toInt(250);
  int h = obj["height"].toInt(200);
  resize(w, h);

  // Locked state.
  m_locked = obj["locked"].toBool(false);
  if (!m_locked) {
    m_textEdit->setReadOnly(false);
    QString lockButtonIcon = "/usr/share/QStickyNotes/Icons/unlock.png";
    m_lockButton->setIcon(QIcon(lockButtonIcon));
  } else {
    m_textEdit->setReadOnly(true);
    QString lockButtonIcon = "/usr/share/QStickyNotes/Icons/lock.png";
    m_lockButton->setIcon(QIcon(lockButtonIcon));
  }

  m_noteColor = QColor(obj["noteColor"].toString("#FFFBB2"));
  m_textColor = QColor(obj["textColor"].toString("#000000"));
  QString fontStr = obj["font"].toString();
  if (!fontStr.isEmpty()) {
    m_font.fromString(fontStr);
  } else {
    m_font = QFont("Noto Sans", 10);
    m_font.setStyleName("Regular");
  }

  applyStyle();

  m_isDirty = false;
}

void StickyNoteWidget::applyStyle() {
  setAutoFillBackground(true);
  QPalette pal = palette();
  pal.setColor(QPalette::Window, m_noteColor);
  setPalette(pal);
  m_textEdit->setStyleSheet(QString("background-color: %1; color: %2;").arg(m_noteColor.name(), m_textColor.name()));
  m_textEdit->setFont(m_font);
}

void StickyNoteWidget::moveEvent(QMoveEvent *event) {
  QWidget::moveEvent(event);

  if (!m_isLoading) {
    m_isDirty = true;
    m_saveTimer->start(1000);
  }
}
