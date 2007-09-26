/**
 * \file generalconfig.cpp
 * General configuration.
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
			int equalPos = line.QCM_indexOf('=');
			if (equalPos > 0) {
				m_map[line.left(equalPos)] = line.mid(equalPos + 1);
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
		for (it = m_map.begin(); it != m_map.end(); ++it) {
			stream << it.key() << "=" << it.data() << "\n";
		}
		file.close();
	}
}

void Kid3Settings::setPath(const QString&, const QString&, Kid3Settings::Scope) {} 

void Kid3Settings::beginGroup(const QString& grp)
{
	m_group = grp;
}

void Kid3Settings::endGroup() {}

void Kid3Settings::writeEntry(const QString& key, int val)
{
	m_map[m_group + key] = QString().setNum(val);
}

void Kid3Settings::writeEntry(const QString& key, bool val)
{
	m_map[m_group + key] = val ? "true" : "false";
}

void Kid3Settings::writeEntry(const QString& key, const QString& val)
{
	m_map[m_group + key] = val;
}

void Kid3Settings::writeEntry(const QString&, const QStringList&) { /* not used */ }

void Kid3Settings::writeEntry(const QString& key, const QMap<QString, QString>& val)
{
	QMap<QString, QString>::ConstIterator it;
	for (it = val.begin(); it != val.end(); ++it) {
		m_map[m_group + key + it.key()] = it.data();
	}
}

QString Kid3Settings::readEntry(const QString& key, const QString& dflt)
{
	return m_map.contains(m_group + key) ? m_map[m_group + key] : dflt;
}

int Kid3Settings::readNumEntry(const QString& key, int dflt)
{
	return m_map.contains(m_group + key) ? m_map[m_group + key].toInt() : dflt;
}

bool Kid3Settings::readBoolEntry(const QString& key, bool dflt)
{
	return m_map.contains(m_group + key) ? (m_map[m_group + key] == "true") : dflt;
}

QStringList Kid3Settings::readListEntry(const QString&) { /* not used */ return QStringList(); }

QMap<QString, QString> Kid3Settings::readMapEntry(const QString& key, const QMap<QString, QString>& dflt)
{
	bool found = false;
	QMap<QString, QString> val;
	QMap<QString, QString>::Iterator it;
	for (it = m_map.begin(); it != m_map.end(); ++it) {
		if (it.key().QCM_indexOf(m_group + key) == 0) {
			found = true;
			val[it.key().mid((m_group + key).length())] = it.data();
		}
	}
	return found ? val : dflt;
}

bool Kid3Settings::removeEntry(const QString& key)
{
	m_map.erase(key);
	return true;
} 
#endif
#endif

/**
 * Constructor.
 * Set default configuration.
 *
 * @param grp configuration group
 */
GeneralConfig::GeneralConfig(const QString& grp) : m_group(grp) {}

/**
 * Destructor.
 */
GeneralConfig::~GeneralConfig() {}
