/**
 * \file trackdatamodel.cpp
 * Model for table with track data.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 15 May 2011
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

#include "trackdatamodel.h"
#include "frametablemodel.h"
#include "coretaggedfileiconprovider.h"

/**
 * Constructor.
 * @param colorProvider colorProvider
 * @param parent parent widget
 */
TrackDataModel::TrackDataModel(CoreTaggedFileIconProvider* colorProvider,
                               QObject* parent)
  : QAbstractTableModel(parent),
    m_colorProvider(colorProvider), m_maxDiff(0), m_diffCheckEnabled(false)
{
  setObjectName(QLatin1String("TrackDataModel"));
}

/**
 * Get item flags for index.
 * @param index model index
 * @return item flags
 */
Qt::ItemFlags TrackDataModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags theFlags = QAbstractTableModel::flags(index);
  if (index.isValid()) {
    theFlags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (static_cast<int>(m_frameTypes.at(index.column()).getType()) <
        FT_FirstTrackProperty) {
      theFlags |= Qt::ItemIsEditable;
    }
    if (index.column() == 0) {
      theFlags |= Qt::ItemIsUserCheckable;
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
QVariant TrackDataModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_trackDataVector.size()) ||
      index.column() < 0 ||
      index.column() >= static_cast<int>(m_frameTypes.size()))
    return QVariant();

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    const ImportTrackData& trackData = m_trackDataVector.at(index.row());
    Frame::ExtendedType type = m_frameTypes.at(index.column());
    auto typeOrProperty = static_cast<int>(type.getType());
    if (typeOrProperty < FT_FirstTrackProperty) {
      QString value(trackData.getValue(type));
      if (!value.isNull())
        return value;
    } else {
      switch (typeOrProperty) {
      case FT_FilePath:
        return trackData.getAbsFilename();
      case FT_FileName:
        return trackData.getFilename();
      case FT_Duration:
        if (int duration = trackData.getFileDuration()) {
          return TaggedFile::formatTime(duration);
        }
        break;
      case FT_ImportDuration:
        if (int duration = trackData.getImportDuration()) {
          return TaggedFile::formatTime(duration);
        }
        break;
      default:
        ;
      }
    }
  } else if (role == FrameTableModel::FrameTypeRole) {
    return m_frameTypes.at(index.column()).getType();
  } else if (role == Qt::BackgroundRole) {
    if (index.column() == 0 && m_diffCheckEnabled) {
      const ImportTrackData& trackData = m_trackDataVector.at(index.row());
      int diff = trackData.getTimeDifference();
      if (diff >= 0 && m_colorProvider) {
        return m_colorProvider->colorForContext(diff > m_maxDiff
            ? ColorContext::Error : ColorContext::None);
      }
    }
  } else if (role == Qt::CheckStateRole && index.column() == 0) {
    return m_trackDataVector.at(index.row()).isEnabled()
        ? Qt::Checked : Qt::Unchecked;
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
bool TrackDataModel::setData(const QModelIndex& index,
                              const QVariant& value, int role)
{
  if (!index.isValid() ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_trackDataVector.size()) ||
      index.column() < 0 ||
      index.column() >= static_cast<int>(m_frameTypes.size()))
    return false;

  if (role == Qt::EditRole) {
    ImportTrackData& trackData = m_trackDataVector[index.row()];
    Frame::ExtendedType type = m_frameTypes.at(index.column());
    if (static_cast<int>(type.getType()) >= FT_FirstTrackProperty)
      return false;

    trackData.setValue(type, value.toString());
    return true;
  } else if (role == Qt::CheckStateRole && index.column() == 0) {
    bool isChecked(value.toInt() == Qt::Checked);
    if (isChecked != m_trackDataVector.at(index.row()).isEnabled()) {
      m_trackDataVector[index.row()].setEnabled(isChecked);
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
QVariant TrackDataModel::headerData(
    int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();
  if (orientation == Qt::Horizontal && section < m_frameTypes.size()) {
    Frame::ExtendedType type = m_frameTypes.at(section);
    auto typeOrProperty = static_cast<int>(type.getType());
    if (typeOrProperty < FT_FirstTrackProperty) {
      return typeOrProperty == Frame::FT_Track
        ? tr("Track") // shorter header for track number
        : Frame::getDisplayName(type.getName());
    } else {
      switch (typeOrProperty) {
      case FT_FilePath:
        return tr("Absolute path to file");
      case FT_FileName:
        return tr("Filename");
      case FT_Duration:
        return tr("Duration");
      case FT_ImportDuration:
        return tr("Length");
      default:
        ;
      }
    }
  } else if (orientation == Qt::Vertical && section < m_trackDataVector.size()) {
    int fileDuration = m_trackDataVector.at(section).getFileDuration();
    if (fileDuration > 0) {
      return TaggedFile::formatTime(fileDuration);
    }
  }
  return section + 1;
}

/**
 * Get number of rows.
 * @param parent parent model index, invalid for table models
 * @return number of rows,
 * if parent is valid number of children (0 for table models)
 */
int TrackDataModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : m_trackDataVector.size();
}

/**
 * Get number of columns.
 * @param parent parent model index, invalid for table models
 * @return number of columns,
 * if parent is valid number of children (0 for table models)
 */
int TrackDataModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : m_frameTypes.size();
}

/**
 * Insert rows.
 * @param row rows are inserted before this row, if 0 at the begin,
 * if rowCount() at the end
 * @param count number of rows to insert
 * @param parent parent model index, invalid for table models
 * @return true if successful
 */
bool TrackDataModel::insertRows(int row, int count, const QModelIndex&)
{
  if (count > 0) {
    beginInsertRows(QModelIndex(), row, row + count - 1);
    m_trackDataVector.insert(row, count, ImportTrackData());
    endInsertRows();
  }
  return true;
}

/**
 * Remove rows.
 * @param row rows are removed starting with this row
 * @param count number of rows to remove
 * @param parent parent model index, invalid for table models
 * @return true if successful
 */
bool TrackDataModel::removeRows(int row, int count,
                        const QModelIndex&)
{
  if (count > 0) {
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    m_trackDataVector.remove(row, count);
    endRemoveRows();
  }
  return true;
}

/**
 * Insert columns.
 * @param column columns are inserted before this column, if 0 at the begin,
 * if columnCount() at the end
 * @param count number of columns to insert
 * @param parent parent model index, invalid for table models
 * @return true if successful
 */
bool TrackDataModel::insertColumns(int column, int count,
                           const QModelIndex&)
{
  if (count > 0) {
    beginInsertColumns(QModelIndex(), column, column + count - 1);
    for (int i = 0; i < count; ++i)
      m_frameTypes.insert(column, Frame::ExtendedType());
    endInsertColumns();
  }
  return true;
}

/**
 * Remove columns.
 * @param column columns are removed starting with this column
 * @param count number of columns to remove
 * @param parent parent model index, invalid for table models
 * @return true if successful
 */
bool TrackDataModel::removeColumns(int column, int count,
                           const QModelIndex&)
{
  if (count > 0) {
    beginRemoveColumns(QModelIndex(), column, column + count - 1);
    for (int i = 0; i < count; ++i)
      m_frameTypes.removeAt(column);
    endRemoveColumns();
  }
  return true;
}

/**
 * Set the check state of all tracks in the table.
 *
 * @param checked true to check the tracks
 */
void TrackDataModel::setAllCheckStates(bool checked)
{
  for (int row = 0; row < rowCount(); ++row) {
    m_trackDataVector[row].setEnabled(checked);
  }
}

/**
 * Set time difference check configuration.
 *
 * @param enable  true to enable check
 * @param maxDiff maximum allowed time difference
 */
void TrackDataModel::setTimeDifferenceCheck(bool enable, int maxDiff) {
  bool changed = m_diffCheckEnabled != enable || m_maxDiff != maxDiff;
  m_diffCheckEnabled = enable;
  m_maxDiff = maxDiff;
  if (changed)
    emit dataChanged(index(0,0), index(rowCount() - 1, 0));
}

/**
 * Calculate accuracy of imported track data.
 * @return accuracy in percent, -1 if unknown.
 */
int TrackDataModel::calculateAccuracy() const
{
  int numImportTracks = 0, numTracks = 0, numMismatches = 0, numMatches = 0;
  for (auto it = m_trackDataVector.constBegin();
       it != m_trackDataVector.constEnd();
       ++it) {
    const ImportTrackData& trackData = *it;
    int diff = trackData.getTimeDifference();
    if (diff >= 0) {
      if (diff > 3) {
        ++numMismatches;
      } else {
        ++numMatches;
      }
    } else {
      // no durations available => try to match using file name and title
      QSet<QString> titleWords = trackData.getTitleWords();
      int numWords = titleWords.size();
      if (numWords > 0) {
        QSet<QString> fileWords = trackData.getFilenameWords();
        if (fileWords.size() < numWords) {
          numWords = fileWords.size();
        }
        int wordMatch = numWords > 0
            ? 100 * (fileWords & titleWords).size() / numWords : 0;
        if (wordMatch < 75) {
          ++numMismatches;
        } else {
          ++numMatches;
        }
      }
    }
    if (trackData.getImportDuration() != 0 || !trackData.getTitle().isEmpty()) {
      ++numImportTracks;
    }
    if (trackData.getFileDuration() != 0) {
      ++numTracks;
    }
  }

  if (numTracks > 0 && numImportTracks > 0 &&
      (numMatches > 0 || numMismatches > 0)) {
    return numMatches * 100 / numTracks;
  }
  return -1;
}


/**
 * Get frame for index.
 * @param index model index
 * @return frame, 0 if no frame.
 */
const Frame* TrackDataModel::getFrameOfIndex(const QModelIndex& index) const
{
  if (!index.isValid() ||
      index.row() < 0 ||
      index.row() >= static_cast<int>(m_trackDataVector.size()) ||
      index.column() < 0 ||
      index.column() >= static_cast<int>(m_frameTypes.size()))
    return nullptr;

  const ImportTrackData& trackData = m_trackDataVector.at(index.row());
  Frame::ExtendedType type = m_frameTypes.at(index.column());
  if (static_cast<int>(type.getType()) >= FT_FirstTrackProperty)
    return nullptr;

  auto it = trackData.findByExtendedType(type);
  return it != trackData.cend() ? &(*it) : nullptr;
}

/**
 * Set track data.
 * @param trackDataVector track data
 */
void TrackDataModel::setTrackData(const ImportTrackDataVector& trackDataVector)
{
  static const int initFrameTypes[] = {
    FT_ImportDuration, FT_FileName, FT_FilePath,
    Frame::FT_Track, Frame::FT_Title,
    Frame::FT_Artist, Frame::FT_Album, Frame::FT_Date, Frame::FT_Genre,
    Frame::FT_Comment
  };

  QList<Frame::ExtendedType> newFrameTypes;
  for (auto initFrameType : initFrameTypes) {
    newFrameTypes.append( // clazy:exclude=reserve-candidates
        Frame::ExtendedType(static_cast<Frame::Type>(initFrameType), QLatin1String("")));
  }

  for (auto tit = trackDataVector.constBegin();
       tit != trackDataVector.constEnd();
       ++tit) {
    for (auto fit = tit->cbegin(); fit != tit->cend(); ++fit) {
      Frame::ExtendedType type = fit->getExtendedType();
      if (type.getType() > Frame::FT_LastV1Frame &&
          !newFrameTypes.contains(type)) {
        newFrameTypes.append(type);
      }
    }
  }

  int oldNumTypes = m_frameTypes.size();
  int newNumTypes = newFrameTypes.size();
  int numColumnsChanged = qMin(oldNumTypes, newNumTypes);
  if (newNumTypes < oldNumTypes)
    beginRemoveColumns(QModelIndex(), newNumTypes, oldNumTypes - 1);
  else if (newNumTypes > oldNumTypes)
    beginInsertColumns(QModelIndex(), oldNumTypes, newNumTypes - 1);

  m_frameTypes = newFrameTypes;

  if (newNumTypes < oldNumTypes)
    endRemoveColumns();
  else if (newNumTypes > oldNumTypes)
    endInsertColumns();

  int oldNumTracks = m_trackDataVector.size();
  int newNumTracks = trackDataVector.size();
  int numRowsChanged = qMin(oldNumTracks, newNumTracks);
  if (newNumTracks < oldNumTracks)
    beginRemoveRows(QModelIndex(), newNumTracks, oldNumTracks - 1);
  else if (newNumTracks > oldNumTracks)
    beginInsertRows(QModelIndex(), oldNumTracks, newNumTracks - 1);

  m_trackDataVector = trackDataVector;

  if (newNumTracks < oldNumTracks)
    endRemoveRows();
  else if (newNumTracks > oldNumTracks)
    endInsertRows();


  if (numRowsChanged > 0)
    emit dataChanged(
          index(0, 0), index(numRowsChanged - 1, numColumnsChanged - 1));
}

/**
 * Get track data.
 * @return track data
 */
ImportTrackDataVector TrackDataModel::getTrackData() const
{
  return m_trackDataVector;
}

/**
 * Get the frame type for a column.
 * @param column model column
 * @return frame type of Frame::Type or TrackDataModel::TrackProperties,
 *         -1 if column invalid.
 */
int TrackDataModel::frameTypeForColumn(int column) const
{
  return column < m_frameTypes.size() ? m_frameTypes.at(column).getType() : -1;
}

/**
 * Get column for a frame type.
 * @param frameType frame type of Frame::Type or
 *                  TrackDataModel::TrackProperties.
 * @return model column, -1 if not found.
 */
int TrackDataModel::columnForFrameType(int frameType) const
{
  return m_frameTypes.indexOf(
        Frame::ExtendedType(static_cast<Frame::Type>(frameType), QLatin1String("")));
}
