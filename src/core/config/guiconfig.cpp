/**
 * \file guiconfig.cpp
 * GUI related configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 29 Jun 2013
 *
 * Copyright (C) 2013  Urs Fleisch
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

int GuiConfig::s_index = -1;


/**
 * Constructor.
 */
GuiConfig::GuiConfig() :
  StoredConfig<GuiConfig>(QLatin1String("GUI")),
  m_autoHideTags(true),
  m_hideFile(false),
  m_hideV1(false),
  m_hideV2(false),
  m_hidePicture(false),
  m_playOnDoubleClick(false)
{
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
  config->setValue(QLatin1String("HideV1"), QVariant(m_hideV1));
  config->setValue(QLatin1String("HideV2"), QVariant(m_hideV2));
  config->setValue(QLatin1String("HidePicture"), QVariant(m_hidePicture));
  config->setValue(QLatin1String("PlayOnDoubleClick"), QVariant(m_playOnDoubleClick));

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
  m_hideV1 = config->value(QLatin1String("HideV1"), m_hideV1).toBool();
  m_hideV2 = config->value(QLatin1String("HideV2"), m_hideV2).toBool();
  m_hidePicture = config->value(QLatin1String("HidePicture"), m_hidePicture).toBool();
  m_playOnDoubleClick = config->value(QLatin1String("PlayOnDoubleClick"), m_playOnDoubleClick).toBool();

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
  config->endGroup();
}
