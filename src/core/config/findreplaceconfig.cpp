/**
 * \file findreplaceconfig.cpp
 * Configuration for find/replace dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 5 Mar 2014
 *
 * Copyright (C) 2014-2024  Urs Fleisch
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

#include "findreplaceconfig.h"
#include "isettings.h"

int FindReplaceConfig::s_index = -1;

/**
 * Constructor.
 */
FindReplaceConfig::FindReplaceConfig()
  : StoredConfig(QLatin1String("FindReplace"))
{
}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void FindReplaceConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("Flags"),
                   QVariant(static_cast<int>(m_params.getFlags())));
  config->setValue(QLatin1String("Frames"), QVariant(m_params.getFrameMask()
#ifdef Q_OS_MAC
                   // Convince Mac OS X to store a 64-bit value.
                                                     | (Q_UINT64_C(1) << 63)
#endif
                                                     ));
  config->endGroup();
  config->beginGroup(m_group, true);
  config->setValue(QLatin1String("WindowGeometry"), QVariant(m_windowGeometry));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void FindReplaceConfig::readFromConfig(ISettings* config)
{
  config->beginGroup(m_group);
  m_params.setFlags(static_cast<TagSearcher::SearchFlags>(
                      config->value(QLatin1String("Flags"),
                        static_cast<int>(m_params.getFlags())).toInt()));
  m_params.setFrameMask(config->value(QLatin1String("Frames"),
                                      m_params.getFrameMask()).toULongLong()
#ifdef Q_OS_MAC
                        & ~(Q_UINT64_C(1) << 63)
#endif
                        );
  config->endGroup();
  config->beginGroup(m_group, true);
  m_windowGeometry = config->value(QLatin1String("WindowGeometry"),
                                   m_windowGeometry).toByteArray();
  config->endGroup();
}

void FindReplaceConfig::setParameterList(const QVariantList& lst)
{
  if (parameterList() != lst) {
    m_params.fromVariantList(lst);
    emit parameterListChanged();
  }
}

void FindReplaceConfig::setWindowGeometry(const QByteArray& windowGeometry)
{
  if (m_windowGeometry != windowGeometry) {
    m_windowGeometry = windowGeometry;
    emit windowGeometryChanged(m_windowGeometry);
  }
}
