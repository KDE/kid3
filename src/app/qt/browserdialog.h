/**
 * \file browserdialog.h
 * Help browser.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jun 2009
 *
 * Copyright (C) 2003-2024  Urs Fleisch
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

#pragma once

#include <QDialog>

class QTextBrowser;
class QLineEdit;

/**
 * Help browser.
 */
class BrowserDialog : public QDialog {
  Q_OBJECT
public:
  /**
   * Constructor.
   */
  BrowserDialog(QWidget* parent, const QString& caption);

  /**
   * Destructor.
   */
  ~BrowserDialog() override = default;

  /**
   * Show context help.
   * @param anchor name of anchor
   */
  void goToAnchor(const QString& anchor);

private slots:
  /**
   * Find previous occurrence of search text.
   */
  void findPrevious();

  /**
   * Find next occurrence of search text.
   */
  void findNext();

private:
  QTextBrowser* m_textBrowser;
  QLineEdit* m_findLineEdit;
  QString m_filename;
};
