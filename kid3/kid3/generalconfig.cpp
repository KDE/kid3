/**
 * \file generalconfig.cpp
 * General configuration.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 17 Sep 2003
 */

#include <qstring.h>
#include "generalconfig.h"

#ifdef CONFIG_USE_KDE
#include <kconfig.h>
#else
#if QT_VERSION >= 300
#include <qsettings.h>
#endif
#endif

#if !defined CONFIG_USE_KDE && (QT_VERSION < 300)
#include <qfile.h>
#include <qtextstream.h>

/** Quick and dirty replacement for QSettings */
QSettings::QSettings()
{
	QFile file("kid3.cfg");
	if (file.open(IO_ReadOnly)) {
		QTextStream stream(&file);
		QString line;
		while (!(line = stream.readLine()).isNull()) {
			int equalPos = line.find('=');
			if (equalPos > 0) {
				map[line.left(equalPos)] = line.mid(equalPos + 1);
			}
		}
		file.close();
	}
}

QSettings::~QSettings()
{
	QFile file("kid3.cfg");
	if (file.open(IO_WriteOnly)) {
		QTextStream stream(&file);
		QMap<QString, QString>::Iterator it;
		for (it = map.begin(); it != map.end(); ++it) {
			stream << it.key() << "=" << it.data() << "\n";
		}
		file.close();
	}
}

void QSettings::setPath(const QString &, const QString &, QSettings::Scope) {} 

void QSettings::beginGroup(const QString &grp)
{
	group = grp;
}

void QSettings::endGroup() {}

void QSettings::writeEntry(const QString &key, int val)
{
	map[group + key] = QString().setNum(val);
}

void QSettings::writeEntry(const QString &key, bool val)
{
	map[group + key] = val ? "true" : "false";
}

void QSettings::writeEntry(const QString &key, const QString &val)
{
	map[group + key] = val;
}

void QSettings::writeEntry(const QString &key, const QStringList &val) { /* not used */ }

void QSettings::writeEntry(const QString &key, const QMap<QString, QString> &val)
{
	QMap<QString, QString>::ConstIterator it;
	for (it = val.begin(); it != val.end(); ++it) {
		map[group + key + it.key()] = it.data();
	}
}

QString QSettings::readEntry(const QString &key, const QString &dflt)
{
	return map.contains(group + key) ? map[group + key] : dflt;
}

int QSettings::readNumEntry(const QString &key, int dflt)
{
	return map.contains(group + key) ? map[group + key].toInt() : dflt;
}

bool QSettings::readBoolEntry(const QString &key, bool dflt)
{
	return map.contains(group + key) ? (map[group + key] == "true") : dflt;
}

QStringList QSettings::readListEntry(const QString &key) { /* not used */ return QStringList(); }

QMap<QString, QString> QSettings::readMapEntry(const QString &key, const QMap<QString, QString> &dflt)
{
	bool found = false;
	QMap<QString, QString> val;
	QMap<QString, QString>::Iterator it;
	for (it = map.begin(); it != map.end(); ++it) {
		if (it.key().find(group + key) == 0) {
			found = true;
			val[it.key().mid((group + key).length())] = it.data();
		}
	}
	return found ? val : dflt;
}
#endif

/**
 * Constructor.
 * Set default configuration.
 */
GeneralConfig::GeneralConfig() :
	formatWhileEditing(false), importDestV1(true)
{
	importFormatIdx = 0;
	importFormatNames.append("freedb HTML text");
	importFormatNames.append("freedb HTML source");
	importFormatNames.append("Title");
	importFormatNames.append("Track Title");
	importFormatNames.append("Track Title Time");
	importFormatNames.append("Custom Format");
	/**
	 * Preset import format regular expressions.
	 * The following codes are used before the () expressions.
	 * %s title (song)
	 * %l album
	 * %a artist
	 * %c comment
	 * %y year
	 * %t track
	 * %g genre
	 */
	importFormatHeaders.append("%a(\\S[^\\r\\n/]*\\S)\\s*/\\s*%l(\\S[^\\r\\n]*\\S)[\\r\\n]+\\s*tracks:\\s+\\d+.*year:\\s*%y(\\d+).*genre:\\s*%g(\\S[^\\r\\n]*\\S)[\\r\\n]");
	importFormatHeaders.append("<[^>]+>%a([^<\\s][^\\r\\n/]*\\S)\\s*/\\s*%l(\\S[^\\r\\n]*[^\\s>])<[^>]+>[\\r\\n]+\\s*tracks:\\s+\\d+.*year:\\s*%y(\\d+).*genre:\\s*%g(\\S[^\\r\\n>]*\\S)<[^>]+>[\\r\\n]");
	importFormatHeaders.append("");
	importFormatHeaders.append("");
	importFormatHeaders.append("");
	importFormatHeaders.append("");
	importFormatTracks.append("[\\r\\n]%t(\\d+)[\\.\\s]+(?:\\d+:\\d+\\s+)?(?!\\d+:\\d+)%s(\\S[^\\r\\n]*\\S)");
	importFormatTracks.append("<td[^>]*>\\s*%t(\\d+).</td><td[^>]*>\\s*\\d+:\\d+</td><td[^>]*>(?:<[^>]+>)?%s([^<\\r\\n]+)");
	importFormatTracks.append("\\s*%s(\\S[^\\r\\n]*\\S)\\s*");
	importFormatTracks.append("\\s*%t(\\d+)[\\.\\s]+%s(\\S[^\\r\\n]*\\S)\\s*");
	importFormatTracks.append("\\s*%t(\\d+)[\\.\\s]+%s(\\S[^\\r\\n]*\\S)\\s+\\d+:\\d+\\s*");
	importFormatTracks.append("");
}

