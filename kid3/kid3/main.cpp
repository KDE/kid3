/**
 * \file main.cpp
 * Main program.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 9 Jan 2003
 */

#include "config.h"
#ifdef CONFIG_USE_KDE

#include <kapp.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

#include "kid3.h"

/** Description for application */
static const char *description = I18N_NOOP("Kid3 ID3 Tagger");

/** Command line options */
static KCmdLineOptions options[] =
{
	{ "+[Dir]", I18N_NOOP("directory to open"), 0 },
	{ 0, 0, 0 }
};

/**
 * Main program.
 *
 * @param argc number of arguments including command name
 * @param argv arguments, argv[0] is command name
 *
 * @return exit code of application.
 */

int main(int argc, char *argv[])
{
	KAboutData aboutData(
	    "kid3", I18N_NOOP("Kid3"),
	    VERSION, description, KAboutData::License_GPL,
	    "(c) 2003, Urs Fleisch", 0, "http://kid3.sourceforge.net",
	    "ufleisch@users.sourceforge.net");
	aboutData.addAuthor("Urs Fleisch",0, "ufleisch@users.sourceforge.net");
	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options);
	KApplication app;

	if (app.isRestored()) {
		RESTORE(Kid3App);
	}
	else {
		Kid3App *kid3 = new Kid3App();
		if (kid3) {
			kid3->show();

			KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

			if (args->count()) {
				kid3->openDirectory(args->arg(0));
			}
			args->clear();
		}
	}

	return app.exec();
}

#else

#include <qapplication.h>
#include <qtextcodec.h>

#include "kid3.h"

/**
 * Main program.
 *
 * @param argc number of arguments including command name
 * @param argv arguments, argv[0] is command name
 *
 * @return exit code of application.
 */

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	// translation file for Qt
	QTranslator qt_tr(0);
	qt_tr.load( QString("qt_") + QTextCodec::locale(), "." );
	app.installTranslator(&qt_tr);

	// translation file for application strings
	QTranslator kid3_tr(0);
	kid3_tr.load( QString("kid3_") + QTextCodec::locale(), "." );
	app.installTranslator(&kid3_tr);

	Kid3App *kid3 = new Kid3App();
	if (kid3) {
		kid3->show();
		app.setMainWidget(kid3);
		if (argc > 1) {
			kid3->openDirectory(argv[1]);
		}
	}
	return app.exec();
}

#endif
