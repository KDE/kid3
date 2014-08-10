/**
 * \file frametablemodel.cpp
 * Model for table with frames.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 01 May 2011
 *
 * Copyright (C) 2011-2013  Urs Fleisch
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
#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include "fileconfig.h"
#include "tagconfig.h"
#include "formatconfig.h"
#include "genremodel.h"
#include "tracknumbervalidator.h"
#include "pictureframe.h"

namespace {

QHash<int,QByteArray> getRoleHash()
{
  QHash<int, QByteArray> roles;
  roles[Qt::CheckStateRole] = "checkState";
  roles[FrameTableModel::FrameTypeRole] = "frameType";
  roles[FrameTableModel::NameRole] = "name";
  roles[FrameTableModel::ValueRole] = "value";
  return roles;
}

}

/**
 * Constructor.
 * @param id3v1  true if model for ID3v1 frames
 * @param parent parent widget
 */
FrameTableModel::FrameTableModel(bool id3v1, QObject* parent) :
  QAbstractTableModel(parent), m_markedRows(0), m_changedFrames(0),
  m_id3v1(id3v1)
{
  setObjectName(QLatin1String("FrameTableModel"));
#if QT_VERSION < 0x050000
  setRoleNames(getRoleHash());
#endif
}

/**
 * Destructor.
 */
