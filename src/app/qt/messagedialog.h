/**
 * \file messagedialog.h
 * Message dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 18 Aug 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include <QDialog>
#include <QMessageBox>

class QLabel;
class QTextEdit;
class QDialogButtonBox;

/**
 * Message dialog.
 * Drop-in replacement for QMessageBox, but suitable for large texts.
 */
class MessageDialog : public QDialog {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param parent parent widget
   */
  explicit MessageDialog(QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~MessageDialog() override = default;

  /**
   * Set the text to be displayed.
   *
   * @param text message text.
   */
  void setText(const QString& text);

  /**
   * Set the informative text.
   * This text can be large and is displayed in a text edit.
   *
   * @param text message text.
   */
  void setInformativeText(const QString& text);

  /**
   * Set the message box's icon.
   *
   * @param icon icon to be displayed
   */
  void setIcon(QMessageBox::Icon icon);

  /**
   * Set buttons to be displayed.
   *
   * @param buttons buttons to be displayed
   */
  void setStandardButtons(QMessageBox::StandardButtons buttons);

  /**
   * Set default button.
   *
   * @param button button which gets default focus
   */
  void setDefaultButton(QMessageBox::StandardButton button);

  /**
   * Display a modal dialog with a list of items.
   *
   * @param parent parent widget
   * @param title dialog title
   * @param text dialog text
   * @param list list of items
   * @param buttons buttons shown
   *
   * @return QMessageBox::Ok or QMessageBox::Cancel.
   */
  static int warningList(QWidget* parent, const QString& title,
                         const QString& text, const QStringList& list,
                         QMessageBox::StandardButtons buttons =
                         QMessageBox::Ok | QMessageBox::Cancel);

private slots:
  /**
   * Called when a button is clicked.
   *
   * @param button button which was clicked
   */
  void buttonClicked(QAbstractButton* button);

private:
  QLabel* m_iconLabel;
  QLabel* m_textLabel;
  QTextEdit* m_textEdit;
  QDialogButtonBox* m_buttonBox;
};

#endif // MESSAGEDIALOG_H
