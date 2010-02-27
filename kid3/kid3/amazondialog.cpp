/**
 * \file amazondialog.cpp
 * Amazon database import dialog.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 13 Dec 2009
 *
 * Copyright (C) 2009  Urs Fleisch
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

#include <qregexp.h>
#include <qdom.h>
#include "kid3.h"
#include "amazonclient.h"
#include "amazondialog.h"

static const char* serverList[] = {
	// Parsing only works with English text
	"www.amazon.com:80",
	"www.amazon.co.uk:80",
	0                  // end of StrList
};

static const ImportSourceDialog::Properties props = {
	serverList,
	"www.amazon.com:80",
	0,
	"import-amazon",
	&Kid3App::s_amazonCfg,
	true
};


/**
 * Constructor.
 *
 * @param parent          parent widget
 * @param trackDataVector track data to be filled with imported values
 */
AmazonDialog::AmazonDialog(
	QWidget* parent,
	ImportTrackDataVector& trackDataVector)
	: ImportSourceDialog(parent, "Amazon", trackDataVector,
											 new AmazonClient, props)
{
}

/**
 * Destructor.
 */
AmazonDialog::~AmazonDialog()
{
}

/**
 * Replace HTML entities in a string.
 *
 * @param str string with HTML entities (e.g. &quot;)
 *
 * @return string with replaced HTML entities.
 */
static QString replaceHtmlEntities(QString str)
{
	str.replace("&quot;", "\"");
	str.replace("&nbsp;", " ");
	str.replace("&lt;", "<");
	str.replace("&gt;", ">");
	str.replace("&amp;", "&");
	return str;
}

/**
 * Replace HTML entities and remove HTML tags.
 *
 * @param str string containing HTML
 *
 * @return clean up string
 */
static QString removeHtml(QString str)
{
	QRegExp htmlTagRe("<[^>]+>");
	return replaceHtmlEntities(str.remove(htmlTagRe)).QCM_trimmed();
}

/**
 * Process finished findCddbAlbum request.
 *
 * @param searchStr search data received
 */
void AmazonDialog::parseFindResults(const QByteArray& searchStr)
{
	/* products have the following format (depending on browser):
<td class="dataColumn"><table cellpadding="0" cellspacing="0" border="0"><tr><td>                      
<a href="http://www.amazon.com/Avenger-Amon-Amarth/dp/B001VROVHO/ref=sr_1_1/178-1209985-8853325?ie=UTF8&s=music&qid=1260707733&sr=1-1"><span class="srTitle">The Avenger</span></a>                           
   by <a href="/Amon-Amarth/e/B000APIBHO/ref=sr_ntt_srch_lnk1/178-1209985-8853325?_encoding=UTF8&amp;qid=1260707733&amp;sr=1-1">Amon Amarth</a> <span class="bindingBlock">(<span class="binding">Audio CD</span> - 2009)</span> - <span class="formatText">Original recording reissued</span></td></tr>             
<td></td>                                                                                              
	   or:
<div class="productTitle"><a href="http://www.amazon.com/Avenger-Amon-Amarth/dp/B001VROVHO/ref=sr_1_1?ie=UTF8&s=music&qid=1260607141&sr=1-1"> The Avenger</a> <span class="ptBrand">by <a href="/Amon-Amarth/e/B000APIBHO/ref=sr_ntt_srch_lnk_1?_encoding=UTF8&amp;qid=1260607141&amp;sr=1-1">Amon Amarth</a></span><span class="binding"> (<span class="format">Audio CD</span> - 2009)</span> - <span class="format">Original recording reissued</span></div>
	 */
	QString str = QString::fromLatin1(searchStr);
	QRegExp catIdTitleArtistRe(
		"<a href=\"[^\"]+/(dp|ASIN|images|product|-)/([A-Z0-9]+)[^\"]+\">"
		"<span class=\"srTitle\">([^<]+)<.*>\\s*by\\s*(?:<[^>]+>)?([^<]+)<");
	QStringList lines = QCM_split(QRegExp("\\n{2,}"), str.remove('\r'));
	m_albumListBox->clear();
	for (QStringList::const_iterator it = lines.begin(); it != lines.end(); ++it) {
		QString line(*it);
		line.remove('\n');
		if (catIdTitleArtistRe.QCM_indexIn(line) != -1) {
			new AlbumListItem(
				m_albumListBox,
				removeHtml(catIdTitleArtistRe.cap(4)) + " - " +
				removeHtml(catIdTitleArtistRe.cap(3)),
				catIdTitleArtistRe.cap(1),
				catIdTitleArtistRe.cap(2));
		}
	}
	m_albumListBox->setFocus();
}

