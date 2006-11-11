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
#if QT_VERSION < 0x030100
#include <qfile.h>
#include <qtextstream.h>
#include "qtcompatmac.h"

/** Quick and dirty replacement for QSettings */
Kid3Settings::Kid3Settings()
{
	QFile file("kid3.cfg");
	if (file.open(QCM_ReadOnly)) {
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

Kid3Settings::~Kid3Settings()
{
	QFile file("kid3.cfg");
	if (file.open(QCM_WriteOnly)) {
		QTextStream stream(&file);
		QMap<QString, QString>::Iterator it;
		for (it = map.begin(); it != map.end(); ++it) {
			stream << it.key() << "=" << it.data() << "\n";
		}
		file.close();
	}
}

void Kid3Settings::setPath(const QString &, const QString &, Kid3Settings::Scope) {} 

void Kid3Settings::beginGroup(const QString &grp)
{
	group = grp;
}

void Kid3Settings::endGroup() {}

void Kid3Settings::writeEntry(const QString &key, int val)
{
	map[group + key] = QString().setNum(val);
}

void Kid3Settings::writeEntry(const QString &key, bool val)
{
	map[group + key] = val ? "true" : "false";
}

void Kid3Settings::writeEntry(const QString &key, const QString &val)
{
	map[group + key] = val;
}

void Kid3Settings::writeEntry(const QString&, const QStringList&) { /* not used */ }

void Kid3Settings::writeEntry(const QString &key, const QMap<QString, QString> &val)
{
	QMap<QString, QString>::ConstIterator it;
	for (it = val.begin(); it != val.end(); ++it) {
		map[group + key + it.key()] = it.data();
	}
}

QString Kid3Settings::readEntry(const QString &key, const QString &dflt)
{
	return map.contains(group + key) ? map[group + key] : dflt;
}

int Kid3Settings::readNumEntry(const QString &key, int dflt)
{
	return map.contains(group + key) ? map[group + key].toInt() : dflt;
}

bool Kid3Settings::readBoolEntry(const QString &key, bool dflt)
{
	return map.contains(group + key) ? (map[group + key] == "true") : dflt;
}

QStringList Kid3Settings::readListEntry(const QString&) { /* not used */ return QStringList(); }

QMap<QString, QString> Kid3Settings::readMapEntry(const QString &key, const QMap<QString, QString> &dflt)
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