FrameTableModel::~FrameTableModel()
{
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
 * Get a display representation of the a frame name.
 * For ID3v2-IDs with description, only the ID is returned.
 * Other non-empty strings are translated.
 *
 * @param str frame name
 *
 * @return display representation of name.
 */
QString FrameTableModel::getDisplayName(const QString& str)
{
  if (!str.isEmpty()) {
    int nlPos = str.indexOf(QLatin1Char('\n'));
    if (nlPos > 0) {
      // probably "TXXX - User defined text information\nDescription" or
      // "WXXX - User defined URL link\nDescription"
      return str.mid(nlPos + 1);
    } else if (str.midRef(4, 3) == QLatin1String(" - ")) {
      // probably "ID3-ID - Description"
      return str.left(4);
    } else {
      return QCoreApplication::translate("@default", str.toLatin1().data());
    }
  }
  return str;
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
      index.row() < 0 || index.row() >= static_cast<int>(m_frames.size()) ||
      index.column() < 0 || index.column() >= CI_NumColumns)
    return QVariant();
  FrameCollection::const_iterator it = frameAt(index.row());
  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    if (index.column() == CI_Enable) {
      QString displayName = getDisplayName(it->getName());
      if (it->getType() == Frame::FT_Picture &&
          it->getValue() != Frame::differentRepresentation()) {
        QVariant fieldValue = it->getFieldValue(Frame::Field::ID_PictureType);
        if (fieldValue.isValid()) {
          PictureFrame::PictureType pictureType =
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
      return FileConfig::instance().markChanges() &&
        (it->isValueChanged() ||
        (static_cast<unsigned>((*it).getType()) < sizeof(m_changedFrames) * 8 &&
         (m_changedFrames & (1ULL << (*it).getType())) != 0))
          ? QApplication::palette().mid() : Qt::NoBrush;
    } else if (index.column() == CI_Value &&
               static_cast<unsigned>(index.row()) < sizeof(m_markedRows) * 8) {
      return (m_markedRows & (1ULL << index.row())) != 0
             ? QBrush(Qt::red) : Qt::NoBrush;
    }
  } else if (role == FrameTypeRole) {
    return it->getType();
  } else if (role == NameRole) {
    return getDisplayName(it->getName());
  } else if (role == ValueRole) {
    return it->getValue();
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
      index.row() < 0 || index.row() >= static_cast<int>(m_frames.size()) ||
      index.column() < 0 || index.column() >= CI_NumColumns)
    return false;
  if ((role == Qt::EditRole && index.column() == CI_Value) ||
      role == ValueRole) {
    QString valueStr(value.toString());
    FrameCollection::iterator it = frameAt(index.row());
    if (valueStr != (*it).getValue()) {
      Frame& frame = const_cast<Frame&>(*it);
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
  return parent.isValid() ? 0 : m_frames.size();
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
  FrameCollection::iterator it = m_frames.upper_bound(frame);
  int row = rowOf(it);
  beginInsertRows(QModelIndex(), row, row);
  it = m_frames.insert(it, frame);
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
    m_frames.erase(frameAt(row), frameAt(row + count));
    resizeFrameSelected();
    endRemoveRows();
  }
  return true;
}

#if QT_VERSION >= 0x050000
/**
 * Map role identifiers to role property names in scripting languages.
 * @return hash mapping role identifiers to names.
 */
QHash<int,QByteArray> FrameTableModel::roleNames() const
{
  static QHash<int, QByteArray> roles = getRoleHash();
  return roles;
}
#endif

/**
 * Get the frame at a specific position in the collection.
 * @param row position of frame
 * @return const iterator to frame
 */
FrameCollection::const_iterator FrameTableModel::frameAt(int row) const {
  FrameCollection::const_iterator it = m_frames.begin();
  for (int i = 0; i < row; ++i) {
    if (++it == m_frames.end()) {
      break;
    }
  }
  return it;
}

/**
 * Get the frame at a specific position in the collection.
 * @param row position of frame
 * @return iterator to frame
 */
FrameCollection::iterator FrameTableModel::frameAt(int row) {
  FrameCollection::iterator it = m_frames.begin();
  for (int i = 0; i < row; ++i) {
    if (++it == m_frames.end()) {
      break;
    }
  }
  return it;
}

/**
 * Get the row corresponding to a frame iterator.
 * @param frameIt frame iterator
 * @return row number, number of rows if not found.
 */
int FrameTableModel::rowOf(FrameCollection::iterator frameIt) const {
  int row = 0;
  for (FrameCollection::const_iterator it = m_frames.begin();
       it != m_frames.end();
       ++it) {
    if (frameIt == it)
      break;
    ++row;
  }
  return row;
}

/**
 * Get frame for index.
 * @param index model index
 * @return frame, 0 if no frame.
 */
const Frame* FrameTableModel::getFrameOfIndex(const QModelIndex& index) const
{
  if (index.isValid() && index.row() < static_cast<int>(m_frames.size())) {
    FrameCollection::const_iterator it = frameAt(index.row());
    return &(*it);
  }
  return 0;
}

/**
 * Get row with frame with a specific frame index.
 * @param index frame index
 * @return row number, -1 if not found.
 */
int FrameTableModel::getRowWithFrameIndex(int index) const
{
  int row = 0;
  for (FrameCollection::const_iterator it = m_frames.begin();
       it != m_frames.end();
       ++it) {
    if (it->getIndex() == index) {
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
  for (FrameCollection::const_iterator it = m_frames.begin();
       it != m_frames.end();
       ++it) {
    if (it->getName() == name) {
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
  for (FrameCollection::const_iterator it = m_frames.begin();
       it != m_frames.end();
       ++it) {
    if (row >= numberRows) break;
    if (!m_frameSelected.at(row)) {
      filter.enable(it->getType(), it->getName(), false);
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
  for (FrameCollection::const_iterator it = m_frames.begin();
       it != m_frames.end();
       ++it) {
    if (row >= numberRows) break;
    if (m_frameSelected.at(row)) {
      enabledFrames.insert(*it);
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
  int row;
  FrameCollection::const_iterator it;
  for (row = 0, it = m_frames.begin();
       row < m_frameSelected.size() && it != m_frames.end();
       ++row, ++it) {
    if (it->isValueChanged()) {
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
  int newSize = m_frames.size();
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
 * Constructor.
 * @param genreModel genre model
 * @param parent parent QTableView
 */
FrameItemDelegate::FrameItemDelegate(GenreModel* genreModel, QObject* parent) :
  QItemDelegate(parent),
  m_genreModel(genreModel),
  m_trackNumberValidator(new TrackNumberValidator(this)),
  m_dateTimeValidator(new QRegExpValidator(QRegExp(QLatin1String(
    // This is a simplified regular expression from
    // http://www.pelagodesign.com/blog/2009/05/20/iso-8601-date-validation-that-doesnt-suck/
    // relaxed to allow appending any string after a slash, so that ISO 8601
    // periods of time can by entered while still providing suffient validation.
    "(\\d{4})(-((0[1-9]|1[0-2])(-([12]\\d|0[1-9]|3[01]))?)(T((([01]\\d|2[0-3])"
    "(:[0-5]\\d)?|24:00))?(:[0-5]\\d([\\.,]\\d+)?)?"
    "([zZ]|([\\+-])([01]\\d|2[0-3]):?([0-5]\\d)?)?)?(/.*)?)?"
  )), this)) {
  setObjectName(QLatin1String("FrameItemDelegate"));
}

/**
 * Destructor.
 */
FrameItemDelegate::~FrameItemDelegate()
{
}

/**
 * Format text if enabled.
 * @param txt text to format and set in line edit
 */
void FrameItemDelegate::formatTextIfEnabled(const QString& txt)
{
  QLineEdit* le;
  if (TagFormatConfig::instance().formatWhileEditing() &&
      (le = qobject_cast<QLineEdit*>(sender())) != 0) {
    QString str(txt);
    TagFormatConfig::instance().formatString(str);
    if (str != txt) {
      int curPos = le->cursorPosition();
      le->setText(str);
      le->setCursorPosition(curPos);
    }
  }
}

/**
 * Create an editor to edit the cells contents.
 * @param parent parent widget
 * @param option style
 * @param index  index of item
 * @return combo box editor widget.
 */
QWidget* FrameItemDelegate::createEditor(
  QWidget* parent, const QStyleOptionViewItem& option,
  const QModelIndex& index) const
{
  int row = index.row();
  int col = index.column();
  const FrameTableModel* ftModel =
    qobject_cast<const FrameTableModel*>(index.model());
  if (row >= 0 && (col == FrameTableModel::CI_Value || !ftModel)) {
    Frame::Type type = static_cast<Frame::Type>(
      index.data(FrameTableModel::FrameTypeRole).toInt());
    bool id3v1 = ftModel && ftModel->isId3v1();
    if (type == Frame::FT_Genre) {
      QComboBox* cb = new QComboBox(parent);
      if (!id3v1) {
        cb->setEditable(true);
        cb->setAutoCompletion(true);
        cb->setDuplicatesEnabled(false);
      }

      cb->setModel(m_genreModel);
      return cb;
    }
    QWidget* editor = QItemDelegate::createEditor(parent, option, index);
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
    if (id3v1 &&
        (type == Frame::FT_Comment || type == Frame::FT_Title ||
         type == Frame::FT_Artist || type == Frame::FT_Album)) {
      if (lineEdit) {
        if (TagFormatConfig::instance().formatWhileEditing()) {
          connect(lineEdit, SIGNAL(textChanged(QString)),
                  this, SLOT(formatTextIfEnabled(QString)));
        }
        lineEdit->setMaxLength(type == Frame::FT_Comment ? 28 : 30);
      }
    } else {
      if (lineEdit) {
        if (TagFormatConfig::instance().formatWhileEditing()) {
          connect(lineEdit, SIGNAL(textChanged(QString)),
                  this, SLOT(formatTextIfEnabled(QString)));
        }
        if (TagFormatConfig::instance().enableValidation()) {
          if (type == Frame::FT_Track || type == Frame::FT_Disc) {
            lineEdit->setValidator(m_trackNumberValidator);
          } else if (type == Frame::FT_Date || type == Frame::FT_OriginalDate) {
            lineEdit->setValidator(m_dateTimeValidator);
          }
        }
      }
    }
    return editor;
  }
  return QItemDelegate::createEditor(parent, option, index);
}

/**
 * Set data to be edited by the editor.
 * @param editor editor widget
 * @param index  index of item
 */
void FrameItemDelegate::setEditorData(
  QWidget* editor, const QModelIndex& index) const
{
  QComboBox* cb = qobject_cast<QComboBox*>(editor);
  if (cb) {
    QString genreStr(index.model()->data(index).toString());
    cb->setCurrentIndex(m_genreModel->getRowForGenre(genreStr));
  } else {
    QItemDelegate::setEditorData(editor, index);
  }
}

/**
 * Set model data supplied by editor.
 * @param editor editor widget
 * @param model  model
 * @param index  index of item
 */
void FrameItemDelegate::setModelData(
  QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
  QComboBox* cb = qobject_cast<QComboBox *>(editor);
  if (cb) {
    model->setData(index, cb->currentText());
  } else {
    QItemDelegate::setModelData(editor, model, index);
  }
}
