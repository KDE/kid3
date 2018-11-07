/**
 * \file frametablemodel.cpp
 * Model for table with frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 01 May 2011
 *
 * Copyright (C) 2011-2018  Urs Fleisch
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

#include "frametablemodel.h"
#include <QGuiApplication>
#include <QBrush>
#include <QPalette>
#include <algorithm>
#include "fileconfig.h"
#include "pictureframe.h"
#include "framenotice.h"

namespace {

QHash<int,QByteArray> getRoleHash()
{
  QHash<int, QByteArray> roles;
  roles[Qt::CheckStateRole] = "checkState";
  roles[FrameTableModel::FrameTypeRole] = "frameType";
  roles[FrameTableModel::NameRole] = "name";
  roles[FrameTableModel::ValueRole] = "value";
  roles[FrameTableModel::ModifiedRole] = "modified";
  roles[FrameTableModel::TruncatedRole] = "truncated";
  roles[FrameTableModel::InternalNameRole] = "internalName";
  roles[FrameTableModel::FieldIdsRole] = "fieldIds";
  roles[FrameTableModel::FieldValuesRole] = "fieldValues";
  return roles;
}

}

/**
 * Constructor.
 * @param id3v1  true if model for ID3v1 frames
 * @param parent parent widget
 */
FrameTableModel::FrameTableModel(bool id3v1, QObject* parent)
  : QAbstractTableModel(parent), m_markedRows(0), m_changedFrames(0),
    m_id3v1(id3v1),
    m_guiApp(qobject_cast<QGuiApplication*>(QCoreApplication::instance()) != nullptr)
{
  setObjectName(QLatin1String("FrameTableModel"));
}

/**
 * Get item flags for index.
 * @param index model index
 * @return item flags
 */
Qt::ItemFlags FrameTableModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags theFlags = QAbstractTableModel::flags(index);
  if (index.isValid()) {
    if (index.column() == CI_Enable) {
      theFlags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    } else if (index.column() == CI_Value) {
      theFlags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }
  }
  return theFlags;
}

/**
 * Get data for a given role.
 * @param index model index
 * @param role item data role
 * @return data for role
 */
QVariant FrameTableModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() ||
      index.row() < 0 || index.row() >= static_cast<int>(frames().size()) ||
      index.column() < 0 || index.column() >= CI_NumColumns)
    return QVariant();
  auto it = frameAt(index.row());
  bool isModified = false, isTruncated = false;
  if ((role == Qt::BackgroundColorRole && index.column() == CI_Enable) ||
      role == ModifiedRole) {
    isModified = FileConfig::instance().markChanges() &&
      (it->isValueChanged() ||
      (static_cast<unsigned>((*it).getType()) < sizeof(m_changedFrames) * 8 &&
       (m_changedFrames & (1ULL << (*it).getType())) != 0));
  }
  if (((role == Qt::BackgroundColorRole || role == Qt::ToolTipRole) &&
       index.column() == CI_Value) ||
      role == TruncatedRole) {
    isTruncated = (static_cast<unsigned>(index.row()) < sizeof(m_markedRows) * 8 &&
        (m_markedRows & (1ULL << index.row())) != 0) || it->isMarked();
  }
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    if (index.column() == CI_Enable) {
      QString displayName = Frame::getDisplayName(it->getName());
      if (it->getType() == Frame::FT_Picture &&
          it->getValue() != Frame::differentRepresentation()) {
        QVariant fieldValue = it->getFieldValue(Frame::ID_PictureType);
        if (fieldValue.isValid()) {
          auto pictureType =
              static_cast<PictureFrame::PictureType>(fieldValue.toInt());
          if (pictureType != PictureFrame::PT_Other) {
            QString typeName = PictureFrame::getPictureTypeName(pictureType);
            if (!typeName.isEmpty()) {
              displayName += QLatin1String(": ");
              displayName += typeName;
            }
          }
        }
      }
      return displayName;
    } else if (index.column() == CI_Value)
      return it->getValue();
  } else if (role == Qt::CheckStateRole && index.column() == CI_Enable) {
    return m_frameSelected.at(index.row()) ? Qt::Checked : Qt::Unchecked;
  } else if (role == Qt::BackgroundColorRole) {
    if (index.column() == CI_Enable) {
      return !isModified ? Qt::NoBrush
                         : m_guiApp ? QGuiApplication::palette().mid()
                                    : QBrush(Qt::gray);
    } else if (index.column() == CI_Value) {
      return isTruncated ? QBrush(Qt::red) : Qt::NoBrush;
    }
  } else if (role == Qt::ToolTipRole) {
    QString toolTip;
    if (isTruncated && index.column() == CI_Value) {
      FrameNotice notice = it->isMarked() ? it->getNotice()
                                          : FrameNotice::Truncated;
      toolTip = notice.getDescription();
    }
    return toolTip;
  } else if (role == FrameTypeRole) {
    return it->getType();
  } else if (role == NameRole) {
    return Frame::getDisplayName(it->getName());
  } else if (role == ValueRole) {
    return it->getValue();
  } else if (role == ModifiedRole) {
    return isModified;
  } else if (role == TruncatedRole) {
    return isTruncated;
  } else if (role == InternalNameRole) {
    return it->getInternalName();
  } else if (role == FieldIdsRole) {
    QVariantList result;
    const Frame::FieldList& fields = it->getFieldList();
    for (auto fit = fields.constBegin(); fit != fields.constEnd(); ++fit) {
      result.append(fit->m_id);
    }
    return result;
  } else if (role == FieldValuesRole) {
    QVariantList result;
    const Frame::FieldList& fields = it->getFieldList();
    for (auto fit = fields.constBegin(); fit != fields.constEnd(); ++fit) {
      result.append(fit->m_value);
    }
    return result;
  }
  return QVariant();
}

