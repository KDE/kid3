/**
 * \file musicbrainzdialog.cpp
 * MusicBrainz import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Sep 2005
 */

#include "musicbrainzdialog.h"
#ifdef HAVE_TUNEPIMP

#ifdef CONFIG_USE_KDE
#include <klocale.h>
#else
#define i18n(s) tr(s)
#define I18N_NOOP(s) QT_TR_NOOP(s)
#endif

#include <qlayout.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qtable.h>
#include <qlabel.h>
#include <qtimer.h>
#include "kid3.h"
#include "musicbrainzclient.h"

/**
 * Constructor.
 *
 * @param parent          parent widget
 * @param trackDataVector track data to be filled with imported values,
 *                        is passed with filenames set
 */
MusicBrainzDialog::MusicBrainzDialog(QWidget* parent,
																		 ImportTrackDataVector& trackDataVector)
	: QDialog(parent, "musicbrainz", true), m_timer(0), m_client(0),
		m_trackDataVector(trackDataVector)
{
	setCaption(i18n("MusicBrainz"));

	QVBoxLayout* vlayout = new QVBoxLayout(this, 6, 6);
	if (!vlayout) {
		return ;
	}

	QHBoxLayout* serverLayout = new QHBoxLayout(vlayout);
	QLabel* serverLabel = new QLabel(i18n("&Server:"), this);
	m_serverComboBox = new QComboBox(true, this);
	if (serverLayout && serverLabel && m_serverComboBox) {
		static const char *serverList[] = {
			"musicbrainz.org:80",
			"de.musicbrainz.org:80",
			"nl.musicbrainz.org:80",
			0                  // end of StrList
		};
		m_serverComboBox->insertStrList(serverList);
#if QT_VERSION >= 0x030100
		m_serverComboBox->setSizePolicy(
			QSizePolicy::Expanding, QSizePolicy::Minimum);
#else
		m_serverComboBox->setSizePolicy(
			QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
#endif
		serverLabel->setBuddy(m_serverComboBox);
		serverLayout->addWidget(serverLabel);
		serverLayout->addWidget(m_serverComboBox);
		connect(m_serverComboBox, SIGNAL(activated(int)),
						this, SLOT(setClientConfig()));
	}
	m_albumTable = new QTable(this, "albumTable");
	if (m_albumTable) {
		m_albumTable->setNumCols(2);
#if QT_VERSION >= 300
		m_albumTable->setColumnReadOnly(1, true);
		m_albumTable->setFocusStyle(QTable::FollowStyle);
#endif
		m_albumTable->setColumnStretchable(0, true);
		m_albumTable->setSelectionMode(QTable::NoSelection);
		QHeader* hHeader = m_albumTable->horizontalHeader();
		hHeader->setLabel(0, "08 A Not So Short Title/Medium Sized Artist - And The Album Title [2005]");
		hHeader->setLabel(1, "A Not So Short State");
		m_albumTable->adjustColumn(0);
		m_albumTable->adjustColumn(1);
		hHeader->setLabel(0, i18n("Track Title/Artist - Album"));
		hHeader->setLabel(1, i18n("State"));
		initTable();
		vlayout->addWidget(m_albumTable);
	}

	QHBoxLayout *hlayout = new QHBoxLayout(vlayout);
	QSpacerItem *hspacer = new QSpacerItem(16, 0, QSizePolicy::Expanding,
	                                       QSizePolicy::Minimum);
	QPushButton* helpButton = new QPushButton(i18n("&Help"), this);
	QPushButton* saveButton = new QPushButton(i18n("&Save Settings"), this);
	QPushButton* okButton = new QPushButton(i18n("&OK"), this);
	QPushButton* applyButton = new QPushButton(i18n("&Apply"), this);
	QPushButton* cancelButton = new QPushButton(i18n("&Cancel"), this);
	if (hlayout && helpButton && saveButton &&
			okButton && cancelButton && applyButton) {
		hlayout->addWidget(helpButton);
		hlayout->addWidget(saveButton);
		hlayout->addItem(hspacer);
		hlayout->addWidget(okButton);
		hlayout->addWidget(applyButton);
		hlayout->addWidget(cancelButton);
		// auto default is switched off to use the return key to set the server
		// configuration
		okButton->setAutoDefault(false);
		cancelButton->setAutoDefault(false);
		applyButton->setAutoDefault(false);
		connect(helpButton, SIGNAL(clicked()), this, SLOT(showHelp()));
		connect(saveButton, SIGNAL(clicked()), this, SLOT(saveConfig()));
		connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
		connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));
	}
}

/**
 * Destructor.
 */
MusicBrainzDialog::~MusicBrainzDialog()
{
	stopClient();
}

/**
 * Initialize the table.
 * Has to be called before reusing the dialog with new track data.
 */
