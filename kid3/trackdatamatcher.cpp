/**
 * \file trackdatamatcher.cpp
 * Shuffle imported tracks to optimize match with length, track or title.
 *
 * \b Project: Kid3
 * \author Urs Fleisch
 * \date 19 Jun 2011
 *
 * Copyright (C) 2011  Urs Fleisch
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

#include "trackdatamatcher.h"
#include <QStringList>
#include <QRegExp>
#include <QDir>
#include "trackdatamodel.h"

/**
 * Match import data with length.
 *
 * @param trackDataModel tracks to match
 * @param diffCheckEnable true if time difference check is enabled
 * @param maxDiff maximum allowed time difference
 */
bool TrackDataMatcher::matchWithLength(TrackDataModel* trackDataModel,
																			 bool diffCheckEnable, int maxDiff)
{
	struct MatchData {
		int fileLen;      // length of file
		int importLen;    // length of import
		int assignedTo;   // number of file import is assigned to, -1 if not assigned
		int assignedFrom; // number of import assigned to file, -1 if not assigned
	};

	bool failed = false;
	ImportTrackDataVector trackDataVector(trackDataModel->getTrackData());
	unsigned numTracks = trackDataVector.size();
	if (numTracks > 0) {
		MatchData* md = new MatchData[numTracks];
		unsigned numFiles = 0, numImports = 0;
		unsigned i = 0;
		for (ImportTrackDataVector::const_iterator it = trackDataVector.begin();
				 it != trackDataVector.end();
				 ++it) {
			if (i >= numTracks) {
				break;
			}
			md[i].fileLen = (*it).getFileDuration();
			if (md[i].fileLen > 0) {
				++numFiles;
			}
			md[i].importLen = (*it).getImportDuration();
			if (md[i].importLen > 0) {
				++numImports;
			}
			md[i].assignedTo = -1;
			md[i].assignedFrom = -1;
			// If time difference checking is enabled and the time difference
			// is not larger then the allowed limit, do not reassign the track.
			if (diffCheckEnable) {
				if (md[i].fileLen != 0 && md[i].importLen != 0) {
					int diff = md[i].fileLen > md[i].importLen ?
						md[i].fileLen - md[i].importLen : md[i].importLen - md[i].fileLen;
					if (diff <= maxDiff) {
						md[i].assignedTo = i;
						md[i].assignedFrom = i;
					}
				}
			}
			++i;
		}

		if (numFiles <= numImports) {
			// more imports than files => first look through all imports
			for (i = 0; i < numTracks; ++i) {
				if (md[i].assignedFrom == -1) {
					int bestTrack = -1;
					int bestDiff = INT_MAX;
					// Find the unassigned import with the best difference
					for (unsigned comparedTrack = 0; comparedTrack < numTracks; ++comparedTrack) {
						if (md[comparedTrack].assignedTo == -1) {
							int comparedDiff = md[i].fileLen > md[comparedTrack].importLen ?
								md[i].fileLen - md[comparedTrack].importLen :
								md[comparedTrack].importLen - md[i].fileLen;
							if (comparedDiff < bestDiff) {
								bestDiff = comparedDiff;
								bestTrack = comparedTrack;
							}
						}
					}
					if (bestTrack >= 0 && bestTrack < static_cast<int>(numTracks)) {
						md[i].assignedFrom = bestTrack;
						md[bestTrack].assignedTo = i;
					} else {
						qDebug("No match for track %d", i);
						failed = true;
						break;
					}
				}
			}
		} else {
			// more files than imports => first look through all files
			for (i = 0; i < numTracks; ++i) {
				if (md[i].assignedTo == -1) {
					int bestTrack = -1;
					int bestDiff = INT_MAX;
					// Find the unassigned file with the best difference
					for (unsigned comparedTrack = 0; comparedTrack < numTracks; ++comparedTrack) {
						if (md[comparedTrack].assignedFrom == -1) {
							int comparedDiff = md[comparedTrack].fileLen > md[i].importLen ?
								md[comparedTrack].fileLen - md[i].importLen :
								md[i].importLen - md[comparedTrack].fileLen;
							if (comparedDiff < bestDiff) {
								bestDiff = comparedDiff;
								bestTrack = comparedTrack;
							}
						}
					}
					if (bestTrack >= 0 && bestTrack < static_cast<int>(numTracks)) {
						md[i].assignedTo = bestTrack;
						md[bestTrack].assignedFrom = i;
					} else {
						qDebug("No match for track %d", i);
						failed = true;
						break;
					}
				}
			}
		}

		if (!failed) {
			ImportTrackDataVector oldTrackDataVector(trackDataVector);
			for (i = 0; i < numTracks; ++i) {
				trackDataVector[i].setFrameCollection(
					oldTrackDataVector[md[i].assignedFrom].getFrameCollection());
				trackDataVector[i].setImportDuration(
					oldTrackDataVector[md[i].assignedFrom].getImportDuration());
			}
			trackDataModel->setTrackData(trackDataVector);
		}

		delete [] md;
	}
	return !failed;
}

