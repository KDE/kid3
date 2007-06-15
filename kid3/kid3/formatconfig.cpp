/**
 * \file formatconfig.cpp
 * Format configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#include "config.h"
#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#else
#include <qstring.h>
#endif
#include "generalconfig.h"
#include "standardtags.h"
#include "formatconfig.h"

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
		dotPos = str.findRev('.');
		if (dotPos != -1) {
			ext = str.right(str.length() - dotPos);
			str = str.left(dotPos);
		}
	}
	if (m_caseConversion != NoChanges) {
		switch (m_caseConversion) {
			case AllLowercase:
				str = str.lower();
				break;
			case AllUppercase:
				str = str.upper();
				break;
			case FirstLetterUppercase:
				str = str.at(0).upper() + str.right(str.length() - 1).lower();
				break;
			case AllFirstLettersUppercase: {
				QString newstr;
				bool wordstart = true;
				for (uint i = 0; i < str.length(); ++i) {
					QChar ch = str.at(i);
					if (!ch.isLetterOrNumber() &&
						ch != '\'' && ch != '`') {
						wordstart = true;
						newstr.append(ch);
					} else if (wordstart) {
						wordstart = false;
						newstr.append(ch.upper());
					} else {
						newstr.append(ch.lower());
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
			str.replace(it.key(), it.data());
#else
			QString key(it.key()), data(it.data());
			int pos = 0, keylen = key.length();
			int datalen = data.length();
			while (pos < (int)str.length()) {
				pos = str.find(key);
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
 * Format title, artist and album using this configuration.
 *
 * @param st standard tags
 */
void FormatConfig::formatStandardTags(StandardTags& st) const
{
	formatString(st.title);
	formatString(st.artist);
	formatString(st.album);
	formatString(st.comment);
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
	config->setGroup(m_group);
	config->writeEntry("FormatWhileEditing", m_formatWhileEditing);
	config->writeEntry("CaseConversion", m_caseConversion);
	config->writeEntry("StrRepEnabled", m_strRepEnabled);
	config->writeEntry("StrRepMapKeys", m_strRepMap.keys());
	config->writeEntry("StrRepMapValues", m_strRepMap.values());
#else
	config->beginGroup("/" + m_group);
	config->writeEntry("/FormatWhileEditing", m_formatWhileEditing);
	config->writeEntry("/CaseConversion", m_caseConversion);
	config->writeEntry("/StrRepEnabled", m_strRepEnabled);
	config->writeEntry("/StrRepMapKeys", m_strRepMap.keys());
	config->writeEntry("/StrRepMapValues", m_strRepMap.values());
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
	config->setGroup(m_group);
	m_formatWhileEditing = config->readBoolEntry("FormatWhileEditing", m_formatWhileEditing);
	m_caseConversion = (CaseConversion)config->readNumEntry("CaseConversion",
														  (int)m_caseConversion);
	m_strRepEnabled = config->readBoolEntry("StrRepEnabled", m_strRepEnabled);
	QStringList keys = config->readListEntry("StrRepMapKeys");
	QStringList values = config->readListEntry("StrRepMapValues");
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
	m_formatWhileEditing = config->readBoolEntry("/FormatWhileEditing", m_formatWhileEditing);
	m_caseConversion = (CaseConversion)config->readNumEntry("/CaseConversion",
														  (int)m_caseConversion);
	m_strRepEnabled = config->readBoolEntry("/StrRepEnabled", m_strRepEnabled);
	QStringList keys = config->readListEntry("/StrRepMapKeys");
	QStringList values = config->readListEntry("/StrRepMapValues");
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