void MusicBrainzDialog::initTable()
{
	setServer(Kid3App::s_musicBrainzCfg.m_server);

#if QT_VERSION >= 300
	unsigned numRows = m_trackDataVector.size();
#else
	unsigned numRows = m_trackDataVector.count();
#endif
	m_trackResults.resize(numRows);
	m_albumTable->setNumRows(numRows);
	for (unsigned i = 0; i < numRows; ++i) {
#if QT_VERSION >= 300
		QComboTableItem* cti = new QComboTableItem(
			m_albumTable, QStringList(i18n("No result")));
		m_albumTable->setItem(i, 0, cti);
		m_albumTable->setText(i, 1, i18n("Unknown"));
#else
		m_albumTable->setText(i, 0, i18n("Unknown"));
#endif
	}
}

/**
 * Clear all results.
 */
void MusicBrainzDialog::clearResults()
{
#if QT_VERSION >= 300
	unsigned numRows = m_trackDataVector.size();
#else
	unsigned numRows = m_trackDataVector.count();
#endif
	for (unsigned i = 0; i < numRows; ++i) {
		m_trackResults[i].clear();
		setFileStatus(i, i18n("Unknown"));
		updateFileTrackData(i);
	}
}

/**
 * Set the configuration in the client.
 */
void MusicBrainzDialog::setClientConfig()
{
	if (m_client) {
		m_client->setConfig(
			getServer(),
			Kid3App::s_miscCfg.m_proxy, Kid3App::s_miscCfg.m_useProxy);
	}
}

/**
 * Create and start the MusicBrainz client.
 */
void MusicBrainzDialog::startClient()
{
	clearResults();
	if (!m_client) {
		m_client = new MusicBrainzClient(m_trackDataVector);
		setClientConfig();
		connect(m_client, SIGNAL(statusChanged(int, QString)),
						this, SLOT(setFileStatus(int, QString)));
		connect(m_client, SIGNAL(metaDataReceived(int, ImportTrackData&)),
						this, SLOT(setMetaData(int, ImportTrackData&)));
		connect(m_client, SIGNAL(resultsReceived(int, ImportTrackDataVector&)),
						this, SLOT(setResults(int, ImportTrackDataVector&)));
		m_client->addFiles();
	}
	if (!m_timer) {
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), this, SLOT(timerDone()));
	}
	if (m_timer) {
		m_timer->start(1000);
	}
}

/**
 * Stop and destroy the MusicBrainz client.
 */
void MusicBrainzDialog::stopClient()
{
	if (m_timer) {
		m_timer->stop();
		// will be destroyed by parent
	}
	if (m_client) {
		m_client->disconnect();
		delete m_client;
		m_client = 0;
	}
}

/**
 * Hides the dialog and sets the result to QDialog::Accepted.
 */
void MusicBrainzDialog::accept()
{
	apply();
	stopClient();
	QDialog::accept();
}

/**
 * Hides the dialog and sets the result to QDialog::Rejected.
 */
void MusicBrainzDialog::reject()
{
	stopClient();
	QDialog::reject();
}

/**
 * Apply imported data.
 */
void MusicBrainzDialog::apply()
{
	bool newTrackData = false;
#if QT_VERSION >= 300
	unsigned numRows = m_trackDataVector.size();
	for (unsigned index = 0; index < numRows; ++index) {
		QComboTableItem* item =
			dynamic_cast<QComboTableItem*>(m_albumTable->item(index, 0));
		if (item) {
			int selectedItem = item->currentItem();
			if (selectedItem > 0) {
				const ImportTrackData& selectedData =
					m_trackResults[index][selectedItem - 1];
				m_trackDataVector[index].title = selectedData.title;
				m_trackDataVector[index].artist = selectedData.artist;
				m_trackDataVector[index].album = selectedData.album;
				m_trackDataVector[index].track = selectedData.track;
				m_trackDataVector[index].year = selectedData.year;
				m_trackDataVector[index].setImportDuration(
					selectedData.getImportDuration());
				newTrackData = true;
			}
		}
	}
#else
	unsigned numRows = m_trackDataVector.count();
	for (unsigned index = 0; index < numRows; ++index) {
		const ImportTrackData& selectedData =
			m_trackResults[index][0];
		m_trackDataVector[index].title = selectedData.title;
		m_trackDataVector[index].artist = selectedData.artist;
		m_trackDataVector[index].album = selectedData.album;
		m_trackDataVector[index].track = selectedData.track;
		m_trackDataVector[index].year = selectedData.year;
		m_trackDataVector[index].setImportDuration(
			selectedData.getImportDuration());
		newTrackData = true;
	}
#endif
	if (newTrackData) {
		emit trackDataUpdated();
	}
}

/**
 * Shows the dialog as a modal dialog.
 */
int MusicBrainzDialog::exec()
{
	startClient();
	return QDialog::exec();
}

