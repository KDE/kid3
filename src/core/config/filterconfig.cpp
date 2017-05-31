/**
 * \file filterconfig.cpp
 * Configuration for filter dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Jan 2008
 *
 * Copyright (C) 2008-2013  Urs Fleisch
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

#include "filterconfig.h"
#include <QString>
#include "config.h"

int FilterConfig::s_index = -1;

/**
 * Constructor.
 */
FilterConfig::FilterConfig() :
  StoredConfig<FilterConfig>(QLatin1String("Filter")), m_filterIdx(0)
{
  /**
   * Preset filter expressions.
   */
  m_filterNames <<
    QLatin1String("All") <<
    QLatin1String("Filename Tag Mismatch") <<
    QLatin1String("No Tag 1") <<
    QLatin1String("No Tag 2") <<
    QLatin1String("ID3v2.2.0 Tag") <<
    QLatin1String("ID3v2.3.0 Tag") <<
    QLatin1String("ID3v2.4.0 Tag") <<
    QLatin1String("Tag 1 != Tag 2") <<
    QLatin1String("Tag 1 == Tag 2") <<
    QLatin1String("Incomplete") <<
    QLatin1String("No Picture") <<
    QLatin1String("Marked") <<
    QLatin1String("Custom Filter");
  m_filterExpressions <<
    QLatin1String("") <<
    QLatin1String("not (%{filepath} contains \"%{artist} - %{album}/%{track} %{title}\")") <<
    QLatin1String("%{tag1} equals \"\"") <<
    QLatin1String("%{tag2} equals \"\"") <<
    QLatin1String("%{tag2} equals \"ID3v2.2.0\"") <<
    QLatin1String("%{tag2} equals \"ID3v2.3.0\"") <<
    QLatin1String("%{tag2} equals \"ID3v2.4.0\"") <<
    QLatin1String("not (%1{title} equals %2{title} and %1{album} equals %2{album} and %1{artist} equals %2{artist} and %1{comment} equals %2{comment} and %1{year} equals %2{year} and %1{track} equals %2{track} and %1{genre} equals %2{genre})") <<
    QLatin1String("%1{title} equals %2{title} and %1{album} equals %2{album} and %1{artist} equals %2{artist} and %1{comment} equals %2{comment} and %1{year} equals %2{year} and %1{track} equals %2{track} and %1{genre} equals %2{genre}") <<
    QLatin1String("%{title} equals \"\" or %{artist} equals \"\" or %{album} equals \"\" or %{year} equals \"\" or %{tracknumber} equals \"\" or %{genre} equals \"\"") <<
    QLatin1String("%{picture} equals \"\"") <<
    QLatin1String("not (%{marked} equals \"\")") <<
    QLatin1String("");
}

/**
 * Destructor.
 */
FilterConfig::~FilterConfig() {}

/**
 * Persist configuration.
 *
 * @param config configuration
 */
void FilterConfig::writeToConfig(ISettings* config) const
{
  config->beginGroup(m_group);
  config->setValue(QLatin1String("FilterNames"), QVariant(m_filterNames));
  config->setValue(QLatin1String("FilterExpressions"), QVariant(m_filterExpressions));
  config->setValue(QLatin1String("FilterIdx"), QVariant(m_filterIdx));
  config->setValue(QLatin1String("WindowGeometry"), QVariant(m_windowGeometry));
  config->endGroup();
}

/**
 * Read persisted configuration.
 *
 * @param config configuration
 */
void FilterConfig::readFromConfig(ISettings* config)
{
  QStringList names, expressions;
  config->beginGroup(m_group);
  names = config->value(QLatin1String("FilterNames"),
                        m_filterNames).toStringList();
  expressions = config->value(QLatin1String("FilterExpressions"),
                              m_filterExpressions).toStringList();
  m_filterIdx = config->value(QLatin1String("FilterIdx"), m_filterIdx).toInt();
  m_windowGeometry = config->value(QLatin1String("WindowGeometry"),
                                   m_windowGeometry).toByteArray();

  config->endGroup();

  // KConfig seems to strip empty entries from the end of the string lists,
  // so we have to append them again.
  const int numNames = names.size();
  while (expressions.size() < numNames)
    expressions.append(QLatin1String(""));

  /* Use defaults if no configuration found */
  QStringList::const_iterator namesIt, expressionsIt;
  for (namesIt = names.begin(), expressionsIt = expressions.begin();
       namesIt != names.end() && expressionsIt != expressions.end();
       ++namesIt, ++expressionsIt) {
    int idx = m_filterNames.indexOf(*namesIt);
    if (idx >= 0) {
      m_filterExpressions[idx] = *expressionsIt;
    } else if (!(*namesIt).isEmpty()) {
      m_filterNames.append(*namesIt);
      m_filterExpressions.append(*expressionsIt);
    }
  }

  if (m_filterIdx >= static_cast<int>(m_filterNames.size()))
    m_filterIdx = 0;
}

/**
 * Set the filename format in the "Filename Tag Mismatch" filter.
 *
 * @param format filename format
 */
void FilterConfig::setFilenameFormat(const QString& format)
{
  int idx = m_filterNames.indexOf(QLatin1String("Filename Tag Mismatch"));
  if (idx != -1) {
    m_filterExpressions[idx] = QLatin1String("not (%{filepath} contains \"") +
      format + QLatin1String("\")");
  }
}

void FilterConfig::setFilterNames(const QStringList& filterNames)
{
  if (m_filterNames != filterNames) {
    m_filterNames = filterNames;
    emit filterNamesChanged(m_filterNames);
  }
}

void FilterConfig::setFilterExpressions(const QStringList& filterExpressions)
{
  if (m_filterExpressions != filterExpressions) {
    m_filterExpressions = filterExpressions;
    emit filterExpressionsChanged(m_filterExpressions);
  }
}

void FilterConfig::setFilterIndex(int filterIdx)
{
  if (m_filterIdx != filterIdx) {
    m_filterIdx = filterIdx;
    emit filterIndexChanged(m_filterIdx);
  }
}

void FilterConfig::setWindowGeometry(const QByteArray& windowGeometry)
{
  if (m_windowGeometry != windowGeometry) {
    m_windowGeometry = windowGeometry;
    emit windowGeometryChanged(m_windowGeometry);
  }
}
