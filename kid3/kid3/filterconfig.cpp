/**
 * \file filterconfig.cpp
 * Configuration for filter dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 16 Jan 2008
 *
 * Copyright (C) 2008  Urs Fleisch
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

#include <qstring.h>
#include "qtcompatmac.h"
#include "filterconfig.h"

#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#include <kconfigskeleton.h>
#endif

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
FilterConfig::FilterConfig(const QString& grp) :
	GeneralConfig(grp), m_filterIdx(0),
	m_windowWidth(-1), m_windowHeight(-1)
{
	/**
	 * Preset filter expressions.
	 */
	m_filterNames <<
		"All" <<
		"Filename Tag Mismatch" <<
		"No Tag 1" <<
		"No Tag 2" <<
		"ID3v2.3.0 Tag" <<
		"ID3v2.4.0 Tag" <<
		"Tag 1 != Tag 2" <<
		"Tag 1 == Tag 2" <<
		"No Picture" <<
		"Custom Filter";
	m_filterExpressions <<
		"" <<
		"not (%{filepath} contains \"%{artist} - %{album}/%{track} %{title}\")" <<
		"%{tag1} equals \"\"" <<
		"%{tag2} equals \"\"" <<
		"%{tag2} equals \"ID3v2.3.0\"" <<
		"%{tag2} equals \"ID3v2.4.0\"" <<
		"not (%1{title} equals %2{title} and %1{album} equals %2{album} and %1{artist} equals %2{artist} and %1{comment} equals %2{comment} and %1{year} equals %2{year} and %1{track} equals %2{track} and %1{genre} equals %2{genre})" <<
		"%1{title} equals %2{title} and %1{album} equals %2{album} and %1{artist} equals %2{artist} and %1{comment} equals %2{comment} and %1{year} equals %2{year} and %1{track} equals %2{track} and %1{genre} equals %2{genre}" <<
		"%{picture} equals \"\"" <<
		"";
}

/**
 * Destructor.
 */
FilterConfig::~FilterConfig() {}

/**
 * Persist configuration.
 *
 * @param config KDE configuration
 */
void FilterConfig::writeToConfig(
#ifdef CONFIG_USE_KDE
	KConfig* config
#else
	Kid3Settings* config
#endif
	) const
{
#ifdef CONFIG_USE_KDE
	KCM_KConfigGroup(cfg, config, m_group);
	cfg.writeEntry("FilterNames", m_filterNames);
	cfg.writeEntry("FilterExpressions", m_filterExpressions);
	cfg.writeEntry("FilterIdx", m_filterIdx);
	cfg.writeEntry("WindowWidth", m_windowWidth);
	cfg.writeEntry("WindowHeight", m_windowHeight);
#else
	config->beginGroup("/" + m_group);
	config->QCM_writeEntry("/FilterNames", m_filterNames);
	config->QCM_writeEntry("/FilterExpressions", m_filterExpressions);
	config->QCM_writeEntry("/FilterIdx", m_filterIdx);
	config->QCM_writeEntry("/WindowWidth", m_windowWidth);
	config->QCM_writeEntry("/WindowHeight", m_windowHeight);

	config->endGroup();
#endif
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void FilterConfig::readFromConfig(
#ifdef CONFIG_USE_KDE
	KConfig* config
#else
	Kid3Settings* config
#endif
	)
{
	QStringList names, expressions;
#ifdef CONFIG_USE_KDE
	KCM_KConfigGroup(cfg, config, m_group);
	names = cfg.KCM_readListEntry("FilterNames");
	expressions = cfg.KCM_readListEntry("FilterExpressions");
	m_filterIdx = cfg.KCM_readNumEntry("FilterIdx", m_filterIdx);
	m_windowWidth = cfg.KCM_readNumEntry("WindowWidth", -1);
	m_windowHeight = cfg.KCM_readNumEntry("WindowHeight", -1);

	// KConfig seems to strip empty entries from the end of the string lists,
	// so we have to append them again.
	unsigned numNames = names.size();
	while (static_cast<unsigned>(expressions.size()) < numNames)
		expressions.append("");
#else
	config->beginGroup("/" + m_group);
	names = config->QCM_readListEntry("/FilterNames");
	expressions = config->QCM_readListEntry("/FilterExpressions");
	m_filterIdx = config->QCM_readNumEntry("/FilterIdx", m_filterIdx);
	m_windowWidth = config->QCM_readNumEntry("/WindowWidth", -1);
	m_windowHeight = config->QCM_readNumEntry("/WindowHeight", -1);

	config->endGroup();
#endif
	/* Use defaults if no configuration found */
	QStringList::const_iterator namesIt, expressionsIt;
	for (namesIt = names.begin(), expressionsIt = expressions.begin();
			 namesIt != names.end() && expressionsIt != expressions.end();
			 ++namesIt, ++expressionsIt) {
#if QT_VERSION >= 0x040000
		int idx = m_filterNames.indexOf(*namesIt);
#else
		int idx = m_filterNames.findIndex(*namesIt);
#endif
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
#if QT_VERSION >= 0x040000
	int idx = m_filterNames.indexOf("Filename Tag Mismatch");
#else
	int idx = m_filterNames.findIndex("Filename Tag Mismatch");
#endif
	if (idx != -1) {
		m_filterExpressions[idx] = QString("not (%{filepath} contains \"") +
			format + "\")";
	}
}

