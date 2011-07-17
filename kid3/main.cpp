/**
 * \file main.cpp
 * Main program.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 *
 * Copyright (C) 2003-2010  Urs Fleisch
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
#include <QFile>
#ifdef CONFIG_USE_KDE

#include <kdeversion.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

#include "kid3mainwindow.h"

/** Description for application */
static const char* description = I18N_NOOP("Kid3 ID3 Tagger");

/**
 * Main program.
 *
 * @param argc number of arguments including command name
 * @param argv arguments, argv[0] is command name
 *
 * @return exit code of application.
 */

int main(int argc, char* argv[])
{
	KAboutData aboutData(
		"kid3", 0, ki18n("Kid3"),
		VERSION, ki18n(description), KAboutData::License_GPL,
		ki18n("(c) 2003-2011 Urs Fleisch"), KLocalizedString(), "http://kid3.sourceforge.net",
		"ufleisch@users.sourceforge.net");
	aboutData.addAuthor(ki18n("Urs Fleisch"), KLocalizedString(), "ufleisch@users.sourceforge.net");
	KCmdLineArgs::init(argc, argv, &aboutData);

	KCmdLineOptions options;
	options.add("+[Dir]", ki18n("directory to open"));
	KCmdLineArgs::addCmdLineOptions(options);
	KApplication app;

	if (app.isSessionRestored()) {
		RESTORE(Kid3MainWindow);
	}
	else {
		Kid3MainWindow* kid3 = new Kid3MainWindow;
		if (kid3) {
			kid3->show();

			KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

			if (args->count()) {
				kid3->confirmedOpenDirectory(args->arg(0));
			}
			args->clear();
		}
	}

	return app.exec();
}

#else

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDir>

#include "kid3mainwindow.h"

/**
 * Main program.
 *
 * @param argc number of arguments including command name
 * @param argv arguments, argv[0] is command name
 *
 * @return exit code of application.
 */

int main(int argc, char* argv[])
{
	Q_INIT_RESOURCE(kid3);

	QApplication app(argc, argv);
	app.setApplicationName("Kid3");
	QString locale(QLocale::system().name());

	// translation file for Qt
	QTranslator qt_tr(0);
#ifdef CFG_TRANSLATIONSDIR
	if (!qt_tr.load(QString("qt_") + locale, CFG_TRANSLATIONSDIR))
#endif
	qt_tr.load( QString("qt_") + locale, "." );
	app.installTranslator(&qt_tr);

	// translation file for application strings
	QTranslator kid3_tr(0);
#ifdef CFG_TRANSLATIONSDIR
	if (!kid3_tr.load(QString("kid3_") + locale, CFG_TRANSLATIONSDIR))
#endif
	kid3_tr.load( QString("kid3_") + locale, "." );
	app.installTranslator(&kid3_tr);

#ifdef __APPLE__
 QDir dir(QApplication::applicationDirPath());
 dir.cdUp();
 dir.cd("PlugIns");
 QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

	Kid3MainWindow* kid3 = new Kid3MainWindow;
	if (kid3) {
		kid3->show();
		if (argc > 1) {
			kid3->confirmedOpenDirectory(QFile::decodeName(argv[1]));
		}
	}
	return app.exec();
}

#endif
