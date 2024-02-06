/**
 * \file progresswidget.h
 * Widget showing progress, similar to QProgressDialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Jan 2017
 *
 * Copyright (C) 2017-2024  Urs Fleisch
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

#include <QFrame>

class QLabel;
class QProgressBar;
class QPushButton;

/**
 * Widget showing progress, similar to QProgressDialog.
 */
class ProgressWidget : public QFrame {
  Q_OBJECT
public:
  /**
   * Constructor.
   * @param parent parent widget
   */
  explicit ProgressWidget(QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  ~ProgressWidget() override = default;

  /**
   * Set title.
   * @param text title
   */
  void setWindowTitle(const QString& text);

  /**
   * Set text of label.
   * @param text label, default is empty
   */
  void setLabelText(const QString& text);

  /**
   * Set text of cancel button.
   * @param text button text, default is "Cancel"
   */
  void setCancelButtonText(const QString& text);

  /**
   * Set minimum value.
   * @param minimum minimum value, default is 0
   */
  void setMinimum(int minimum);

  /**
   * Set maximum value.
   * @param maximum maximum value, default is 100
   */
  void setMaximum(int maximum);

  /**
   * Set current amount of progress made.
   * @param value progress value
   */
  void setValue(int value);

  /**
   * Set value and maximum, but only if it changes the current percentage.
   *
   * This will have better performance by avoiding too many UI updates.
   *
   * @param value progress value
   * @param maximum maximum value
   */
  void setValueAndMaximum(int value, int maximum);

  /**
   * Set format used for progress text.
   * @param format format string, can contain %p, %v, %m for
   *               percentage, value, total
   */
  void setFormat(const QString& format);

  /**
   * Reset the progress widget.
   */
  void reset();

  /**
   * Check if the cancel button was pressed.
   * @return true if the progress widget was canceled.
   */
  bool wasCanceled() const { return m_wasCanceled; }

signals:
  /**
   * Emitted when cancel is clicked.
   */
  void canceled();

private slots:
  void onCancelClicked();

private:
  QLabel* m_title;
  QLabel* m_label;
  QProgressBar* m_progress;
  QPushButton* m_cancelButton;
  int m_percentage;
  bool m_wasCanceled;
};