/**
 * Set data for a given role.
 * @param index model index
 * @param value data value
 * @param role item data role
 * @return true if successful
 */
bool FrameTableModel::setData(const QModelIndex& index,
                              const QVariant& value, int role)
{
  if (!index.isValid() ||
      index.row() < 0 || index.row() >= static_cast<int>(frames().size()) ||
      index.column() < 0 || index.column() >= CI_NumColumns)
    return false;
  if ((role == Qt::EditRole && index.column() == CI_Value) ||
      role == ValueRole) {
    QString valueStr(value.toString());
    auto it = frameAt(index.row());
    if (valueStr != (*it).getValue()) {
      auto& frame = const_cast<Frame&>(*it);
      if (valueStr.isNull()) valueStr = QLatin1String("");
      frame.setValueIfChanged(valueStr);
      emit dataChanged(index, index);

      // Automatically set the checkbox when a value is changed
      if (!m_frameSelected.at(index.row())) {
        m_frameSelected[index.row()] = true;
        QModelIndex checkIndex(index.sibling(index.row(), CI_Enable));
        emit dataChanged(checkIndex, checkIndex);
      }
    }
    return true;
  } else if (role == Qt::CheckStateRole && index.column() == CI_Enable) {
    bool isChecked(value.toInt() == Qt::Checked);
    if (isChecked != m_frameSelected.at(index.row())) {
      m_frameSelected[index.row()] = isChecked;
      emit dataChanged(index, index);
    }
    return true;
  }
  return false;
}

/**
 * Get data for header section.
 * @param section column or row
 * @param orientation horizontal or vertical
 * @param role item data role
 * @return header data for role
 */
QVariant FrameTableModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation == Qt::Horizontal) {
    return section == CI_Enable ? tr("Name") : tr("Data");
  }
  return section + 1;
}

/**
 * Get number of rows.
 * @param parent parent model index, invalid for table models
 * @return number of rows,
 * if parent is valid number of children (0 for table models)
 */
int FrameTableModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : frames().size();
}

/**
 * Get number of columns.
 * @param parent parent model index, invalid for table models
 * @return number of columns,
 * if parent is valid number of children (0 for table models)
 */
int FrameTableModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : CI_NumColumns;
}

/**
 * Insert rows.
 * @param row rows are inserted before this row, if 0 at the begin,
 * if rowCount() at the end
 * @param count number of rows to insert
 * @param parent parent model index, invalid for table models
 * @return true if successful
 */
bool FrameTableModel::insertRows(int, int count, const QModelIndex&)
{
  for (int i = 0; i < count; ++i)
    insertFrame(Frame());
  return true;
}

