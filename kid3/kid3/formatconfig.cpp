/**
 * \file formatconfig.cpp
 * Format configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 *
 * Copyright (C) 2003-2007  Urs Fleisch
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

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#include <kconfigskeleton.h>
#else
#include <qstring.h>
#include <qstringlist.h>
#endif
#include "qtcompatmac.h"
#include "generalconfig.h"
#include "formatconfig.h"
#include "frame.h"

/**
 * Constructor.
 */
FormatConfig::FormatConfig(const QString& grp) :
	GeneralConfig(grp),
	m_formatWhileEditing(false),
	m_caseConversion(AllFirstLettersUppercase),
	m_strRepEnabled(false),
	m_filenameFormatter(false)
{
	m_strRepMap.clear();
}

/**
 * Destructor.
 */
FormatConfig::~FormatConfig() {}

/**
 * Set specific properties for a filename format.
 * This will set default string conversions and not touch the file
 * extension when formatting.
 */
void FormatConfig::setAsFilenameFormatter()
{
	m_filenameFormatter = true;
	m_strRepEnabled = true;
	m_strRepMap["/"] = "-";
	m_strRepMap[":"] = "-";
	m_strRepMap["."] = "";
	m_strRepMap["?"] = "";
	m_strRepMap["*"] = "";
	m_strRepMap["\""] = "''";
	m_strRepMap["ä"] = "ae";
	m_strRepMap["ö"] = "oe";
	m_strRepMap["ü"] = "ue";
	m_strRepMap["Ä"] = "Ae";
	m_strRepMap["Ö"] = "Oe";
	m_strRepMap["Ü"] = "Ue";
	m_strRepMap["ß"] = "ss";
}

/**
 * Format a string using this configuration.
 *
 * @param str string to format
 */
void FormatConfig::formatString(QString& str) const
{
	QString ext;
	int dotPos = -1;
	if (m_filenameFormatter) {
		/* Do not format the extension if it is a filename */
		dotPos = str.QCM_lastIndexOf('.');
		if (dotPos != -1) {
			ext = str.right(str.length() - dotPos);
			str = str.left(dotPos);
		}
	}
	if (m_caseConversion != NoChanges) {
		switch (m_caseConversion) {
			case AllLowercase:
				str = str.QCM_toLower();
				break;
			case AllUppercase:
				str = str.QCM_toUpper();
				break;
			case FirstLetterUppercase:
				str = str.at(0).QCM_toUpper() + str.right(str.length() - 1).QCM_toLower();
				break;
			case AllFirstLettersUppercase: {
				QString newstr;
				bool wordstart = true;
				for (unsigned i = 0; i < static_cast<unsigned>(str.length()); ++i) {
					QChar ch = str.at(i);
					if (!ch.isLetterOrNumber() &&
						ch != '\'' && ch != '`') {
						wordstart = true;
						newstr.append(ch);
					} else if (wordstart) {
						wordstart = false;
						newstr.append(ch.QCM_toUpper());
					} else {
						newstr.append(ch.QCM_toLower());
					}
				}
				str = newstr;
				break;
			}
			default:
				;
		}
	}
	if (m_strRepEnabled) {
		QMap<QString, QString>::ConstIterator it;
		for (it = m_strRepMap.begin(); it != m_strRepMap.end(); ++it) {
#if QT_VERSION >= 0x030100
			str.replace(it.key(), *it);
#else
			QString key(it.key()), data(it.data());
			int pos = 0, keylen = key.length();
			int datalen = data.length();
			while (pos < (int)str.length()) {
				pos = str.QCM_indexOf(key);
				if (pos == -1) break;
				str.replace(pos, keylen, data);
				pos += datalen;
			}
#endif
		}
	}
	/* append extension if it was removed */
	if (dotPos != -1) {
		str.append(ext);
	}
}

/**
 * Format frames using this configuration.
 *
 * @param frames frames
 */
void FormatConfig::formatFrames(FrameCollection& frames) const
{
	for (FrameCollection::iterator it = frames.begin();
			 it != frames.end();
			 ++it) {
		Frame& frame = const_cast<Frame&>(*it);
		if (frame.getType() != Frame::FT_Genre) {
			QString value(frame.getValue());
			if (!value.isEmpty()) {
				formatString(value);
				frame.setValueIfChanged(value);
			}
		}
	}
}

/**
 * Persist configuration.
 *
 * @param config KDE configuration
 */
void FormatConfig::writeToConfig(
#ifdef CONFIG_USE_KDE
	KConfig* config
#else
	Kid3Settings* config
#endif
	) const
{
#ifdef CONFIG_USE_KDE
	KCM_KConfigGroup(cfg, config, m_group);
	cfg.writeEntry("FormatWhileEditing", m_formatWhileEditing);
	cfg.writeEntry("CaseConversion", static_cast<int>(m_caseConversion));
	cfg.writeEntry("StrRepEnabled", m_strRepEnabled);
	cfg.writeEntry("StrRepMapKeys", m_strRepMap.keys());
	cfg.writeEntry("StrRepMapValues", m_strRepMap.values());
#else
	config->beginGroup("/" + m_group);
	config->QCM_writeEntry("/FormatWhileEditing", m_formatWhileEditing);
	config->QCM_writeEntry("/CaseConversion", m_caseConversion);
	config->QCM_writeEntry("/StrRepEnabled", m_strRepEnabled);
	config->QCM_writeEntry("/StrRepMapKeys", m_strRepMap.keys());
	config->QCM_writeEntry("/StrRepMapValues", m_strRepMap.values());
	config->endGroup();
#endif
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void FormatConfig::readFromConfig(
#ifdef CONFIG_USE_KDE
	KConfig* config
#else
	Kid3Settings* config
#endif
	)
{
#ifdef CONFIG_USE_KDE
	KCM_KConfigGroup(cfg, config, m_group);
	m_formatWhileEditing = cfg.KCM_readBoolEntry("FormatWhileEditing", m_formatWhileEditing);
	m_caseConversion = (CaseConversion)cfg.KCM_readNumEntry("CaseConversion",
														  (int)m_caseConversion);
	m_strRepEnabled = cfg.KCM_readBoolEntry("StrRepEnabled", m_strRepEnabled);
	QStringList keys = cfg.KCM_readListEntry("StrRepMapKeys");
	QStringList values = cfg.KCM_readListEntry("StrRepMapValues");
	if (!keys.empty() && !values.empty()) {
		QStringList::Iterator itk, itv;
		m_strRepMap.clear();
		for (itk = keys.begin(), itv = values.begin();
			 itk != keys.end() && itv != values.end();
			 ++itk, ++itv) {
			m_strRepMap[*itk] = *itv;
		}
	}
#else
	config->beginGroup("/" + m_group);
	m_formatWhileEditing = config->QCM_readBoolEntry("/FormatWhileEditing", m_formatWhileEditing);
	m_caseConversion = (CaseConversion)config->QCM_readNumEntry("/CaseConversion",
														  (int)m_caseConversion);
	m_strRepEnabled = config->QCM_readBoolEntry("/StrRepEnabled", m_strRepEnabled);
	QStringList keys = config->QCM_readListEntry("/StrRepMapKeys");
	QStringList values = config->QCM_readListEntry("/StrRepMapValues");
	if (!keys.empty() && !values.empty()) {
		QStringList::Iterator itk, itv;
		m_strRepMap.clear();
		for (itk = keys.begin(), itv = values.begin();
			 itk != keys.end() && itv != values.end();
			 ++itk, ++itv) {
			m_strRepMap[*itk] = *itv;
		}
	}
	config->endGroup();
#endif
}
