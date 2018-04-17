/**
 * \file guiconfig.cpp
 * GUI related configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013-2014  Urs Fleisch
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

#include "guiconfig.h"
#include <QStringList>

int GuiConfig::s_index = -1;

/**
 * Constructor.
 */
GuiConfig::GuiConfig() :
  StoredConfig<GuiConfig>(QLatin1String("GUI")),
  m_fileListSortColumn(0),
  m_fileListSortOrder(Qt::AscendingOrder),
  m_dirListSortColumn(0),
  m_dirListSortOrder(Qt::AscendingOrder),
  m_autoHideTags(true),
  m_hideFile(false),
  m_hidePicture(false),
  m_playOnDoubleClick(false)
{
  FOR_ALL_TAGS(tagNr) {
    m_hideTag[tagNr] = false;
  }
}

/**
 * Destructor.
 */
GuiConfig::~GuiConfig() {}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void GuiConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("AutoHideTags"), QVariant(m_autoHideTags));
  config->setValue(QLatin1String("HideFile"), QVariant(m_hideFile));
  FOR_ALL_TAGS(tagNr) {
    config->setValue(QLatin1String("HideV") + Frame::tagNumberToString(tagNr),
                     QVariant(m_hideTag[tagNr]));
  }
  config->setValue(QLatin1String("HidePicture"), QVariant(m_hidePicture));
  config->setValue(QLatin1String("PlayOnDoubleClick"), QVariant(m_playOnDoubleClick));
  config->setValue(QLatin1String("FileListSortColumn"),
                   QVariant(m_fileListSortColumn));
  config->setValue(QLatin1String("FileListSortOrder"),
                   QVariant(m_fileListSortOrder));
  config->setValue(QLatin1String("FileListVisibleColumns"),
                   QVariant(intListToStringList(m_fileListVisibleColumns)));
  config->setValue(QLatin1String("DirListSortColumn"),
                   QVariant(m_dirListSortColumn));
  config->setValue(QLatin1String("DirListSortOrder"),
                   QVariant(m_dirListSortOrder));
  config->setValue(QLatin1String("DirListVisibleColumns"),
                   QVariant(intListToStringList(m_dirListVisibleColumns)));

  QList<int>::const_iterator it;
  int i;
  for (it = m_splitterSizes.begin(), i = 0;
     it != m_splitterSizes.end();
     ++it, ++i) {
    config->setValue(QLatin1String("SplitterSize") + QString::number(i), QVariant(*it));
  }
  for (it = m_vSplitterSizes.begin(), i = 0;
     it != m_vSplitterSizes.end();
     ++it, ++i) {
    config->setValue(QLatin1String("VSplitterSize") + QString::number(i), QVariant(*it));
  }
  config->setValue(QLatin1String("ConfigWindowGeometry"), QVariant(m_configWindowGeometry));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void GuiConfig::readFromConfig(ISettings* config)
{
  config->beginGroup(m_group);
  m_autoHideTags = config->value(QLatin1String("AutoHideTags"), m_autoHideTags).toBool();
  m_hideFile = config->value(QLatin1String("HideFile"), m_hideFile).toBool();
  FOR_ALL_TAGS(tagNr) {
    m_hideTag[tagNr] = config->value(
          QLatin1String("HideV") + Frame::tagNumberToString(tagNr),
          m_hideTag[tagNr]).toBool();
  }
  m_hidePicture = config->value(QLatin1String("HidePicture"), m_hidePicture).toBool();
  m_playOnDoubleClick = config->value(QLatin1String("PlayOnDoubleClick"), m_playOnDoubleClick).toBool();
  m_fileListSortColumn = config->value(QLatin1String("FileListSortColumn"),
                                       m_fileListSortColumn).toInt();
  m_fileListSortOrder = static_cast<Qt::SortOrder>(
        config->value(QLatin1String("FileListSortOrder"),
                      static_cast<int>(m_fileListSortOrder)).toInt());
  m_fileListVisibleColumns = stringListToIntList(
        config->value(QLatin1String("FileListVisibleColumns"), QStringList()).
        toStringList());
  if (m_fileListVisibleColumns.isEmpty()) {
    // Uninitialized, otherwise there is at least the value 0 in the list.
    m_fileListVisibleColumns << 0 << 1 << 3;
  }
  m_dirListSortColumn = config->value(QLatin1String("DirListSortColumn"),
                                       m_dirListSortColumn).toInt();
  m_dirListSortOrder = static_cast<Qt::SortOrder>(
        config->value(QLatin1String("DirListSortOrder"),
                      static_cast<int>(m_dirListSortOrder)).toInt());
  m_dirListVisibleColumns = stringListToIntList(
        config->value(QLatin1String("DirListVisibleColumns"), QStringList()).
        toStringList());
  if (m_dirListVisibleColumns.isEmpty()) {
    // Uninitialized, otherwise there is at least the value 0 in the list.
    m_dirListVisibleColumns << 0 << 3;
  }

  m_splitterSizes.clear();
  for (int i = 0; i < 5; ++i) {
    int val = config->value(QLatin1String("SplitterSize") + QString::number(i), -1).toInt();
    if (val != -1) {
      m_splitterSizes.push_back(val);
    } else {
      break;
    }
  }
  m_vSplitterSizes.clear();
  for (int j = 0; j < 5; ++j) {
    int val = config->value(QLatin1String("VSplitterSize") + QString::number(j), -1).toInt();
    if (val != -1) {
      m_vSplitterSizes.push_back(val);
    } else {
      break;
    }
  }
  m_configWindowGeometry = config->value(QLatin1String("ConfigWindowGeometry"),
                                         m_configWindowGeometry).toByteArray();
  config->endGroup();
}