/**
 * Insert a frame.
 * @param frame frame to insert
 * @return true if successful
 */
void FrameTableModel::insertFrame(const Frame& frame)
{
  auto it = m_frames.upper_bound(frame);
  int row = rowOf(it);
  beginInsertRows(QModelIndex(), row, row);
  it = m_frames.insert(it, frame);
  updateFrameRowMapping();
  resizeFrameSelected();
  endInsertRows();
}

/**
 * Remove rows.
 * @param row rows are removed starting with this row
 * @param count number of rows to remove
 * @param parent parent model index, invalid for table models
 * @return true if successful
 */
bool FrameTableModel::removeRows(int row, int count,
                        const QModelIndex&)
{
  if (count > 0) {
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = row; i < row + count; ++i) {
      m_frames.erase(frameAt(i));
    }
    updateFrameRowMapping();
    resizeFrameSelected();
    endRemoveRows();
  }
  return true;
}

/**
 * Map role identifiers to role property names in scripting languages.
 * @return hash mapping role identifiers to names.
 */
QHash<int,QByteArray> FrameTableModel::roleNames() const
{
  static QHash<int, QByteArray> roles = getRoleHash();
  return roles;
}

/**
 * Get the frame at a specific position in the collection.
 * @param row position of frame
 * @return iterator to frame
 */
FrameCollection::iterator FrameTableModel::frameAt(int row) const {
  return row >= 0 && row < m_frameOfRow.size()
      ? m_frameOfRow.at(row) : frames().end();
}

/**
 * Get the row corresponding to a frame iterator.
 * @param frameIt frame iterator
 * @return row number, number of rows if not found.
 */
int FrameTableModel::rowOf(FrameCollection::iterator frameIt) const {
  int row = 0;
  for (auto it = m_frameOfRow.constBegin(); it != m_frameOfRow.constEnd(); ++it) {
    if (frameIt == *it)
      break;
    ++row;
  }
  return row;
}

/**
 * Mark rows.
 * @param rowMask mask with bits of rows to mark
 */
void FrameTableModel::markRows(quint64 rowMask)
{
  quint64 changedBits = m_markedRows ^ rowMask;
  m_markedRows = rowMask;

  // Emit a change signal for all indexes affected by the change.
  if (!changedBits)
    return;

  quint64 mask;
  int row;
  for (mask = 1ULL, row = 0;
       static_cast<unsigned>(row) < sizeof(changedBits) * 8;
       mask <<= 1, ++row) {
    if ((changedBits & mask) != 0) {
      // Include both the columns for Qt::BackgroundColorRole and TruncatedRole.
      emit dataChanged(index(row, 0), index(row, 1));
    }
  }
}

/**
 * Mark changed frames.
 * @param frameMask mask with bits of frame types to mark
 */
void FrameTableModel::markChangedFrames(quint64 frameMask)
{
  quint64 changedBits = m_changedFrames ^ frameMask;
  m_changedFrames = frameMask;

  // Emit a change signal for all indexes affected by the change.
  if (!FileConfig::instance().markChanges() || !changedBits)
    return;

  const FrameCollection& frameCollection = frames();
  auto it = frameCollection.cbegin();
  int row = 0;
  for (; it != frameCollection.cend(); ++it, ++row) {
    if (it->isValueChanged() ||
        (static_cast<unsigned>((*it).getType()) < sizeof(changedBits) * 8 &&
         (changedBits & (1ULL << (*it).getType())) != 0)) {
      QModelIndex idx = index(row, CI_Enable);
      emit dataChanged(idx, idx);
    }
  }
}

/**
 * Get frame for index.
 * @param index model index
 * @return frame, 0 if no frame.
 */
const Frame* FrameTableModel::getFrameOfIndex(const QModelIndex& index) const
{
  if (index.isValid() && index.row() < static_cast<int>(frames().size())) {
    auto it = frameAt(index.row());
    return &(*it);
  }
  return nullptr;
}

/**
 * Get row with frame with a specific frame index.
 * @param index frame index
 * @return row number, -1 if not found.
 */
int FrameTableModel::getRowWithFrameIndex(int index) const
{
  int row = 0;
  for (auto it = m_frameOfRow.constBegin(); it != m_frameOfRow.constEnd(); ++it) {
    if ((*it)->getIndex() == index) {
      return row;
    }
    ++row;
  }
  return -1;
}

