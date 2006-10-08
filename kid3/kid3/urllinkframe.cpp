/**
 * \file urllinkframe.cpp
 * URL link frame for TagLib.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 01 Oct 2006
 */

#include "urllinkframe.h"

#if defined HAVE_TAGLIB && !defined TAGLIB_SUPPORTS_URLLINK_FRAMES

class UrlLinkFrame::UrlLinkFramePrivate {
public:
	TagLib::String url;
};

class UserUrlLinkFrame::UserUrlLinkFramePrivate {
public:
  UserUrlLinkFramePrivate() : textEncoding(TagLib::String::Latin1) {}
  TagLib::String::Type textEncoding;
	TagLib::String description;
};

UrlLinkFrame::UrlLinkFrame(const TagLib::ByteVector &data) :
	Frame(data)
{
	d = new UrlLinkFramePrivate;
	setData(data);
}

UrlLinkFrame::~UrlLinkFrame()
{
	delete d;
}

void UrlLinkFrame::setUrl(const TagLib::String &s)
{
	d->url = s;
}

TagLib::String UrlLinkFrame::url() const
{
	return d->url;
}

void UrlLinkFrame::setText(const TagLib::String &s)
{
	setUrl(s);
}

TagLib::String UrlLinkFrame::toString() const
{
	return url();
}

void UrlLinkFrame::parseFields(const TagLib::ByteVector &data)
{
	d->url = TagLib::String(data);
}

TagLib::ByteVector UrlLinkFrame::renderFields() const
{
	return d->url.data(TagLib::String::Latin1);
}

UrlLinkFrame::UrlLinkFrame(const TagLib::ByteVector &data, Header *h) : Frame(h)
{
  d = new UrlLinkFramePrivate;
  parseFields(fieldData(data));
}


UserUrlLinkFrame::UserUrlLinkFrame(TagLib::String::Type encoding) :
	UrlLinkFrame("WXXX")
{
	d = new UserUrlLinkFramePrivate;
	d->textEncoding = encoding;
}

UserUrlLinkFrame::UserUrlLinkFrame(const TagLib::ByteVector &data) :
	UrlLinkFrame(data)
{
	d = new UserUrlLinkFramePrivate;
	setData(data);
}

UserUrlLinkFrame::~UserUrlLinkFrame()
{
	delete d;
}

TagLib::String UserUrlLinkFrame::toString() const
{
  return "[" + description() + "] " + url();
}

TagLib::String::Type UserUrlLinkFrame::textEncoding() const
{
	return d->textEncoding;
}

void UserUrlLinkFrame::setTextEncoding(TagLib::String::Type encoding)
{
	d->textEncoding = encoding;
}

TagLib::String UserUrlLinkFrame::description() const
{
	return d->description;
}

void UserUrlLinkFrame::setDescription(const TagLib::String &s)
{
	d->description = s;
}

void UserUrlLinkFrame::parseFields(const TagLib::ByteVector &data)
{
  if (data.size() < 2) {
    debug("A user URL link frame must contain at least 2 bytes.");
    return;
  }

  int pos = 0;

	d->textEncoding = TagLib::String::Type(data[0]);
	pos += 1;

	if (d->textEncoding == TagLib::String::Latin1 || d->textEncoding == TagLib::String::UTF8) {
		int offset = data.find(textDelimiter(d->textEncoding), pos);
		if (offset < pos)
			return;

		d->description = TagLib::String(data.mid(pos, offset - pos), d->textEncoding);
		pos = offset + 1;
	} else {
		int len = data.mid(pos).find(textDelimiter(d->textEncoding), 0, 2);
		if (len < 0)
			return;

		d->description = TagLib::String(data.mid(pos, len), d->textEncoding);
		pos += len + 2;
	}

	setUrl(TagLib::String(data.mid(pos)));
}

TagLib::ByteVector UserUrlLinkFrame::renderFields() const
{
	TagLib::ByteVector v;

	v.append(char(d->textEncoding));
	v.append(d->description.data(d->textEncoding));
	v.append(textDelimiter(d->textEncoding));
	v.append(url().data(TagLib::String::Latin1));

	return v;
}

UserUrlLinkFrame::UserUrlLinkFrame(const TagLib::ByteVector &data, Header *h) : UrlLinkFrame(data, h)
{
  d = new UserUrlLinkFramePrivate;
  parseFields(fieldData(data));
}

#endif // HAVE_TAGLIB && !TAGLIB_SUPPORTS_URLLINK_FRAMES
