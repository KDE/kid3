/**
 * \file pictureframe.cpp
 * Frame containing picture.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 03 Mar 2008
 *
 * Copyright (C) 2008-2009  Urs Fleisch
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

#include "pictureframe.h"
#include <qfile.h>
#include <qimage.h>
#include <qbuffer.h>
#if QT_VERSION < 0x040000 && defined CONFIG_USE_KDE
#include <kmdcodec.h>
#endif
#include "qtcompatmac.h"

/**
 * Constructor.
 *
 * @param data        binary picture data
 * @param description description
 * @param pictureType picture type
 * @param mimeType    MIME type
 * @param enc         text encoding
 * @param imgFormat   image format
 */
PictureFrame::PictureFrame(
	const QByteArray& data,
	const QString& description,
	PictureType pictureType,
	const QString& mimeType,
	Field::TextEncoding enc,
	const QString& imgFormat)
{
	setType(FT_Picture);
	setFields(*this, enc, imgFormat, mimeType, pictureType, description, data);
}

/**
 * Constructor.
 *
 * @param frame general frame
 */
PictureFrame::PictureFrame(const Frame& frame)
{
	*(static_cast<Frame*>(this)) = frame;
	setType(FT_Picture);

	// Make sure all fields are available in the correct order
	Field::TextEncoding enc = Field::TE_ISO8859_1;
	PictureType pictureType = PT_CoverFront;
	QString imgFormat("JPG"), mimeType("image/jpeg"), description;
	QByteArray data;
	getFields(*this, enc, imgFormat, mimeType, pictureType, description, data);
	setFields(*this, enc, imgFormat, mimeType, pictureType, description, data);
}

/**
 * Destructor.
 */
PictureFrame::~PictureFrame()
{
}

/**
 * Set all properties.
 *
 * @param frame       frame to set
 * @param enc         text encoding
 * @param imgFormat   image format
 * @param mimeType    MIME type
 * @param pictureType picture type
 * @param description description
 * @param data        binary picture data
 */
void PictureFrame::setFields(Frame& frame,
														 Field::TextEncoding enc, const QString& imgFormat,
														 const QString& mimeType, PictureType pictureType,
														 const QString& description, const QByteArray& data)
{
	Field field;
	FieldList& fields = frame.fieldList();
	fields.clear();

	field.m_id = Field::ID_TextEnc;
	field.m_value = enc;
	fields.push_back(field);

	field.m_id = Field::ID_ImageFormat;
	field.m_value = imgFormat;
	fields.push_back(field);

	field.m_id = Field::ID_MimeType;
	field.m_value = mimeType;
	fields.push_back(field);

	field.m_id = Field::ID_PictureType;
	field.m_value = pictureType;
	fields.push_back(field);

	field.m_id = Field::ID_Description;
	field.m_value = description;
	fields.push_back(field);

	field.m_id = Field::ID_Data;
	field.m_value = data;
	fields.push_back(field);

	frame.setValue(description);
}

/**
 * Get all properties.
 * Unavailable fields are not set.
 *
 * @param frame       frame to get
 * @param enc         text encoding
 * @param imgFormat   image format
 * @param mimeType    MIME type
 * @param pictureType picture type
 * @param description description
 * @param data        binary picture data
 */
void PictureFrame::getFields(const Frame& frame,
														 Field::TextEncoding& enc, QString& imgFormat,
														 QString& mimeType, PictureType& pictureType,
														 QString& description, QByteArray& data)
{
	for (Frame::FieldList::const_iterator it = frame.getFieldList().begin();
			 it != frame.getFieldList().end();
			 ++it) {
		switch ((*it).m_id) {
			case Field::ID_TextEnc:
				enc = static_cast<Field::TextEncoding>((*it).m_value.toInt());
				break;
			case Field::ID_ImageFormat:
				imgFormat = (*it).m_value.toString();
				break;
			case Field::ID_MimeType:
				mimeType = (*it).m_value.toString();
				break;
			case Field::ID_PictureType:
				pictureType = static_cast<PictureType>((*it).m_value.toInt());
				break;
			case Field::ID_Description:
				description = (*it).m_value.toString();
				break;
			case Field::ID_Data:
				data = (*it).m_value.toByteArray();
				break;
			default:
				qDebug("Unknown picture field ID");
		}
	}
}

/**
 * Set value of a field.
 *
 * @param frame frame to set
 * @param id    field ID
 * @param value field value
 *
 * @return true if field found and set.
 */