/**
 * Parse result of album request and populate m_trackDataVector with results.
 *
 * @param albumStr album data received
 */
void AmazonDialog::parseAlbumResults(const QByteArray& albumStr)
{
	/*
		title (empty lines removed):
<div class="buying"><h1 class="parseasinTitle"><span id="btAsinTitle" style="">Avenger</span></h1>
<span >
<a href="/Amon-Amarth/e/B000APIBHO/ref=ntt_mus_dp_pel">Amon Amarth</a>
</span>
</div>

		details (empty lines removed):
<a name="productDetails" id="productDetails"></a>
<hr noshade="noshade" size="1" class="bucketDivider" />
<table cellpadding="0" cellspacing="0" border="0">     
  <tr>                                                 
    <td class="bucket">                                
<h2>Product Details</h2>
  <div class="content"> 
<ul>
<li><b>Audio CD</b>  (November 2, 1999)</li>
<li><b>Original Release Date:</b> November 2, 1999</li>
<li><b>Number of Discs:</b> 1</li>
<li><b>Label:</b> Metal Blade</li>

		tracks:
<tr class='rowEven'><td class="playCol"><a href="/gp/dmusic/media/sample.m3u/ref=dm_mu_dp_trk1_smpl/175-1810673-7649752?ie=UTF8&catalogItemType=track&ASIN=B0016OAHCK&DownloadLocation=CD" onclick='return cd_trackPreviewPressed("B0016OAHCK");'><img src="http://g-ecx.images-amazon.com/images/G/01/digital/music/dp/play-control-2._V223646478_.gif" width="19" alt="listen" id="cd_trackPreviewB0016OAHCK" title="listen" height="19" border="0" /></a></td><td class="titleCol">&nbsp; 1. <a href="http://www.amazon.com/gp/product/B0016OAHCK/ref=dm_mu_dp_trk1/175-1810673-7649752">Bleed For Ancient Gods</a></td><td class="runtimeCol"> 4:31</td><td class="priceCol">$0.99</td><td class="buyCol">

    alternatively (empty lines removed):
<tr class="listRowEven">
<td>
1. Before the Devil Knows You're Dead
</td>                                
	 */
	QString str = QString::fromLatin1(albumStr);
	FrameCollection framesHdr;
	// search for 'id="btAsinTitle"', text after '>' until ' [' or '<' => album
	int end = 0;
	int start = str.QCM_indexOf("id=\"btAsinTitle\"");
	if (start >= 0) {
		start = str.QCM_indexOf(">", start);
		if (start >= 0) {
			end = str.QCM_indexOf("<", start);
			if (end > start) {
				int bracketPos = str.QCM_indexOf(" [", start);
				if (bracketPos >= 0 && bracketPos < end) {
					end = bracketPos;
				}
				framesHdr.setAlbum(
					replaceHtmlEntities(str.mid(start + 1, end - start - 1)));

				// next '<a href=', text after '>' until '<' => artist
				start = str.QCM_indexOf("<a href=", end);
				if (start >= 0) {
					start = str.QCM_indexOf(">", start);
					if (start >= 0) {
						end = str.QCM_indexOf("<", start);
						if (end > start) {
							framesHdr.setArtist(
								replaceHtmlEntities(str.mid(start + 1, end - start - 1)));
						}
					}
				}
			}
		}
	}
	
	// search for >Product Details<, >Original Release Date:<, >Label:<
	const bool additionalTags = getAdditionalTags();
	QString albumArtist;
	start = str.QCM_indexOf(">Product Details<");
	if (start >= 0) {
		int detailStart = str.QCM_indexOf(">Original Release Date:<", start);
		if (detailStart < 0) {
			detailStart  = str.QCM_indexOf(">Audio CD<", start);
		}
		if (detailStart >= 0) {
			int detailEnd = str.QCM_indexOf("\n", detailStart + 10);
			if (detailEnd > detailStart + 10) {
				QRegExp yearRe("(\\d{4})");
				if (yearRe.QCM_indexIn(
							str.mid(detailStart + 10, detailEnd - detailStart - 11)) >= 0) {
					framesHdr.setYear(yearRe.cap(1).toInt());
				}
			}
		}
		if (additionalTags) {
			detailStart = str.QCM_indexOf(">Label:<", start);
			if (detailStart > 0) {
				int detailEnd = str.QCM_indexOf("\n", detailStart + 8);
				if (detailEnd > detailStart + 8) {
					QRegExp labelRe(">\\s*([^<]+)<");
					if (labelRe.QCM_indexIn(
								str.mid(detailStart + 8, detailEnd - detailStart - 9)) >= 0) {
						framesHdr.setValue(Frame::FT_Publisher, removeHtml(labelRe.cap(1)));
					}
				}
			}
			detailStart = str.QCM_indexOf(">Performer:<", start);
			if (detailStart > 0) {
				int detailEnd = str.QCM_indexOf("</li>", detailStart + 12);
				if (detailEnd > detailStart + 12) {
					framesHdr.setValue(
						Frame::FT_Performer,
						removeHtml(str.mid(detailStart + 11, detailEnd - detailStart - 11)));
				}
			}
			detailStart = str.QCM_indexOf(">Orchestra:<", start);
			if (detailStart > 0) {
				int detailEnd = str.QCM_indexOf("</li>", detailStart + 12);
				if (detailEnd > detailStart + 12) {
					albumArtist =
						removeHtml(str.mid(detailStart + 11, detailEnd - detailStart - 11));
				}
			}
			detailStart = str.QCM_indexOf(">Conductor:<", start);
			if (detailStart > 0) {
				int detailEnd = str.QCM_indexOf("</li>", detailStart + 12);
				if (detailEnd > detailStart + 12) {
					framesHdr.setValue(
						Frame::FT_Conductor,
						removeHtml(str.mid(detailStart + 11, detailEnd - detailStart - 11)));
				}
			}
			detailStart = str.QCM_indexOf(">Composer:<", start);
			if (detailStart > 0) {
				int detailEnd = str.QCM_indexOf("</li>", detailStart + 11);
				if (detailEnd > detailStart + 11) {
					framesHdr.setValue(
						Frame::FT_Composer,
						removeHtml(str.mid(detailStart + 10, detailEnd - detailStart - 10)));
				}
			}
		}
	}

	if (getCoverArt()) {
		// <input type="hidden" id="ASIN" name="ASIN" value="B0025AY48W" />
		start = str.QCM_indexOf("id=\"ASIN\"");
		if (start > 0) {
			start = str.QCM_indexOf("value=\"", start);
			if (start > 0) {
				end = str.QCM_indexOf("\"", start + 7);
				if (end > start) {
					m_trackDataVector.setCoverArtUrl(
						QString("http://www.amazon.com/dp/") +
						str.mid(start + 7, end - start - 7));
				}
			}
		}
	}

	bool hasTitleCol = false;
	bool hasArtist = str.QCM_indexOf("<td>Song Title</td><td>Artist</td>") != -1;
	// search 'class="titleCol"', next '<a href=', text after '>' until '<'
	// => title
	// if not found: alternatively look for 'class="listRow'
	start = str.QCM_indexOf("class=\"titleCol\"");
	if (start >= 0) {
		hasTitleCol = true;
	} else {
		start = str.QCM_indexOf("class=\"listRow");
	}
	if (start >= 0) {
		QRegExp durationRe("(\\d+):(\\d+)");
		QRegExp nrTitleRe("\\s*\\d+\\.\\s+(.*\\S)");
		FrameCollection frames(framesHdr);
		ImportTrackDataVector::iterator it = m_trackDataVector.begin();
		bool atTrackDataListEnd = (it == m_trackDataVector.end());
		int trackNr = 1;
		while (start >= 0) {
			QString title;
			QString artist;
			int duration = 0;
			if (hasTitleCol) {
				end = str.QCM_indexOf("\n", start);
				if (end > start) {
					QString line = str.mid(start, end - start);
					int titleStart = line.QCM_indexOf("<a href=");
					if (titleStart >= 0) {
						titleStart = line.QCM_indexOf(">", titleStart);
						if (titleStart >= 0) {
							int titleEnd = line.QCM_indexOf("<", titleStart);
							if (titleEnd > titleStart) {
								title = line.mid(titleStart + 1, titleEnd - titleStart - 1);
								// if there was an Artist title,
								// search for artist in a second titleCol
								if (hasArtist) {
									int artistStart =
										line.QCM_indexOf("class=\"titleCol\"", titleEnd);
									if (artistStart >= 0) {
										artistStart = line.QCM_indexOf("<a href=", artistStart);
										if (artistStart >= 0) {
											artistStart = line.QCM_indexOf(">", artistStart);
											if (artistStart >= 0) {
												int artistEnd = line.QCM_indexOf("<", artistStart);
												if (artistEnd > artistStart) {
													artist = line.mid(
														artistStart + 1, artistEnd - artistStart - 1);
													if (albumArtist.isEmpty()) {
														albumArtist = frames.getArtist();
													}
												}
											}
										}
									}
								}
								// search for next 'class="', if it is 'class="runtimeCol"',
								// text after '>' until '<' => duration
								int runtimeStart =
									line.QCM_indexOf("class=\"runtimeCol\"", titleEnd);
								if (runtimeStart >= 0) {
									runtimeStart = line.QCM_indexOf(">", runtimeStart + 18);
									if (runtimeStart >= 0) {
										int runtimeEnd = line.QCM_indexOf("<", runtimeStart);
										if (runtimeEnd > runtimeStart) {
											if (durationRe.QCM_indexIn(
														line.mid(runtimeStart + 1,
																		 runtimeEnd - runtimeStart - 1)) >= 0) {
												duration = durationRe.cap(1).toInt() * 60 +
													durationRe.cap(2).toInt();
											}
										}
									}
								}
								start = str.QCM_indexOf("class=\"titleCol\"", end);
							} else {
								start = -1;
							}
						}
					}
				}
			} else {
				// 'class="listRow' found
				start = str.QCM_indexOf("<td>", start);
				if (start >= 0) {
					end = str.QCM_indexOf("</td>", start);
					if (end > start &&
							nrTitleRe.QCM_indexIn(str.mid(start + 4, end - start - 4)) >= 0) {
						title = nrTitleRe.cap(1);
						start = str.QCM_indexOf("class=\"listRow", end);
					} else {
						start = -1; 
					}
				}
			}
			if (!title.isEmpty()) {
				frames.setTitle(replaceHtmlEntities(title));
				if (!artist.isEmpty()) {
					frames.setArtist(replaceHtmlEntities(artist));
				}
				if (!albumArtist.isEmpty() && additionalTags) {
					frames.setValue(Frame::FT_AlbumArtist, albumArtist);
				}
				frames.setTrack(trackNr);
				if (atTrackDataListEnd) {
					ImportTrackData trackData;
					trackData.setFrameCollection(frames);
					trackData.setImportDuration(duration);
					m_trackDataVector.push_back(trackData);
				} else {
					(*it).setFrameCollection(frames);
					(*it).setImportDuration(duration);
					++it;
					atTrackDataListEnd = (it == m_trackDataVector.end());
				}
				++trackNr;
				frames = framesHdr;
			}
		}

		// handle redundant tracks
		frames.clear();
		while (!atTrackDataListEnd) {
			if ((*it).getFileDuration() == 0) {
				it = m_trackDataVector.erase(it);
			} else {
				(*it).setFrameCollection(frames);
				(*it).setImportDuration(0);
				++it;
			}
			atTrackDataListEnd = (it == m_trackDataVector.end());
		}
	} else if (!framesHdr.empty()) {
		// if there are no track data, fill frame header data
		for (ImportTrackDataVector::iterator it = m_trackDataVector.begin();
				 it != m_trackDataVector.end();
				 ++it) {
			(*it).setFrameCollection(framesHdr);
		}
	}
}
