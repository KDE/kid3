/**
 * \file imageviewer.h
 * Window to view image.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 10 Jun 2009
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

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <qdialog.h>

class QImage;
class QLabel;

/** Window to view image */
class ImageViewer : public QDialog {
Q_OBJECT
public:
	/**
	 * Constructor.
	 *
	 * @param parent parent widget
	 * @param img    image to display in window
	 */
	ImageViewer(QWidget* parent, const QImage& img);

	/**
	 * Destructor.
	 */
	virtual ~ImageViewer() {}

private:
	/** image to view */
	QLabel* m_image; 
};

#endif // IMAGEVIEWER_H
