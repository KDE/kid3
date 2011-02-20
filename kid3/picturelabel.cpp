/**
 * \file picturelabel.cpp
 * Label for picture preview.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 04 Jan 2009
 */

#include "picturelabel.h"
#include "qtcompatmac.h"
#include <QHash>

/**
 * Constructor.
 *
 * @param parent parent widget
 */
PictureLabel::PictureLabel(QWidget* parent) : QLabel(parent), m_pixmapHash(0)
{
	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	setWordWrap(true);
	clearPicture();
}

/**
 * Destructor.
 */
PictureLabel::~PictureLabel()
{
}

/**
 * Get preferred height for a given width.
 * @return height.
 */
int PictureLabel::heightForWidth(int w) const
{
	return w;
}

/**
 * Set picture.
 */
void PictureLabel::setPicture()
{
	setMargin(0);
	setPixmap(m_pixmap);
}

/**
 * Clear picture.
 */
void PictureLabel::clearPicture()
{
	const char* const msg = I18N_NOOP("Drag album\nartwork\nhere");
	setMargin(6);
	setText(QCM_translate(msg));
}

/**
 * Set picture data.
 *
 * @param data picture data, 0 if no picture is available
 */
void PictureLabel::setData(const QByteArray* data)
{
	if (data && !data->isEmpty()) {
		uint hash = qHash(*data);
		if (hash != m_pixmapHash) {
			// creating new pixmap
			if (m_pixmap.loadFromData(*data)) {
				m_pixmap = m_pixmap.scaled(width(), height(), Qt::KeepAspectRatio);
				m_pixmapHash = hash;
				setPicture();
			} else {
				clearPicture();
			}
		} else {
			if (!pixmap()) {
				// using cached pixmap
				setPicture();
			}
			// else keeping current pixmap
		}
	} else {
		clearPicture();
	}
}
