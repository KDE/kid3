/**
 * \file texttablemodel.cpp
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

#include "texttablemodel.h"
#include <QRegExp>

/**
 * Constructor.
 * @param parent parent widget
 */
TextTableModel::TextTableModel(QObject* parent) :
  QAbstractTableModel(parent), m_hasHeaderLine(false)
{
  setObjectName("TextTableModel");
}

/**
 * Destructor.
 */
TextTableModel::~TextTableModel()
{
}

/**
 * Get item flags for index.
 * @param index model index
 * @return item flags
 */
Qt::ItemFlags TextTableModel::flags(const QModelIndex& index) const
{
  if (index.isValid())
    return Qt::ItemIsEnabled;
  return QAbstractTableModel::flags(index);
}

/**
 * Get data for a given role.
 * @param index model index
 * @param role item data role
 * @return data for role
 */
QVariant TextTableModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();
  int rowNr = index.row() + (m_hasHeaderLine ? 1 : 0);
  if (!index.isValid() ||
      rowNr < 0 || rowNr >= m_cells.size() || index.column() < 0)
    return QVariant();
  const QStringList& row = m_cells.at(rowNr);
  if (index.column() < row.size() &&
      (role == Qt::DisplayRole || role == Qt::EditRole)) {
    return row.at(index.column());
  }
  return QVariant();
}

/**
 * Get data for header section.
 * @param section column or row
 * @param orientation horizontal or vertical
 * @param role item data role
 * @return header data for role
 */
QVariant TextTableModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation == Qt::Horizontal && m_hasHeaderLine && !m_cells.isEmpty() &&
      section < m_cells.first().size()) {
    return m_cells.first().at(section);
  }
  return section + 1;
}

/**
 * Get number of rows.
 * @param parent parent model index, invalid for table models
 * @return number of rows,
 * if parent is valid number of children (0 for table models)
 */
int TextTableModel::rowCount(const QModelIndex& parent) const
{
  int numRows = m_cells.size();
  if (m_hasHeaderLine && numRows > 0)
    --numRows;
  return parent.isValid() ? 0 : numRows;
}

/**
 * Get number of columns.
 * @param parent parent model index, invalid for table models
 * @return number of columns,
 * if parent is valid number of children (0 for table models)
 */
int TextTableModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : m_cells.isEmpty() ? 0 : m_cells.first().size();
}

/**
 * Set the text to be displayed in the table.
 * @param text text with tab-separated columns and newline-separated rows
 * @param hasHeaderLine true if the first line is the header
 * @return true if the first line of the text contains a tab character.
 */
bool TextTableModel::setText(const QString& text, bool hasHeaderLine)
{
  beginResetModel();
  m_hasHeaderLine = hasHeaderLine;
  m_cells.clear();
  QStringList lines = text.split(QRegExp("[\\r\\n]+"));
  if (lines.isEmpty() || lines.first().indexOf('\t') == -1) {
    endResetModel();
    return false;
  }

  for (int i = 0; i < lines.size(); ++i) {
    const QString& line = lines.at(i);
    if (i == lines.size() - 1 && line.isEmpty())
      break;
    m_cells.append(line.split('\t'));
  }
  endResetModel();
  return true;
}