/**
 * Get row with frame with a specific frame name.
 * @param name name of frame
 * @return row number, -1 if not found.
 */
int FrameTableModel::getRowWithFrameName(const QString& name) const
{
  int row = 0;
  for (auto it = m_frameOfRow.constBegin(); it != m_frameOfRow.constEnd(); ++it) {
    if ((*it)->getName() == name) {
      return row;
    }
    ++row;
  }
  return -1;
}

/**
 * Get filter with enabled frames.
 *
 * @param allDisabledToAllEnabled true to enable all if all are disabled
 *
 * @return filter with enabled frames.
 */
FrameFilter FrameTableModel::getEnabledFrameFilter(
  bool allDisabledToAllEnabled) const
{
  FrameFilter filter;
  filter.enableAll();
  bool allDisabled = true;
  int numberRows = rowCount();
  int row = 0;
  for (auto it = m_frameOfRow.constBegin(); it != m_frameOfRow.constEnd(); ++it) {
    if (row >= numberRows) break;
    if (!m_frameSelected.at(row)) {
      filter.enable((*it)->getType(), (*it)->getName(), false);
    } else {
      allDisabled = false;
    }
    ++row;
  }
  if (allDisabledToAllEnabled && allDisabled) {
    filter.enableAll();
  }
  return filter;
}

/**
 * Get enabled frames.
 * @return frame collection with enabled frames.
 */
FrameCollection FrameTableModel::getEnabledFrames() const
{
  FrameCollection enabledFrames;
  const int numberRows = m_frameSelected.size();
  int row = 0;
  for (auto it = m_frameOfRow.constBegin(); it != m_frameOfRow.constEnd(); ++it) {
    if (row >= numberRows) break;
    if (m_frameSelected.at(row)) {
      enabledFrames.insert(**it);
    }
    ++row;
  }
  return enabledFrames;
}

/**
 * Clear frame collection.
 */
void FrameTableModel::clearFrames()
{
  const int numFrames = m_frames.size();
  if (numFrames > 0) {
    beginRemoveRows(QModelIndex(), 0, numFrames - 1);
    m_frames.clear();
    updateFrameRowMapping();
    m_frameSelected.clear();
    endRemoveRows();
  }
}

/**
 * Transfer frames to frame collection.
 * @param src frames to move into frame collection, will be cleared
 */
void FrameTableModel::transferFrames(FrameCollection& src)
{
  int oldNumFrames = m_frames.size();
  int newNumFrames = src.size();
  int numRowsChanged = qMin(oldNumFrames, newNumFrames);
  if (newNumFrames < oldNumFrames)
    beginRemoveRows(QModelIndex(), newNumFrames, oldNumFrames - 1);
  else if (newNumFrames > oldNumFrames)
    beginInsertRows(QModelIndex(), oldNumFrames, newNumFrames - 1);

  m_frames.clear();
  src.swap(m_frames);
  updateFrameRowMapping();
  resizeFrameSelected();

  if (newNumFrames < oldNumFrames)
    endRemoveRows();
  else if (newNumFrames > oldNumFrames)
    endInsertRows();
  if (numRowsChanged > 0)
    emit dataChanged(index(0, 0), index(numRowsChanged - 1, CI_NumColumns - 1));
}

/**
 * Set values which are different inactive.
 *
 * @param others frames to compare, will be modified
 */
void FrameTableModel::filterDifferent(FrameCollection& others)
{
  int oldNumFrames = m_frames.size();

  m_frames.filterDifferent(others);
  updateFrameRowMapping();
  resizeFrameSelected();

  if (oldNumFrames > 0)
    emit dataChanged(index(0, 0), index(oldNumFrames - 1, CI_NumColumns - 1));
  int newNumFrames = m_frames.size();
  if (newNumFrames > oldNumFrames) {
    beginInsertRows(QModelIndex(), oldNumFrames, newNumFrames - 1);
    endInsertRows();
  }
}

/**
 * Set the check state of all frames in the table.
 *
 * @param checked true to check the frames
 */
