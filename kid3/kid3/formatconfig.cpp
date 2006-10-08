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
#if QT_VERSION < 0x030100
#include <qregexp.h>
#endif
#endif
#include "generalconfig.h"
#include "standardtags.h"
#include "formatconfig.h"

/**
 * Constructor.
 */
FormatConfig::FormatConfig(const QString &grp) :
	GeneralConfig(grp),
	m_formatWhileEditing(false),
	caseConversion(AllFirstLettersUppercase),
	strRepEnabled(false),
	filenameFormatter(false)
{
	strRepMap.clear();
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
	filenameFormatter = true;
	strRepEnabled = true;
	strRepMap["/"] = "-";
	strRepMap[":"] = "-";
	strRepMap["."] = "";
	strRepMap["?"] = "";
	strRepMap["*"] = "";
	strRepMap["ä"] = "ae";
	strRepMap["ö"] = "oe";
	strRepMap["ü"] = "ue";
	strRepMap["Ä"] = "Ae";
	strRepMap["Ö"] = "Oe";
	strRepMap["Ü"] = "Ue";
	strRepMap["ß"] = "ss";
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
	if (filenameFormatter) {
		/* Do not format the extension if it is a filename */
		dotPos = str.findRev('.');
		if (dotPos != -1) {
			ext = str.right(str.length() - dotPos);
			str = str.left(dotPos);
		}
	}
	if (caseConversion != NoChanges) {
		switch (caseConversion) {
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
	if (strRepEnabled) {
		QMap<QString, QString>::ConstIterator it;
		for (it = strRepMap.begin(); it != strRepMap.end(); ++it) {
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
	KConfig *config
#else
	Kid3Settings *config
#endif
	) const
{
#ifdef CONFIG_USE_KDE
	config->setGroup(group);
	config->writeEntry("FormatWhileEditing", m_formatWhileEditing);
	config->writeEntry("CaseConversion", caseConversion);
	config->writeEntry("StrRepEnabled", strRepEnabled);
	config->writeEntry("StrRepMapKeys", strRepMap.keys());
	config->writeEntry("StrRepMapValues", strRepMap.values());
#else
	config->beginGroup("/" + group);
	config->writeEntry("/FormatWhileEditing", m_formatWhileEditing);
	config->writeEntry("/CaseConversion", caseConversion);
	config->writeEntry("/StrRepEnabled", strRepEnabled);
#if QT_VERSION >= 300
	config->writeEntry("/StrRepMapKeys", strRepMap.keys());
	config->writeEntry("/StrRepMapValues", strRepMap.values());
#else
	config->writeEntry("/StrRepMap", strRepMap);
#endif
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
	KConfig *config
#else
	Kid3Settings *config
#endif
	)
{
#ifdef CONFIG_USE_KDE
	config->setGroup(group);
	m_formatWhileEditing = config->readBoolEntry("FormatWhileEditing", m_formatWhileEditing);
	caseConversion = (CaseConversion)config->readNumEntry("CaseConversion",
														  (int)caseConversion);
	strRepEnabled = config->readBoolEntry("StrRepEnabled", strRepEnabled);
	QStringList keys = config->readListEntry("StrRepMapKeys");
	QStringList values = config->readListEntry("StrRepMapValues");
	if (!keys.empty() && !values.empty()) {
		QStringList::Iterator itk, itv;
		strRepMap.clear();
		for (itk = keys.begin(), itv = values.begin();
			 itk != keys.end() && itv != values.end();
			 ++itk, ++itv) {
			strRepMap[*itk] = *itv;
		}
	}
#else
	config->beginGroup("/" + group);
	m_formatWhileEditing = config->readBoolEntry("/FormatWhileEditing", m_formatWhileEditing);
	caseConversion = (CaseConversion)config->readNumEntry("/CaseConversion",
														  (int)caseConversion);
	strRepEnabled = config->readBoolEntry("/StrRepEnabled", strRepEnabled);
#if QT_VERSION >= 300
	QStringList keys = config->readListEntry("/StrRepMapKeys");
	QStringList values = config->readListEntry("/StrRepMapValues");
	if (!keys.empty() && !values.empty()) {
		QStringList::Iterator itk, itv;
		strRepMap.clear();
		for (itk = keys.begin(), itv = values.begin();
			 itk != keys.end() && itv != values.end();
			 ++itk, ++itv) {
			strRepMap[*itk] = *itv;
		}
	}
#else
	strRepMap = config->readMapEntry("/StrRepMap", strRepMap);
#endif
	config->endGroup();
#endif
}
