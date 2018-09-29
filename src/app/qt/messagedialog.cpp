/**
 * \file messagedialog.cpp
 * Message dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Aug 2011
 *
 * Copyright (C) 2011-2013  Urs Fleisch
 *
 * This file is part of Kid3.
 *
 * Kid3 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Kid3 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "messagedialog.h"
#include <QLabel>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QStyle>
#include <QIcon>
#include <QPushButton>

/**
 * Constructor.
 *
 * @param parent parent widget
 */
MessageDialog::MessageDialog(QWidget* parent)
  : QDialog(parent)
{
  setObjectName(QLatin1String("MessageDialog"));
  QVBoxLayout* vlayout = new QVBoxLayout(this);

  QHBoxLayout* hlayout = new QHBoxLayout;
  m_iconLabel = new QLabel;
  m_iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  hlayout->addWidget(m_iconLabel);
  m_textLabel = new QLabel;
  m_textLabel->setWordWrap(true);
  m_textLabel->setMinimumSize(50, 50);
  hlayout->addWidget(m_textLabel);
  vlayout->addLayout(hlayout);

  m_textEdit = new QTextEdit;
  m_textEdit->setFocusPolicy(Qt::NoFocus);
  m_textEdit->setReadOnly(true);
  m_textEdit->hide();
  vlayout->addWidget(m_textEdit);

  m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
  m_buttonBox->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  connect(m_buttonBox, SIGNAL(clicked(QAbstractButton*)),
          this, SLOT(buttonClicked(QAbstractButton*)));
  vlayout->addWidget(m_buttonBox);
}

/**
 * Destructor.
 */
MessageDialog::~MessageDialog()
{}

/**
 * Set the text to be displayed.
 *
 * @param text message text.
 */
void MessageDialog::setText(const QString &text)
{
  m_textLabel->setText(text);
}

/**
 * Set the informative text.
   * This text can be large and is displayed in a text edit.
 *
 * @param text message text.
 */
void MessageDialog::setInformativeText(const QString& text)
{
  m_textEdit->setText(text);
  const QStringList lines = text.split(QLatin1Char('\n'));
  QFontMetrics fm(m_textEdit->fontMetrics());
  int maxWidth = 0;
  for (const QString& line : lines) {
    int lineWidth = fm.width(line);
    if (maxWidth < lineWidth) {
      maxWidth = lineWidth;
    }
  }
  maxWidth += fm.width(QLatin1String("WW")); // some space for the borders

  if (maxWidth <= 1000) {
    m_textEdit->setMinimumWidth(maxWidth);
    m_textEdit->setWordWrapMode(QTextOption::NoWrap);
  } else {
    m_textEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  }

  if (text.isEmpty()) {
    m_textEdit->hide();
  } else {
    m_textEdit->show();
  }
}

/**
 * Set the message box's icon.
 *
 * @param icon icon to be displayed
 */
void MessageDialog::setIcon(QMessageBox::Icon icon)
{
  QStyle::StandardPixmap sp = QStyle::SP_MessageBoxQuestion;
  bool hasIcon = true;
  switch (icon) {
  case QMessageBox::Question:
    sp = QStyle::SP_MessageBoxQuestion;
    break;
  case QMessageBox::Information:
    sp = QStyle::SP_MessageBoxInformation;
    break;
  case QMessageBox::Warning:
    sp = QStyle::SP_MessageBoxWarning;
    break;
  case QMessageBox::Critical:
    sp = QStyle::SP_MessageBoxCritical;
    break;
  case QMessageBox::NoIcon:
  default:
    hasIcon = false;
  }
  if (hasIcon) {
    QStyle* widgetStyle = style();
    int iconSize = widgetStyle->pixelMetric(
          QStyle::PM_MessageBoxIconSize, nullptr, this);
    m_iconLabel->setPixmap(widgetStyle->standardIcon(sp, nullptr, this).
                           pixmap(iconSize, iconSize));
  } else {
    m_iconLabel->setPixmap(QPixmap());
  }
}

/**
 * Set buttons to be displayed.
 *
 * @param buttons buttons to be displayed
 */
void MessageDialog::setStandardButtons(QMessageBox::StandardButtons buttons)
{
  m_buttonBox->setStandardButtons(
        QDialogButtonBox::StandardButtons(static_cast<int>(buttons)));
}

/**
 * Set default button.
 *
 * @param button button which gets default focus
 */
void MessageDialog::setDefaultButton(QMessageBox::StandardButton button)
{
  m_buttonBox->button(static_cast<QDialogButtonBox::StandardButton>(button))->
      setDefault(true);
}

/**
 * Called when a button is clicked.
 *
 * @param button button which was clicked
 */
void MessageDialog::buttonClicked(QAbstractButton* button)
{
  done(m_buttonBox->standardButton(button));
}

/**
 * Display a modal dialog with a list of items.
 *
 * @param parent parent widget
 * @param title dialog title
 * @param text dialog text
 * @param list list of items
 * @param buttons buttons shown
 *
 * @return QMessageBox::StandardButton code of pressed button.
 */
int MessageDialog::warningList(
  QWidget* parent, const QString& title,
  const QString& text, const QStringList& list,
  QMessageBox::StandardButtons buttons)
{
  MessageDialog dialog(parent);
  dialog.setWindowTitle(title);
  dialog.setText(text);
  dialog.setInformativeText(list.join(QLatin1String("\n")));
  dialog.setIcon(QMessageBox::Warning);
  dialog.setStandardButtons(buttons);
  return dialog.exec();
}