/**
 * Match import data with track number.
 *
 * @param trackDataModel tracks to match
 */
bool TrackDataMatcher::matchWithTrack(TrackDataModel* trackDataModel)
{
	struct MatchData {
		int track;        // track number starting with 0
		int assignedTo;   // number of file import is assigned to, -1 if not assigned
		int assignedFrom; // number of import assigned to file, -1 if not assigned
	};

	bool failed = false;
	ImportTrackDataVector trackDataVector(trackDataModel->getTrackData());
	unsigned numTracks = trackDataVector.size();
	if (numTracks > 0) {
		MatchData* md = new MatchData[numTracks];

		// 1st pass: Get track data and keep correct assignments.
		unsigned i = 0;
		for (ImportTrackDataVector::const_iterator it = trackDataVector.begin();
				 it != trackDataVector.end();
				 ++it) {
			if (i >= numTracks) {
				break;
			}
			if ((*it).getTrack() > 0 && (*it).getTrack() <= static_cast<int>(numTracks)) {
				md[i].track = (*it).getTrack() - 1;
			} else {
				md[i].track = -1;
			}
			md[i].assignedTo = -1;
			md[i].assignedFrom = -1;
			if (md[i].track == static_cast<int>(i)) {
				md[i].assignedTo = i;
				md[i].assignedFrom = i;
			}
			++i;
		}

		// 2nd pass: Assign imported track numbers to unassigned tracks.
		for (i = 0; i < numTracks; ++i) {
			if (md[i].assignedTo == -1 &&
					md[i].track >= 0 && md[i].track < static_cast<int>(numTracks)) {
				if (md[md[i].track].assignedFrom == -1) {
					md[md[i].track].assignedFrom = i;
					md[i].assignedTo = md[i].track;
				}
			}
		}

		// 3rd pass: Assign remaining tracks.
		unsigned unassignedTrack = 0;
		for (i = 0; i < numTracks; ++i) {
			if (md[i].assignedFrom == -1) {
				while (unassignedTrack < numTracks) {
					if (md[unassignedTrack].assignedTo == -1) {
						md[i].assignedFrom = unassignedTrack;
						md[unassignedTrack++].assignedTo = i;
						break;
					}
					++unassignedTrack;
				}
				if (md[i].assignedFrom == -1) {
					qDebug("No track assigned to %d", i);
					failed = true;
				}
			}
		}

		if (!failed) {
			ImportTrackDataVector oldTrackDataVector(trackDataVector);
			for (i = 0; i < numTracks; ++i) {
				trackDataVector[i].setFrameCollection(
					oldTrackDataVector[md[i].assignedFrom].getFrameCollection());
				trackDataVector[i].setImportDuration(
					oldTrackDataVector[md[i].assignedFrom].getImportDuration());
			}
			trackDataModel->setTrackData(trackDataVector);
		}

		delete [] md;
	}
	return !failed;
}

/**
 * Match import data with title.
 *
 * @param trackDataModel tracks to match
 */
