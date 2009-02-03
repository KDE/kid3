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
#if QT_VERSION >= 0x040000
#include <QHash>
#else
#include <qimage.h>

/**
 * ELF hash function.
 *
 * @param key bytes for which hash is calculated
 *
 * @return hash value for key.
 */
static uint qHash(const QByteArray& key)
{
	uint hash = 0, x = 0;
	for (uint i = 0; i < key.size(); ++i) {
		hash = (hash << 4) + key[i];
		if ((x = hash & 0xF0000000L) != 0) {
			hash ^= x >> 24;
		}
		hash &= ~x;
	}
	return hash;
}

#endif

/**
 * Constructor.
 *
 * @param parent parent widget
 */
PictureLabel::PictureLabel(QWidget* parent) : QLabel(parent), m_pixmapHash(0)
{
#if QT_VERSION >= 0x040000
	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	setWordWrap(true);
#else
	setAlignment(Qt::AlignHCenter | Qt::AlignVCenter | Qt::WordBreak);
#endif
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
#if QT_VERSION >= 0x040000
			if (m_pixmap.loadFromData(*data)) {
				m_pixmap = m_pixmap.scaled(width(), height(), Qt::KeepAspectRatio);
				m_pixmapHash = hash;
				setPicture();
			} else {
				clearPicture();
			}
#else
			QImage image;
			if (image.loadFromData(*data)) {
				m_pixmap.convertFromImage(image.scale(width(), height(),
				                                      QImage::ScaleMin));
				m_pixmapHash = hash;
				setPicture();
			} else {
				clearPicture();
			}
#endif
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