void GuiConfig::setFileListSortColumn(int fileListSortColumn)
{
  if (m_fileListSortColumn != fileListSortColumn) {
    m_fileListSortColumn = fileListSortColumn;
    emit fileListSortColumnChanged(m_fileListSortColumn);
  }
}

void GuiConfig::setFileListSortOrder(Qt::SortOrder fileListSortOrder)
{
  if (m_fileListSortOrder != fileListSortOrder) {
    m_fileListSortOrder = fileListSortOrder;
    emit fileListSortOrderChanged(m_fileListSortOrder);
  }
}

void GuiConfig::setFileListVisibleColumns(const QList<int>& fileListVisibleColumns)
{
  if (m_fileListVisibleColumns != fileListVisibleColumns) {
    m_fileListVisibleColumns = fileListVisibleColumns;
    emit fileListVisibleColumnsChanged(m_fileListVisibleColumns);
  }
}

void GuiConfig::setDirListSortColumn(int dirListSortColumn)
{
  if (m_dirListSortColumn != dirListSortColumn) {
    m_dirListSortColumn = dirListSortColumn;
    emit dirListSortColumnChanged(m_dirListSortColumn);
  }
}

void GuiConfig::setDirListSortOrder(Qt::SortOrder dirListSortOrder)
{
  if (m_dirListSortOrder != dirListSortOrder) {
    m_dirListSortOrder = dirListSortOrder;
    emit dirListSortOrderChanged(m_dirListSortOrder);
  }
}

void GuiConfig::setDirListVisibleColumns(const QList<int>& dirListVisibleColumns)
{
  if (m_dirListVisibleColumns != dirListVisibleColumns) {
    m_dirListVisibleColumns = dirListVisibleColumns;
    emit dirListVisibleColumnsChanged(m_dirListVisibleColumns);
  }
}

void GuiConfig::setSplitterSizes(const QList<int>& splitterSizes)
{
  if (m_splitterSizes != splitterSizes) {
    m_splitterSizes = splitterSizes;
    emit splitterSizesChanged(m_splitterSizes);
  }
}

void GuiConfig::setVSplitterSizes(const QList<int>& vSplitterSizes)
{
  if (m_vSplitterSizes != vSplitterSizes) {
    m_vSplitterSizes = vSplitterSizes;
    emit vSplitterSizesChanged(m_vSplitterSizes);
  }
}

void GuiConfig::setAutoHideTags(bool autoHideTags)
{
  if (m_autoHideTags != autoHideTags) {
    m_autoHideTags = autoHideTags;
    emit autoHideTagsChanged(m_autoHideTags);
  }
}

void GuiConfig::setHideFile(bool hideFile)
{
  if (m_hideFile != hideFile) {
    m_hideFile = hideFile;
    emit hideFileChanged(m_hideFile);
  }
}

void GuiConfig::setHideTag(Frame::TagNumber tagNr, bool hide)
{
  if (m_hideTag[tagNr] != hide) {
    m_hideTag[tagNr] = hide;
    emit hideTagChanged();
  }
}

void GuiConfig::setHidePicture(bool hidePicture)
{
  if (m_hidePicture != hidePicture) {
    m_hidePicture = hidePicture;
    emit hidePictureChanged(m_hidePicture);
  }
}

void GuiConfig::setPlayOnDoubleClick(bool playOnDoubleClick)
{
  if (m_playOnDoubleClick != playOnDoubleClick) {
    m_playOnDoubleClick = playOnDoubleClick;
    emit playOnDoubleClickChanged(m_playOnDoubleClick);
  }
}

void GuiConfig::setConfigWindowGeometry(const QByteArray& configWindowGeometry)
{
  if (m_configWindowGeometry != configWindowGeometry) {
    m_configWindowGeometry = configWindowGeometry;
    emit configWindowGeometryChanged(m_configWindowGeometry);
  }
}
