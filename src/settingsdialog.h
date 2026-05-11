#pragma once

#include <QColor>
#include <QDialog>
#include <QFont>

class QPushButton;
class QLabel;
class QDialogButtonBox;

class SettingsDialog : public QDialog {
  Q_OBJECT

public:
  explicit SettingsDialog(QWidget *parent = nullptr);
  QColor noteColor() const;
  QColor textColor() const;
  QFont noteFont() const;
  void setNoteColor(const QColor &color);
  void setTextColor(const QColor &color);
  void setFont(const QFont &font);

private slots:
  void chooseNoteColor();
  void chooseTextColor();
  void chooseFont();

private:
  void updatePreview();
  QColor m_noteColor;
  QColor m_textColor;
  QFont m_font;
  QPushButton *noteColorButton;
  QPushButton *textColorButton;
  QPushButton *fontButton;
  QLabel *previewLabel;
  QDialogButtonBox *buttonBox;
};
