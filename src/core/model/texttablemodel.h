/**
 * \file texttablemodel.h
 * Model to display a text with tabulators in a table.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 12 Aug 2011
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

#ifndef TEXTTABLEMODEL_H
#define TEXTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QStringList>
#include "kid3api.h"

/**
 * Model to display a text with tabulators in a table.
 */
class KID3_CORE_EXPORT TextTableModel : public QAbstractTableModel {
public:
  /**
   * Constructor.
   * @param parent parent widget
   */
  explicit TextTableModel(QObject* parent = 0);

  /**
   * Destructor.
   */
  virtual ~TextTableModel() override;

  /**
   * Get item flags for index.
   * @param index model index
   * @return item flags
   */
  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

  /**
   * Get data for a given role.
   * @param index model index
   * @param role item data role
   * @return data for role
   */
  virtual QVariant data(const QModelIndex& index,
                        int role=Qt::DisplayRole) const override;

  /**
   * Get data for header section.
   * @param section column or row
   * @param orientation horizontal or vertical
   * @param role item data role
   * @return header data for role
   */
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role=Qt::DisplayRole) const override;

  /**
   * Get number of rows.
   * @param parent parent model index, invalid for table models
   * @return number of rows,
   * if parent is valid number of children (0 for table models)
   */
  virtual int rowCount(const QModelIndex& parent=QModelIndex()) const override;

  /**
   * Get number of columns.
   * @param parent parent model index, invalid for table models
   * @return number of columns,
   * if parent is valid number of children (0 for table models)
   */
  virtual int columnCount(const QModelIndex& parent=QModelIndex()) const override;

  /**
   * Set the text to be displayed in the table.
   * @param text text with tab-separated columns and newline-separated rows
   * @param hasHeaderLine true if the first line is the header
   * @return true if the first line of the text contains a tab character.
   */
  bool setText(const QString& text, bool hasHeaderLine);

private:
  QList<QStringList> m_cells;
  bool m_hasHeaderLine;
};

#endif // TEXTTABLEMODEL_H
