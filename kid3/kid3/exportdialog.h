/**
 * \file exportdialog.h
 * Export dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 May 2006
 */

#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <qdialog.h>
#include <qstringlist.h>
#include "importtrackdata.h"

class QTextEdit;
class QLineEdit;
class QPushButton;
class QComboBox;

/**
 * Export dialog.
 */
class ExportDialog : public QDialog {
Q_OBJECT

public:
	/** Export source */
	enum Source { SrcV1, SrcV2 };

	/**
	 * Constructor.
	 *
	 * @param parent  parent widget
	 */
	ExportDialog(QWidget* parent);

	/**
	 * Destructor.
	 */
	~ExportDialog();

	/**
	 * Set export format.
	 *
	 * @param names    export format names list
	 * @param headers  export format headers
	 * @param tracks   export format tracks
	 * @param trailers export format trailers
	 * @param idx      selected index
	 */
	void setExportFormat(const QStringList& names,
											 const QStringList& headers,
											 const QStringList& tracks,
											 const QStringList& trailers,
											 int idx);
	/**
	 * Get export format.
	 *
	 * @param name    export format name
	 * @param header  export format header
	 * @param track   export format track
	 * @param trailer export format trailer
	 *
	 * @return index of current selection.
	 */
	int getExportFormat(QString& name,
											QString& header,
											QString& track,
											QString& trailer) const;

	/**
	 * Set ID3v1 or ID3v2 tags as export source.
	 *
	 * @param v1 true to set ID3v1, false for ID3v2
	 */
	void setSrcV1(bool v1);

	/**
	 * Get export source.
	 *
	 * @return true if ID3v1 is source,
	 *         false if ID3v2.
	 */
	bool getSrcV1() const;

	/**
	 * Set size of window.
	 *
	 * @param width  width
	 * @param height height
	 */
	void setWindowSize(int width, int height);

	/**
	 * Get size of window.
	 *
	 * @param width  the width is returned here
	 * @param height the height is returned here
	 */
	void getWindowSize(int& width, int& height) const;

	/**
	 * Set data to be exported.
	 *
	 * @param trackDataVector data to export
	 */
	void setExportData(const ImportTrackDataVector& trackDataVector);

signals:
	/**
	 * Emitted when new export data has to be provided
	 * Parameter: SrcV1 for ID3v1 data, SrcV2 for ID3v2 data
	 */
	void exportDataRequested(int);

private slots:
	/**
	 * Export to a file.
	 */
	void slotToFile();

	/**
	 * Export to clipboard.
	 */
	void slotToClipboard();

	/**
	 * Set the format lineedits to the format selected in the combo box.
	 *
	 * @param index current index of the combo box
	 */
	void setFormatLineEdit(int index);

	/**
	 * Show exported text as preview in editor.
	 */
	void showPreview();

	/**
	 * Save the size of the window and close it.
	 */
	void saveWindowSizeAndClose();

private:
  /** Text editor */
  QTextEdit* m_edit;
	/** cobobox with formats */
	QComboBox* m_formatComboBox;
	/** LineEdit for header */
	QLineEdit* m_headerLineEdit;
	/** LineEdit for track */
	QLineEdit* m_trackLineEdit;
	/** LineEdit for trailer */
	QLineEdit* m_trailerLineEdit;
	/** To File button */
	QPushButton* m_fileButton;
	/** To Clipboard button */
	QPushButton* m_clipButton;
	/** combobox with export sources */
	QComboBox* m_srcComboBox;
	/** header formats */
	QStringList m_formatHeaders;
	/** track formats */
	QStringList m_formatTracks;
	/** trailer formats */
	QStringList m_formatTrailers;

	/** data to export */
	ImportTrackDataVector m_trackDataVector;
	/** width of window */
	int m_windowWidth;
	/** height of window */
	int m_windowHeight;
};

#endif
