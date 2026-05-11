#include "settingsdialog.h"
#include "settings.h"

#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFontDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Settings");

  // Note color.
  QString noteColorStr = gSettings->value("settings/noteColor", "#FFFBB2").toString();
  m_noteColor = QColor(noteColorStr);

  // Text color.
  QString textColorStr = gSettings->value("settings/textColor", "#000000").toString();
  m_textColor = QColor(textColorStr);

  // Font.
  QFont defaultFont("Noto Sans", 10);
  defaultFont.setStyleName("Regular");
  QVariant fontVar = gSettings->value("settings/font");
  if (fontVar.isValid()) {
    m_font = fontVar.value<QFont>();
  } else {
    m_font = defaultFont;
  }

  noteColorButton = new QPushButton(tr("Note color"), this);
  textColorButton = new QPushButton(tr("Text color"), this);
  fontButton = new QPushButton(tr("Font"), this);

  previewLabel = new QLabel(tr("Preview text"), this);
  previewLabel->setMinimumHeight(60);
  previewLabel->setAlignment(Qt::AlignCenter);

  buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(noteColorButton);
  layout->addWidget(textColorButton);
  layout->addWidget(fontButton);
  layout->addWidget(previewLabel);
  layout->addWidget(buttonBox);

  connect(noteColorButton, &QPushButton::clicked, this, &SettingsDialog::chooseNoteColor);
  connect(textColorButton, &QPushButton::clicked, this, &SettingsDialog::chooseTextColor);
  connect(fontButton, &QPushButton::clicked, this, &SettingsDialog::chooseFont);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);

  updatePreview();
}

QColor SettingsDialog::noteColor() const { return m_noteColor; }

QColor SettingsDialog::textColor() const { return m_textColor; }

QFont SettingsDialog::noteFont() const { return m_font; }

void SettingsDialog::chooseNoteColor() {
  QColor color = QColorDialog::getColor(m_noteColor, this, tr("Select note color"));

  if (color.isValid()) {
    m_noteColor = color;
    updatePreview();
  }
}

void SettingsDialog::chooseTextColor() {
  QColor color = QColorDialog::getColor(m_textColor, this, tr("Select text color"));

  if (color.isValid()) {
    m_textColor = color;
    updatePreview();
  }
}

void SettingsDialog::chooseFont() {
  bool ok = false;
  QFont font = QFontDialog::getFont(&ok, m_font, this, tr("Select font"));

  if (ok) {
    m_font = font;
    updatePreview();
  }
}

void SettingsDialog::updatePreview() {
  QString style = QString("background-color: %1; color: %2;").arg(m_noteColor.name(), m_textColor.name());

  previewLabel->setStyleSheet(style);
  previewLabel->setFont(m_font);
}

void SettingsDialog::setNoteColor(const QColor &color) {
  m_noteColor = color;
  updatePreview();
}

void SettingsDialog::setTextColor(const QColor &color) {
  m_textColor = color;
  updatePreview();
}

void SettingsDialog::setFont(const QFont &font) {
  m_font = font;
  updatePreview();
}
