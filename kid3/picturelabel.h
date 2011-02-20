/**
 * \file picturelabel.h
 * Label for picture preview.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Jan 2009
 */

#ifndef PICTURELABEL_H
#define PICTURELABEL_H

#include <qlabel.h>
#include <qpixmap.h>
#if QT_VERSION >= 0x040000
#include <QByteArray>
#else
#include <qcstring.h> 
#endif

/**
 * Label for picture preview.
 */
class PictureLabel : public QLabel {
public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 */
	PictureLabel(QWidget* parent);

	/**
	 * Destructor.
	 */
	virtual ~PictureLabel();

	/**
	 * Get preferred height for a given width.
	 * @return height.
	 */
	virtual int heightForWidth(int w) const;

	/**
	 * Set picture data.
	 *
	 * @param data picture data, 0 if no picture is available
	 */
	void setData(const QByteArray* data);

private:
	/**
	 * Set picture.
	 */
	void setPicture();

	/**
	 * Clear picture.
	 */
	void clearPicture();

	QPixmap m_pixmap;
	uint m_pixmapHash;
};

#endif // PICTURELABEL_H
