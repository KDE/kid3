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
 *
 * @param grp configuration group
 */
GeneralConfig::GeneralConfig(const QString &grp) : group(grp) {}

/**
 * Destructor.
 */
GeneralConfig::~GeneralConfig() {}
