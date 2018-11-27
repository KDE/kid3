/**
 * \file stringlisteditdialog.h
 * Editor to edit a list of strings.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 25 Nov 2018
 *
 * Copyright (C) 2018  Urs Fleisch
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

class QStringListModel;
class StringListEdit;

/**
 * Editor to edit a list of strings.
 */
class StringListEditDialog : public QDialog {
public:
  /**
   * Constructor.
   * @param strings list of strings to edit
   * @param title dialog title
   * @param parent parent widget
   */
  StringListEditDialog(const QStringList& strings, const QString& title,
                       QWidget* parent = nullptr);

  /**
   * Get list of strings edited in dialog.
   * Can be used to retrieve result when dialog is accepted.
   * @return list of strings currently shown in dialog.
   */
  QStringList stringList() const;

private:
  QStringListModel* const m_model;
  StringListEdit* const m_formatEdit;
};