bool PictureFrame::setField(Frame& frame, Field::Id id, const QVariant& value)
{
	for (Frame::FieldList::iterator it = frame.fieldList().begin();
			 it != frame.fieldList().end();
			 ++it) {
		if ((*it).m_id == id) {
			(*it).m_value = value;
			if (id == Field::ID_Description) frame.setValue(value.toString());
			return true;
		}
	}
	return false;
}

/**
 * Get value of a field.
 *
 * @param frame frame to get
 * @param id    field ID
 *
 * @return field value, invalid if not found.
 */
QVariant PictureFrame::getField(const Frame& frame, Field::Id id)
{
	QVariant result;
	if (!frame.getFieldList().empty()) {
		for (Frame::FieldList::const_iterator it = frame.getFieldList().begin();
				 it != frame.getFieldList().end();
				 ++it) {
			if ((*it).m_id == id) {
				result = (*it).m_value;
				break;
			}
		}
	}
	return result;
}

/**
 * Set text encoding.
 *
 * @param frame frame to set
 * @param enc   text encoding
 *
 * @return true if field found and set.
 */
bool PictureFrame::setTextEncoding(Frame& frame, Field::TextEncoding enc)
{
	return setField(frame, Field::ID_TextEnc, enc);
}

/**
 * Get text encoding.
 *
 * @param frame frame to get
 * @param enc   the text encoding is returned here
 *
 * @return true if field found.
 */
bool PictureFrame::getTextEncoding(const Frame& frame, Field::TextEncoding& enc)
{
	QVariant var(getField(frame, Field::ID_TextEnc));
	if (var.isValid()) {
		enc = static_cast<Field::TextEncoding>(var.toInt());
		return true;
	}
	return false;
}

/**
 * Set image format.
 *
 * @param frame     frame to set
 * @param imgFormat image format
 *
 * @return true if field found and set.
 */
bool PictureFrame::setImageFormat(Frame& frame, const QString& imgFormat)
{
	return setField(frame, Field::ID_ImageFormat, imgFormat);
}

/**
 * Get image format.
 *
 * @param frame     frame to get
 * @param imgFormat the image format is returned here
 *
 * @return true if field found.
 */
bool PictureFrame::getImageFormat(const Frame& frame, QString& imgFormat)
{
	QVariant var(getField(frame, Field::ID_ImageFormat));
	if (var.isValid()) {
		imgFormat = var.toString();
		return true;
	}
	return false;
}

/**
 * Set MIME type.
 *
 * @param frame   frame to set
 * @param mimeType MIME type
 *
 * @return true if field found and set.
 */
bool PictureFrame::setMimeType(Frame& frame, const QString& mimeType)
{
	return setField(frame, Field::ID_MimeType, mimeType);
}

/**
 * Get MIME type.
 *
 * @param frame    frame to get
 * @param mimeType the MIME type is returned here
 *
 * @return true if field found.
 */
bool PictureFrame::getMimeType(const Frame& frame, QString& mimeType)
{
	QVariant var(getField(frame, Field::ID_MimeType));
	if (var.isValid()) {
		mimeType = var.toString();
		return true;
	}
	return false;
}

/**
 * Set picture type.
 *
 * @param frame       frame to set
 * @param pictureType picture type
 *
 * @return true if field found and set.
 */
bool PictureFrame::setPictureType(Frame& frame, PictureType pictureType)
{
	return setField(frame, Field::ID_PictureType, pictureType);
}

/**
 * Get picture type.
 *
 * @param frame       frame to get
 * @param pictureType the picture type is returned here
 *
 * @return true if field found.
 */
bool PictureFrame::getPictureType(const Frame& frame, PictureType& pictureType)
{
	QVariant var(getField(frame, Field::ID_PictureType));
	if (var.isValid()) {
		pictureType = static_cast<PictureType>(var.toInt());
		return true;
	}
	return false;
}

/**
 * Set description.
 *
 * @param frame       frame to set
 * @param description description
 *
 * @return true if field found and set.
 */
bool PictureFrame::setDescription(Frame& frame, const QString& description)
{
	return setField(frame, Field::ID_Description, description);
}

/**
 * Get description.
 *
 * @param frame       frame to get
 * @param description the description is returned here
 *
 * @return true if field found.
 */
bool PictureFrame::getDescription(const Frame& frame, QString& description)
{
	QVariant var(getField(frame, Field::ID_Description));
	if (var.isValid()) {
		description = var.toString();
		return true;
	}
	return false;
}

/**
 * Set binary data.
 *
 * @param frame frame to set
 * @param data  binary data
 *
 * @return true if field found and set.
 */
