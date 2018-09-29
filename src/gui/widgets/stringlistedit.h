/**
 * \file stringlistedit.h
 * Widget to edit a string list.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 14 Apr 2007
 *
 * Copyright (C) 2007  Urs Fleisch
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

#ifndef STRINGLISTEDIT_H
#define STRINGLISTEDIT_H

#include "abstractlistedit.h"

/**
 * Widget to edit a string list.
 */
class StringListEdit : public AbstractListEdit {
  Q_OBJECT
public:
  /**
   * Constructor.
   *
   * @param model item model, e.g. a QStringListModel
   * @param parent parent widget
   */
  explicit StringListEdit(QAbstractItemModel* model, QWidget* parent = nullptr);

  /**
   * Destructor.
   */
  virtual ~StringListEdit() override;

public slots:
  /**
   * Add a new item.
   */
  virtual void addItem() override;

  /**
   * Edit the selected item.
   */
  virtual void editItem() override;
};

#endif // STRINGLISTEDIT_H