/**
 * Persist configuration.
 *
 * @param config KDE configuration
 */
void GeneralConfig::writeToConfig(
#ifdef CONFIG_USE_KDE
	KConfig *config
#else
	QSettings *config
#endif
	) const
{
#ifdef CONFIG_USE_KDE
	config->setGroup("General Options");
	config->writeEntry("FormatWhileEditing", formatWhileEditing);
	config->writeEntry("ImportDestinationV1", importDestV1);
	config->writeEntry("ImportFormatNames", importFormatNames);
	config->writeEntry("ImportFormatHeaders", importFormatHeaders);
	config->writeEntry("ImportFormatTracks", importFormatTracks);
	config->writeEntry("ImportFormatIdx", importFormatIdx);
#else
	config->beginGroup("/General Options");
	config->writeEntry("/FormatWhileEditing", formatWhileEditing);
	config->writeEntry("/ImportDestinationV1", importDestV1);
	config->writeEntry("/ImportFormatNames", importFormatNames);
	config->writeEntry("/ImportFormatHeaders", importFormatHeaders);
	config->writeEntry("/ImportFormatTracks", importFormatTracks);
	config->writeEntry("/ImportFormatIdx", importFormatIdx);
	config->endGroup();
#endif
}

/**
 * Read persisted configuration.
 *
 * @param config KDE configuration
 */
void GeneralConfig::readFromConfig(
#ifdef CONFIG_USE_KDE
	KConfig *config
#else
	QSettings *config
#endif
	)
{
	QStringList names, headers, tracks;
#ifdef CONFIG_USE_KDE
	config->setGroup("General Options");
	formatWhileEditing = config->readBoolEntry("FormatWhileEditing", formatWhileEditing);
	importDestV1 = config->readBoolEntry("ImportDestinationV1", importDestV1);
	names = config->readListEntry("ImportFormatNames");
	headers = config->readListEntry("ImportFormatHeaders");
	tracks = config->readListEntry("ImportFormatTracks");
	importFormatIdx = config->readNumEntry("ImportFormatIdx", importFormatIdx);
#else
	config->beginGroup("/General Options");
	formatWhileEditing = config->readBoolEntry("/FormatWhileEditing", formatWhileEditing);
	importDestV1 = config->readBoolEntry("/ImportDestinationV1", importDestV1);
	names = config->readListEntry("/ImportFormatNames");
	headers = config->readListEntry("/ImportFormatHeaders");
	tracks = config->readListEntry("/ImportFormatTracks");
	importFormatIdx = config->readNumEntry("/ImportFormatIdx", importFormatIdx);
	config->endGroup();
#endif
	/* Use defaults if no configuration found */
#if QT_VERSION >= 300
	if (!names.empty())   importFormatNames = names;
	if (!headers.empty()) importFormatHeaders = headers;
	if (!tracks.empty())  importFormatTracks = tracks;
#else
	if (!names.isEmpty())   importFormatNames = names;
	if (!headers.isEmpty()) importFormatHeaders = headers;
	if (!tracks.isEmpty())  importFormatTracks = tracks;
#endif
	/* Make sure that there are as many formats as names */
	int i, appendCnt = importFormatNames.count() - importFormatHeaders.count();
	for (i = 0; i < appendCnt; i++) {
		importFormatHeaders.append("");
	}
	appendCnt = importFormatNames.count() - importFormatTracks.count();
	for (i = 0; i < appendCnt; i++) {
		importFormatTracks.append("");
	}
}