bool PictureFrame::setData(Frame& frame, const QByteArray& data)
{
	return setField(frame, Field::ID_Data, data);
}

/**
 * Get binary data.
 *
 * @param frame frame to get
 * @param data  the binary data is returned here
 *
 * @return true if field found.
 */
bool PictureFrame::getData(const Frame& frame, QByteArray& data)
{
	QVariant var(getField(frame, Field::ID_Data));
	if (var.isValid()) {
		data = var.toByteArray();
		return true;
	}
	return false;
}

/**
 * Read binary data from file.
 *
 * @param frame frame to set
 * @param fileName name of data file
 *
 * @return true if file read, field found and set.
 */
bool PictureFrame::setDataFromFile(Frame& frame, const QString& fileName)
{
	bool result = false;
	if (!fileName.isEmpty()) {
		QFile file(fileName);
		if (file.open(QCM_ReadOnly)) {
			size_t size = file.size();
			char* data = new char[size];
			if (data) {
				QDataStream stream(&file);
				stream.QCM_readRawData(data, size);
				QByteArray ba;
				QCM_duplicate(ba, data, size);
				result = setData(frame, ba);
				delete [] data;
			}
			file.close();
		}
	}
	return result;
}

/**
 * Get binary data from image.
 *
 * @param frame frame to set
 * @param image image
 *
 * @return true if field found and set.
 */
bool PictureFrame::setDataFromImage(Frame& frame, const QImage& image)
{
	QByteArray ba;
#if QT_VERSION >= 0x040000
	QBuffer buffer(&ba);
#else
	QBuffer buffer(ba);
#endif
	buffer.open(QCM_WriteOnly);
	image.save(&buffer, "JPG");
	return setData(frame, ba);
}

/**
 * Save binary data to a file.
 *
 * @param frame    frame
 * @param fileName name of data file to save
 *
 * @return true if field found and saved.
 */
bool PictureFrame::writeDataToFile(const Frame& frame, const QString& fileName)
{
	QByteArray ba;
	if (getData(frame, ba)) {
		QFile file(fileName);
		if (file.open(QCM_WriteOnly)) {
			QDataStream stream(&file);
			stream.QCM_writeRawData(ba.data(), ba.size());
			file.close();
			return true;
		}
	}
	return false;
}

/**
 * Set the MIME type and image format from the file name extension.
 *
 * @param frame frame to set
 * @param fileName name of data file
 *
 * @return true if field found and set.
 */
bool PictureFrame::setMimeTypeFromFileName(Frame& frame, const QString& fileName)
{
	if (fileName.endsWith(".jpg", QCM_CaseInsensitive) ||
			fileName.endsWith(".jpeg", QCM_CaseInsensitive)) {
		return setMimeType(frame, "image/jpeg") && setImageFormat(frame, "JPG");
	} else if (fileName.endsWith(".png", QCM_CaseInsensitive)) {
		return setMimeType(frame, "image/png") && setImageFormat(frame, "PNG");
	}
	return false;
}

#ifdef HAVE_BASE64_ENCODING
/**
 * Get a 32-bit number from a byte array stored in big-endian order.
 *
 * @param data byte array
 * @param index index of first byte in data
 *
 * @return big endian 32-bit value.
 */
static unsigned long getBigEndianULongFromByteArray(const QByteArray& data,
                                                    int index)
{
	return
		 ((unsigned char)data[index + 3] & 0xff)        |
		(((unsigned char)data[index + 2] & 0xff) << 8)  |
		(((unsigned char)data[index + 1] & 0xff) << 16) |
		(((unsigned char)data[index + 0] & 0xff) << 24);
}

/**
 * Render a 32-bit number to a byte array in big-endian order.
 *
 * @param value 32-bit value
 * @param data  byte array
 * @param index index of first byte in data
 */
static void renderBigEndianULongToByteArray(unsigned long value,
                                            QByteArray& data, int index)
{
	data[index + 3] = value & 0xff;
	value >>= 8;
	data[index + 2] = value & 0xff;
	value >>= 8;
	data[index + 1] = value & 0xff;
	value >>= 8;
	data[index + 0] = value & 0xff;
}

/**
 * Copy characters into a byte array.
 *
 * @param str   source string
 * @param data  destination byte array
 * @param index index of first byte in data
 * @param len   number of bytes to copy
 */
static void renderCharsToByteArray(const char* str, QByteArray& data,
                                   int index, int len)
{
	for (int i = 0; i < len; ++i) {
		data[index++] = *str++;
	}
}