bool TrackDataMatcher::matchWithTitle(TrackDataModel* trackDataModel)
{
	struct MatchData {
		QStringList fileWords;  // words in file name
		QStringList titleWords; // words in title
		int assignedTo;   // number of file import is assigned to, -1 if not assigned
		int assignedFrom; // number of import assigned to file, -1 if not assigned
	};

	bool failed = false;
	ImportTrackDataVector trackDataVector(trackDataModel->getTrackData());
	unsigned numTracks = trackDataVector.size();
	if (numTracks > 0) {
		MatchData* md = new MatchData[numTracks];
		unsigned numFiles = 0, numImports = 0;
		QRegExp nonWordCharRegExp("\\W");
		QRegExp nonLetterSpaceRegExp("[^a-z ]");
		unsigned i = 0;
		for (ImportTrackDataVector::const_iterator it = trackDataVector.begin();
				 it != trackDataVector.end();
				 ++it) {
			if (i >= numTracks) {
				break;
			}
			QString fileName = (*it).getAbsFilename();
			if (!fileName.isEmpty()) {
				++numFiles;
				int startIndex = fileName.lastIndexOf(QDir::separator()) + 1;
				int endIndex = fileName.lastIndexOf('.');
				if (endIndex > startIndex) {
					fileName = fileName.mid(startIndex, endIndex - startIndex);
				} else {
					fileName = fileName.mid(startIndex);
				}
				md[i].fileWords = fileName.toLower().
					replace(nonLetterSpaceRegExp, " ").split(nonWordCharRegExp);
			}
			if (!(*it).getTitle().isEmpty()) {
				++numImports;
				md[i].titleWords = (*it).getTitle().toLower().
					replace(nonLetterSpaceRegExp, " ").split(nonWordCharRegExp);
			}
			md[i].assignedTo = -1;
			md[i].assignedFrom = -1;
			++i;
		}

		if (numFiles <= numImports) {
			// more imports than files => first look through all imports
			for (i = 0; i < numTracks; ++i) {
				if (md[i].assignedFrom == -1) {
					int bestTrack = -1;
					int bestMatch = -1;
					// Find the unassigned import with the best match
					for (unsigned comparedTrack = 0; comparedTrack < numTracks; ++comparedTrack) {
						if (md[comparedTrack].assignedTo == -1) {
							int comparedMatch = 0;
							for (QStringList::const_iterator fwit = md[i].fileWords.begin();
									 fwit != md[i].fileWords.end();
									 ++fwit) {
								if (md[comparedTrack].titleWords.contains(*fwit)) {
									++comparedMatch;
								}
							}
							if (comparedMatch > bestMatch) {
								bestMatch = comparedMatch;
								bestTrack = comparedTrack;
							}
						}
					}
					if (bestTrack >= 0 && bestTrack < static_cast<int>(numTracks)) {
						md[i].assignedFrom = bestTrack;
						md[bestTrack].assignedTo = i;
					} else {
						qDebug("No match for track %d", i);
						failed = true;
						break;
					}
				}
			}
		} else {
			// more files than imports => first look through all files
			for (i = 0; i < numTracks; ++i) {
				if (md[i].assignedTo == -1) {
					int bestTrack = -1;
					int bestMatch = -1;
					// Find the unassigned file with the best match
					for (unsigned comparedTrack = 0; comparedTrack < numTracks; ++comparedTrack) {
						if (md[comparedTrack].assignedFrom == -1) {
							int comparedMatch = 0;
							for (QStringList::const_iterator fwit = md[comparedTrack].fileWords.begin();
									 fwit != md[comparedTrack].fileWords.end();
									 ++fwit) {
								if (md[i].titleWords.contains(*fwit)) {
									++comparedMatch;
								}
							}
							if (comparedMatch > bestMatch) {
								bestMatch = comparedMatch;
								bestTrack = comparedTrack;
							}
						}
					}
					if (bestTrack >= 0 && bestTrack < static_cast<int>(numTracks)) {
						md[i].assignedTo = bestTrack;
						md[bestTrack].assignedFrom = i;
					} else {
						qDebug("No match for track %d", i);
						failed = true;
						break;
					}
				}
			}
		}
		if (!failed) {
			ImportTrackDataVector oldTrackDataVector(trackDataVector);
			for (i = 0; i < numTracks; ++i) {
				trackDataVector[i].setFrameCollection(
					oldTrackDataVector[md[i].assignedFrom].getFrameCollection());
				trackDataVector[i].setImportDuration(
					oldTrackDataVector[md[i].assignedFrom].getImportDuration());
			}
			trackDataModel->setTrackData(trackDataVector);
		}

		delete [] md;
	}
	return !failed;
}
