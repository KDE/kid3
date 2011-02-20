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
#include <QString>
#include <QStringList>
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
		dotPos = str.lastIndexOf('.');
		if (dotPos != -1) {
			ext = str.right(str.length() - dotPos);
			str = str.left(dotPos);
		}
	}
	if (m_caseConversion != NoChanges) {
		switch (m_caseConversion) {
			case AllLowercase:
				str = str.toLower();
				break;
			case AllUppercase:
				str = str.toUpper();
				break;
			case FirstLetterUppercase:
				str = str.at(0).toUpper() + str.right(str.length() - 1).toLower();
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
						newstr.append(ch.toUpper());
					} else {
						newstr.append(ch.toLower());
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
			str.replace(it.key(), *it);
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
	KConfigGroup cfg = config->group(m_group);
	cfg.writeEntry("FormatWhileEditing", m_formatWhileEditing);
	cfg.writeEntry("CaseConversion", static_cast<int>(m_caseConversion));
	cfg.writeEntry("StrRepEnabled", m_strRepEnabled);
	cfg.writeEntry("StrRepMapKeys", m_strRepMap.keys());
	cfg.writeEntry("StrRepMapValues", m_strRepMap.values());
#else
	config->beginGroup("/" + m_group);
	config->setValue("/FormatWhileEditing", QVariant(m_formatWhileEditing));
	config->setValue("/CaseConversion", QVariant(m_caseConversion));
	config->setValue("/StrRepEnabled", QVariant(m_strRepEnabled));
	config->setValue("/StrRepMapKeys", QVariant(m_strRepMap.keys()));
	config->setValue("/StrRepMapValues", QVariant(m_strRepMap.values()));
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
	KConfigGroup cfg = config->group(m_group);
	m_formatWhileEditing = cfg.readEntry("FormatWhileEditing", m_formatWhileEditing);
	m_caseConversion = (CaseConversion)cfg.readEntry("CaseConversion",
														  (int)m_caseConversion);
	m_strRepEnabled = cfg.readEntry("StrRepEnabled", m_strRepEnabled);
	QStringList keys = cfg.readEntry("StrRepMapKeys", QStringList());
	QStringList values = cfg.readEntry("StrRepMapValues", QStringList());
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
	m_formatWhileEditing = config->value("/FormatWhileEditing", m_formatWhileEditing).toBool();
	m_caseConversion = (CaseConversion)config->value("/CaseConversion",
																									 (int)m_caseConversion).toInt();
	m_strRepEnabled = config->value("/StrRepEnabled", m_strRepEnabled).toBool();
	QStringList keys = config->value("/StrRepMapKeys").toStringList();
	QStringList values = config->value("/StrRepMapValues").toStringList();
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