/**
 * Set picture from a base64 string.
 *
 * @param frame       frame to set
 * @param base64Value base64 string
 */
void PictureFrame::setFieldsFromBase64(Frame& frame, const QString& base64Value)
{
#if QT_VERSION >= 0x040000
	QByteArray ba = QByteArray::fromBase64(base64Value.toAscii());
#elif defined CONFIG_USE_KDE
	QByteArray ba;
	QCString baBase64(base64Value.ascii());
	KCodecs::base64Decode(baBase64, ba);
#endif
	PictureFrame::PictureType pictureType = PictureFrame::PT_CoverFront;
	QString mimeType("image/jpeg");
	QString description("");
	if (frame.getName(true) == "METADATA_BLOCK_PICTURE") {
		unsigned long baSize = static_cast<unsigned long>(ba.size());
		if (baSize < 32) return;
		int index = 0;
		pictureType = static_cast<PictureFrame::PictureType>(
			getBigEndianULongFromByteArray(ba, index));
		index += 4;
		unsigned long mimeLen = getBigEndianULongFromByteArray(ba, index);
		index += 4;
		if (baSize < index + mimeLen + 24) return;
		mimeType = QString::fromAscii(ba.data() + index, mimeLen);
		index += mimeLen;
		unsigned long descLen = getBigEndianULongFromByteArray(ba, index);
		index += 4;
		if (baSize < index + descLen + 20) return;
		description = QString::fromUtf8(ba.data() + index, descLen);
		index += descLen;
		index += 16; // width, height, depth, number of colors
		unsigned long picLen = getBigEndianULongFromByteArray(ba, index);
		index += 4;
		if (baSize < index + picLen) return;
#if QT_VERSION >= 0x040000
		ba = ba.mid(index);
#else
		ba.duplicate(ba.data() + index, picLen);
#endif
	}
	PictureFrame::setFields(
		frame, Frame::Field::TE_UTF8,	"", mimeType,
		pictureType, description, ba);
}

/**
 * Get picture to a base64 string.
 *
 * @param frame       frame to get
 * @param base64Value base64 string to set
 */
void PictureFrame::getFieldsToBase64(const Frame& frame, QString& base64Value)
{
	Frame::Field::TextEncoding enc;
	PictureFrame::PictureType pictureType = PictureFrame::PT_CoverFront;
	QString imgFormat, mimeType, description;
	QByteArray pic;
	PictureFrame::getFields(frame, enc, imgFormat, mimeType,
	                        pictureType, description, pic);
	if (frame.getName(true) == "METADATA_BLOCK_PICTURE") {
		QCM_QCString mimeStr = mimeType.QCM_toAscii();
		QCM_QCString descStr = description.QCM_toUtf8();
		int mimeLen = mimeStr.length();
		int descLen = descStr.length();
		int picLen = pic.size();
		QByteArray ba(32 + mimeLen + descLen + picLen
#if QT_VERSION >= 0x040000
		               , '\0'
#endif
		               );
		int index = 0;
		renderBigEndianULongToByteArray(pictureType, ba, index);
		index += 4;
		renderBigEndianULongToByteArray(mimeLen, ba, index);
		index += 4;
		renderCharsToByteArray(mimeStr, ba, index, mimeLen);
		index += mimeLen;
		renderBigEndianULongToByteArray(descLen, ba, index);
		index += 4;
		renderCharsToByteArray(descStr, ba, index, descLen);
		index += descLen;

		int width = 0, height = 0, depth = 0, numColors = 0;
		QImage image;
		if (image.loadFromData(pic)) {
			width = image.width();
			height = image.height();
			depth = image.depth();
			numColors = image.numColors();
		}
		renderBigEndianULongToByteArray(width, ba, index);
		index += 4;
		renderBigEndianULongToByteArray(height, ba, index);
		index += 4;
		renderBigEndianULongToByteArray(depth, ba, index);
		index += 4;
		renderBigEndianULongToByteArray(numColors, ba, index);
		index += 4;

		renderBigEndianULongToByteArray(picLen, ba, index);
		index += 4;
		renderCharsToByteArray(pic.data(), ba, index, picLen);
		pic = ba;
	}
#if QT_VERSION >= 0x040000
	base64Value = pic.toBase64();
#elif defined CONFIG_USE_KDE
	QByteArray picBase64;
	KCodecs::base64Encode(pic, picBase64);
	base64Value = QString(picBase64);
#endif
}
#endif // HAVE_BASE64_ENCODING
