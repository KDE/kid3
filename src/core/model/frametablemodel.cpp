/**
 * \file frametablemodel.cpp
 * Model for table with frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 01 May 2011
 *
 * Copyright (C) 2011-2024  Urs Fleisch
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
#include <algorithm>
#include "coretaggedfileiconprovider.h"
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
  roles[FrameTableModel::CompletionsRole] = "completions";
  roles[FrameTableModel::NoticeRole] = "notice";
  roles[FrameTableModel::NoticeWarningRole] = "noticeWarning";
  return roles;
}

}

/**
 * Constructor.
 * @param id3v1  true if model for ID3v1 frames
 * @param colorProvider colorProvider
 * @param parent parent widget
 */
FrameTableModel::FrameTableModel(
    bool id3v1, CoreTaggedFileIconProvider* colorProvider, QObject* parent)
  : QAbstractTableModel(parent), m_markedRows(0), m_changedFrames(0),
    m_colorProvider(colorProvider), m_id3v1(id3v1), m_emptyHeaders(false)
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
  if ((role == Qt::BackgroundRole && index.column() == CI_Enable) ||
      role == ModifiedRole) {
    Frame::ExtendedType extendedType = it->getExtendedType();
    Frame::Type type = extendedType.getType();
    isModified = FileConfig::instance().markChanges() &&
      (it->isValueChanged() ||
       (type != Frame::FT_Other &&
        static_cast<unsigned>(type) < sizeof(m_changedFrames) * 8 &&
        (m_changedFrames & (1ULL << type)) != 0) ||
       (type == Frame::FT_Other &&
        m_changedOtherFrameNames.contains(extendedType.getInternalName())));
  }
  if (((role == Qt::BackgroundRole || role == Qt::ToolTipRole) &&
       index.column() == CI_Value) ||
      (role == TruncatedRole || role == NoticeRole || role == NoticeWarningRole)) {
    isTruncated = (static_cast<unsigned>(index.row()) < sizeof(m_markedRows) * 8 &&
        (m_markedRows & (1ULL << index.row())) != 0) || it->isMarked();
  }
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    if (index.column() == CI_Enable) {
      QString displayName = Frame::getDisplayName(it->getName());
      if (it->getValue() != Frame::differentRepresentation()) {
        if (it->getType() == Frame::FT_Picture) {
          if (QVariant fieldValue = it->getFieldValue(Frame::ID_PictureType);
              fieldValue.isValid()) {
            if (auto pictureType =
                  static_cast<PictureFrame::PictureType>(fieldValue.toInt());
                pictureType != PictureFrame::PT_Other) {
              if (QString typeName = PictureFrame::getPictureTypeName(pictureType);
                  !typeName.isEmpty()) {
                displayName += QLatin1String(": ");
                displayName += typeName;
              }
            }
          }
        } else if (it->getType() == Frame::FT_Other) {
          if (it->getInternalName().startsWith(QLatin1String("RVA2"))) {
            if (QVariant fieldValue = it->getFieldValue(Frame::ID_Id);
                fieldValue.isValid()) {
              if (auto identifier = fieldValue.toString();
                  !identifier.isEmpty()) {
                displayName = tr("Volume");
                displayName += QLatin1String(": ");
                displayName += identifier;
              }
            }
          } else if (it->getInternalName().startsWith(QLatin1String("UFID"))) {
            if (QVariant fieldValue = it->getFieldValue(Frame::ID_Owner);
                fieldValue.isValid()) {
              if (auto owner = fieldValue.toString(); !owner.isEmpty()) {
                // Shorten the owner so that it is visible in the frame type column.
                // For example http://musicbrainz.org -> musicbrainz
                //             http://www.cddb.com/id3/taginfo.html -> taginfo
                //             http://www.id3.org/dummy/ufid.html -> ufid
                if (int endPos = owner.lastIndexOf(QLatin1Char('.'));
                    endPos != -1) {
                  int startPos = owner.lastIndexOf(QLatin1Char('.'), endPos - 1);
                  if (int slashPos = owner.lastIndexOf(QLatin1Char('/'), endPos - 1);
                      slashPos != -1 && slashPos > startPos) {
                    startPos = slashPos;
                  }
                  if (startPos != -1) {
                    owner = owner.mid(startPos + 1, endPos - startPos - 1);
                  }
                }
                displayName = tr("File ID");
                displayName += QLatin1String(": ");
                displayName += owner;
              }
            }
          }
        }
      }
      return displayName;
    }
    if (index.column() == CI_Value)
      return it->getValue();
  } else if (role == Qt::CheckStateRole && index.column() == CI_Enable) {
    return m_frameSelected.at(index.row()) ? Qt::Checked : Qt::Unchecked;
  } else if (role == Qt::BackgroundRole) {
    if (m_colorProvider) {
      if (index.column() == CI_Enable) {
        return m_colorProvider->colorForContext(
              isModified ? ColorContext::Marked : ColorContext::None);
      }
      if (index.column() == CI_Value) {
        return m_colorProvider->colorForContext(
          isTruncated ? ColorContext::Error : ColorContext::None);
      }
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
  } else if (role == CompletionsRole) {
#if QT_VERSION >= 0x050e00
    const QSet<QString> completions =
        getCompletionsForType(it->getExtendedType());
    QStringList result(completions.constBegin(), completions.constEnd());
#else
    QStringList result = getCompletionsForType(it->getExtendedType()).toList();
#endif
    result.sort();
    return result;
  } else if (role == NoticeRole) {
    QString toolTip;
    if (isTruncated) {
      FrameNotice notice = it->isMarked() ? it->getNotice()
                                          : FrameNotice::Truncated;
      toolTip = notice.getDescription();
    }
    return toolTip;
  } else if (role == NoticeWarningRole) {
    return isTruncated ? (it->isMarked() ? it->getNotice().getWarning()
                                         : FrameNotice::Truncated)
                       : FrameNotice::None;
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
    if (auto it = frameAt(index.row()); valueStr != it->getValue()) {
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
  }
  if (role == Qt::CheckStateRole && index.column() == CI_Enable) {
    if (bool isChecked(value.toInt() == Qt::Checked);
      isChecked != m_frameSelected.at(index.row())) {
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
  if (role != Qt::DisplayRole || m_emptyHeaders)
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
  return parent.isValid() ? 0 : static_cast<int>(frames().size());
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
 * @param count number of rows to insert
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
      // Include both the columns for Qt::BackgroundRole and TruncatedRole.
      emit dataChanged(index(row, 0), index(row, 1));
    }
  }
}

/**
 * Mark changed frames.
 * @param types frame types to mark
 */
void FrameTableModel::markChangedFrames(const QList<Frame::ExtendedType>& types)
{
  quint64 mask = 0;
  QSet<QString> changedOtherFrameNames;
  changedOtherFrameNames.clear();
  for (const auto& extendedType : types) {
    Frame::Type type = extendedType.getType();
    mask |= 1ULL << type;
    if (type == Frame::FT_Other) {
      if (const QString internalName = extendedType.getInternalName();
          !internalName.isEmpty()) {
        changedOtherFrameNames.insert(internalName);
      }
    }
  }

  quint64 changedBits = m_changedFrames ^ mask;
  m_changedFrames = mask;
  QSet<QString> addedNames = changedOtherFrameNames - m_changedOtherFrameNames;
  QSet<QString> removedNames = m_changedOtherFrameNames - changedOtherFrameNames;
  m_changedOtherFrameNames.swap(changedOtherFrameNames);

  // Emit a change signal for all indexes affected by the change.
  if (!FileConfig::instance().markChanges() ||
      (!changedBits && addedNames.isEmpty() && removedNames.isEmpty()))
    return;

  const FrameCollection& frameCollection = frames();
  auto it = frameCollection.cbegin();
  int row = 0;
  for (; it != frameCollection.cend(); ++it, ++row) {
    Frame::ExtendedType extendedType = it->getExtendedType();
    if (Frame::Type type = extendedType.getType(); type != Frame::FT_Other) {
      if (it->isValueChanged() ||
          (static_cast<unsigned>(type) < sizeof(changedBits) * 8 &&
           (changedBits & (1ULL << type)) != 0)) {
        QModelIndex idx = index(row, CI_Enable);
        emit dataChanged(idx, idx);
      }
    } else {
      if (const QString name = extendedType.getInternalName();
          it->isValueChanged() ||
          addedNames.contains(name) || removedNames.contains(name)) {
        QModelIndex idx = index(row, CI_Enable);
        emit dataChanged(idx, idx);
      }
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
 * @param index index within multiple frames with given @a name
 * @return row number, -1 if not found.
 */
int FrameTableModel::getRowWithFrameName(const QString& name, int index) const
{
  int row = 0;
  for (auto it = m_frameOfRow.constBegin(); it != m_frameOfRow.constEnd(); ++it) {
    if ((*it)->getName() == name) {
      if (index > 0) {
        --index;
      } else {
        return row;
      }
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
  if (const int numFrames = static_cast<int>(m_frames.size()); numFrames > 0) {
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
  int oldNumFrames = static_cast<int>(m_frames.size());
  int newNumFrames = static_cast<int>(src.size());
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
 * Start filtering different values.
 */
void FrameTableModel::beginFilterDifferent()
{
  m_differentValues.clear();
}

/**
 * End filtering different values.
 */
void FrameTableModel::endFilterDifferent()
{
}

/**
 * Get the different values which have been filtered for a frame type.
 * @param type frame type
 * @return different values.
 */
QSet<QString> FrameTableModel::getCompletionsForType(
    Frame::ExtendedType type) const
{
  return m_differentValues.value(type);
}

/**
 * Set values which are different inactive.
 *
 * @param others frames to compare, will be modified
 */
void FrameTableModel::filterDifferent(FrameCollection& others)
{
  int oldNumFrames = static_cast<int>(m_frames.size());

  m_frames.filterDifferent(others, &m_differentValues);
  updateFrameRowMapping();
  resizeFrameSelected();

  if (oldNumFrames > 0)
    emit dataChanged(index(0, 0), index(oldNumFrames - 1, CI_NumColumns - 1));
  if (int newNumFrames = static_cast<int>(m_frames.size());
      newNumFrames > oldNumFrames) {
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
  int newSize = static_cast<int>(frames().size());
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
  m_frameOfRow.resize(static_cast<int>(frameCollection.size()));
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
  }
  if (frameTypes.size() < Frame::FT_Custom1) {
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
  while (seqNr <= Frame::FT_LastFrame) {
    m_frameTypeSeqNr[seqNr] = seqNr;
    ++seqNr;
  }
}