void FrameTableModel::setAllCheckStates(bool checked)
{
  const int numRows = rowCount();
  m_frameSelected.fill(checked, 0, numRows);
  emit dataChanged(index(0, CI_Enable), index(numRows - 1, CI_Enable));
}

/**
 * Select all frames in the table.
 */
void FrameTableModel::selectAllFrames()
{
  setAllCheckStates(true);
}

/**
 * Deselect all frames in the table.
 */
void FrameTableModel::deselectAllFrames()
{
  setAllCheckStates(false);
}

/**
 * Select changed frames in the table.
 */
void FrameTableModel::selectChangedFrames()
{
  int row = 0;
  auto it = m_frameOfRow.constBegin();
  for (; row < m_frameSelected.size() && it != m_frameOfRow.constEnd();
       ++row, ++it) {
    if ((*it)->isValueChanged()) {
      m_frameSelected[row] = true;
      QModelIndex idx = index(row, CI_Enable);
      emit dataChanged(idx, idx);
    }
  }
}

/**
 * Resize the bit array with the frame selection to match the frames size.
 */
void FrameTableModel::resizeFrameSelected()
{
  // If all bits are set, set also the new bits.
  int oldSize = m_frameSelected.size();
  int newSize = frames().size();
  bool setNewBits = newSize > oldSize && oldSize > 0 &&
      m_frameSelected.count(true) == oldSize;

  m_frameSelected.resize(newSize);

  if (setNewBits) {
    for (int i = oldSize; i < newSize; ++i) {
      m_frameSelected.setBit(i, true);
    }
  }
}

/**
 * Update the frame to row mapping.
 */
void FrameTableModel::updateFrameRowMapping()
{
  const FrameCollection& frameCollection = frames();
  m_frameOfRow.resize(frameCollection.size());
  auto frameIt = frameCollection.cbegin();
  auto rowIt = m_frameOfRow.begin(); // clazy:exclude=detaching-member
  for (; frameIt != frameCollection.cend(); ++frameIt, ++rowIt) {
    *rowIt = frameIt;
  }
  if (!m_frameTypeSeqNr.isEmpty()) {
    const QVector<int>& frameTypeSeqNr = m_frameTypeSeqNr;
    std::stable_sort(m_frameOfRow.begin(), m_frameOfRow.end(), // clazy:exclude=detaching-member
                     [&frameTypeSeqNr](FrameCollection::iterator lhs,
                                       FrameCollection::iterator rhs) {
      int lhsType = lhs->getType();
      int rhsType = rhs->getType();
      int lhsSeqNr = frameTypeSeqNr.at(lhsType);
      int rhsSeqNr = frameTypeSeqNr.at(rhsType);
      return lhsSeqNr < rhsSeqNr ||
             (lhsType == Frame::FT_Other && lhsType == rhsType &&
              lhs->getInternalName() < rhs->getInternalName());
    });
  }
}

/**
 * Set order of frames in frame table.
 * @param frameTypes ordered sequence of frame types
 * @remark This order is not used for ID3v1 frames.
 * @see TagConfig::quickAccessFrameOrder().
 */
void FrameTableModel::setFrameOrder(const QList<int>& frameTypes)
{
  if (frameTypes.isEmpty()) {
    m_frameTypeSeqNr.clear();
    return;
  } else if (frameTypes.size() != Frame::FT_LastFrame + 1) {
    qWarning("FrameTableModel::setFrameOrder: Invalid parameter size");
    m_frameTypeSeqNr.clear();
    return;
  }
  m_frameTypeSeqNr.resize(Frame::FT_UnknownFrame + 1);
  m_frameTypeSeqNr[Frame::FT_UnknownFrame] = Frame::FT_UnknownFrame;
  m_frameTypeSeqNr[Frame::FT_Other] = Frame::FT_Other;

  int seqNr = 0;
  auto it = frameTypes.constBegin();
  for (; it != frameTypes.constEnd(); ++it, ++seqNr) {
    int frameType = *it;
    if (frameType < 0 || frameType > Frame::FT_LastFrame) {
      qWarning("FrameTableModel::setFrameOrder: Invalid frame type %d",
               frameType);
      m_frameTypeSeqNr.clear();
      return;
    }
    m_frameTypeSeqNr[frameType] = seqNr;
  }
}