/**
 * Called when the periodic timer times out.
 * Used to poll the MusicBrainz client.
 */
void MusicBrainzDialog::timerDone()
{
	if (m_client) {
		m_client->pollStatus();
	}
}

/**
 * Set the status of a file.
 *
 * @param index  index of file
 * @param status status string
 */
void MusicBrainzDialog::setFileStatus(int index, QString status)
{
	m_albumTable->setText(index, 1, status);
}

/**
 * Update the track data combo box of a file.
 *
 * @param index  index of file
 */
void MusicBrainzDialog::updateFileTrackData(int index)
{
	QStringList stringList;
#if QT_VERSION >= 300
	unsigned numResults = m_trackResults[index].size();
#else
	unsigned numResults = m_trackResults[index].count();
#endif
	QString str(numResults == 0 ?
							i18n("No result") : i18n("No result selected"));
#if QT_VERSION >= 300
	stringList.push_back(str);
#else
	stringList.append(str);
#endif
	for (
#if QT_VERSION >= 300
		ImportTrackDataVector::const_iterator
#else
		ImportTrackDataVector::ConstIterator
#endif
			 it = m_trackResults[index].begin();
			 it != m_trackResults[index].end();
			 ++it) {
		str.sprintf("%02d ", (*it).track);
		str += (*it).title;
		str += '/';
		str += (*it).artist;
		str += " - ";
		str += (*it).album;
		if ((*it).year > 0) {
			str += QString(" [%1]").arg((*it).year);
		}
#if QT_VERSION >= 300
		stringList.push_back(str);
#else
		stringList.append(str);
#endif
	}
#if QT_VERSION >= 300
	QComboTableItem* item =
		dynamic_cast<QComboTableItem*>(m_albumTable->item(index, 0));
	if (item) {
		item->setStringList(stringList);
		// if there is only one result, select it, else let the user select
		if (numResults == 1) {
			item->setCurrentItem(1);
		}
	}
#else
	m_albumTable->setText(index, 0, stringList.first());
#endif
}

/**
 * Set meta data for a file.
 *
 * @param index     index of file
 * @param trackData meta data
 */
void MusicBrainzDialog::setMetaData(int index, ImportTrackData& trackData)
{
	m_trackResults[index].clear();
#if QT_VERSION >= 300
	m_trackResults[index].push_back(trackData);
#else
	m_trackResults[index].append(trackData);
#endif
	updateFileTrackData(index);
}

/**
 * Set result list for a file.
 *
 * @param index           index of file
 * @param trackDataVector result list
 */
void MusicBrainzDialog::setResults(
	int index, ImportTrackDataVector& trackDataVector)
{
	m_trackResults[index] = trackDataVector;
	updateFileTrackData(index);
	for (
#if QT_VERSION >= 300
		 ImportTrackDataVector::const_iterator
#else
		 ImportTrackDataVector::ConstIterator
#endif
			 it = trackDataVector.begin();
			 it != trackDataVector.end();
			 ++it) {
	}
}

/**
 * Get string with server and port.
 *
 * @return "servername:port".
 */
QString MusicBrainzDialog::getServer() const
{
	QString server(m_serverComboBox->currentText());
	if (server.isEmpty()) {
		server = "musicbrainz.org:80";
	}
	return server;
}

/**
 * Set string with server and port.
 *
 * @param srv "servername:port"
 */
void MusicBrainzDialog::setServer(const QString& srv)
{
#if QT_VERSION >= 300
	m_serverComboBox->setCurrentText(srv);
#else
	m_serverComboBox->setEditText(srv);
#endif
}

/**
 * Save the local settings to the configuration.
 */
void MusicBrainzDialog::saveConfig()
{
	Kid3App::s_musicBrainzCfg.m_server = getServer();
}

/**
 * Show help.
 */
void MusicBrainzDialog::showHelp()
{
	Kid3App::displayHelp("import-musicbrainz");
}

#else // HAVE_TUNEPIMP

MusicBrainzDialog::MusicBrainzDialog(QWidget*, ImportTrackDataVector&) {}
MusicBrainzDialog::~MusicBrainzDialog() {}
int MusicBrainzDialog::exec() { return 0; }
void MusicBrainzDialog::accept() {}
void MusicBrainzDialog::reject() {}
void MusicBrainzDialog::setClientConfig() {}
void MusicBrainzDialog::timerDone() {}
void MusicBrainzDialog::apply() {}
void MusicBrainzDialog::setFileStatus(int, QString) {}
void MusicBrainzDialog::updateFileTrackData(int) {}
void MusicBrainzDialog::setMetaData(int, ImportTrackData&) {}
void MusicBrainzDialog::setResults(int, ImportTrackDataVector&) {}
void MusicBrainzDialog::saveConfig() {}
void MusicBrainzDialog::showHelp() {}

#endif // HAVE_TUNEPIMP
