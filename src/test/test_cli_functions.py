#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import unittest
import sys
import os
import subprocess
import tempfile
import platform
import json
from kid3testsupport import kid3_cli_path, call_kid3_cli, create_test_file, ignore_audio_properties, \
    Kid3ConfigFileUsingOnlyTagLib, Kid3ConfigFileUsingOnlyId3lib, Kid3ConfigFileUsingOnlyOggFlac, \
    Kid3ConfigFileUsingOnlyMp4v2


def setUpModule():
    # Use an invalid config file to use a default configuration.
    os.environ['KID3_CONFIG_FILE'] = ''
    # Use English locale.
    os.environ['LANGUAGE'] = 'en_US.UTF-8'
    os.environ['LANG'] = 'en_US.UTF-8'


class CliFunctionsTestCase(unittest.TestCase):
    def test_help(self):
        full_help = call_kid3_cli('-h')
        self.assertIn('Available Commands', full_help)
        self.assertRegex(full_help, r'help \[S\]\s+Help')
        self.assertRegex(full_help, r'^kid3-cli (\d+\.\d+\.\d+|git\d{8}) \(c\) \d{4} Urs Fleisch')
        full_help = call_kid3_cli(['-c', 'help'])
        self.assertIn('Available Commands', full_help)
        self.assertRegex(full_help, r'help \[S\]\s+Help')
        self.assertTrue(full_help.startswith('Parameter\n  P = File path'))
        self.assertRegex(full_help, r'playlist\s+Create playlist')
        self.assertRegex(call_kid3_cli(['-c', 'help paste']), r'paste \[T\]\s+Paste')

    def test_invalid(self):
        p = subprocess.Popen([kid3_cli_path(), '-c', 'invalid'],
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = p.communicate()
        self.assertEqual(stdout, b'')
        self.assertIn(b"Unknown command 'invalid', -h for help.", stderr)
        self.assertEqual(p.returncode, 1)

    def test_invalid_interactive(self):
        p = subprocess.Popen([kid3_cli_path()],
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True)
        stdout, stderr = p.communicate('invalid\n')
        self.assertEqual(stdout, '')
        self.assertIn("Unknown command 'invalid'. Type 'help' for help.", stderr)
        self.assertEqual(p.returncode, 0)

    def test_timeout(self):
        self.assertEqual(call_kid3_cli(
            ['-c', 'timeout', '-c', 'timeout 5000', '-c', 'timeout',
             '-c', 'timeout off', '-c', 'timeout',
             '-c', 'timeout default', '-c', 'timeout']),
            'Timeout: default\n'
            'Timeout: 5000 ms\n'
            'Timeout: 5000 ms\n'
            'Timeout: off\n'
            'Timeout: off\n'
            'Timeout: default\n'
            'Timeout: default\n')

    def test_exit(self):
        self.assertEqual(call_kid3_cli(['-c', 'exit']), '')

    def test_config(self):
        self.assertEqual(call_kid3_cli(['-c', 'config']),
            'BatchImport\n'
            'Export\n'
            'File\n'
            'FilenameFormat\n'
            'Filter\n'
            'Import\n'
            'Network\n'
            'NumberTracks\n'
            'Playlist\n'
            'RenameFolder\n'
            'Tag\n'
            'TagFormat\n')
        self.assertEqual(call_kid3_cli(
            ['-c', 'config Tag.id3v2Version',
             '-c', 'config Tag.id3v2Version ID3v2_4_0',
             '-c', 'config Tag.id3v2Version',
             '-c', 'config Tag.id3v2Version ID3v2_3_0']),
            'ID3v2_3_0\n'
            'ID3v2_4_0\n'
            'ID3v2_4_0\n'
            'ID3v2_3_0\n')
        self.assertEqual(call_kid3_cli(
            ['-c', 'config TagFormat.caseConversion',
             '-c', 'config TagFormat.caseConversion NoChanges',
             '-c', 'config TagFormat.caseConversion',
             '-c', 'config TagFormat.caseConversion AllFirstLettersUppercase']),
            'AllFirstLettersUppercase\n'
            'NoChanges\n'
            'NoChanges\n'
            'AllFirstLettersUppercase\n')
        self.assertEqual(call_kid3_cli(
            ['-c', 'config NumberTracks.numberTracksDestination',
             '-c', 'config NumberTracks.numberTracksDestination 21',
             '-c', 'config NumberTracks.numberTracksDestination',
             '-c', 'config NumberTracks.numberTracksDestination 1']),
            '1\n'
            '21\n'
            '21\n'
            '1\n')

    def test_config_invalid(self):
        p = subprocess.Popen([kid3_cli_path(), '-c', 'config NoSuchGroup.foo'],
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = p.communicate()
        self.assertEqual(stdout, b'')
        self.assertIn(b'NoSuchGroup does not exist', stderr)
        self.assertEqual(p.returncode, 1)

        p = subprocess.Popen([kid3_cli_path(), '-c', 'config Tag.textEncoding UTF8'],
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = p.communicate()
        self.assertEqual(stdout, b'')
        self.assertIn(b'Invalid value', stderr)
        self.assertEqual(p.returncode, 1)

    def test_config_playlist(self):
        lines = call_kid3_cli(
            ['-c', 'config Playlist.format PF_XSPF',
             '-c', 'config Playlist.format',
             '-c', 'config Playlist.location PL_EveryDirectory',
             '-c', 'config Playlist.location']).splitlines()
        self.assertEqual(lines[0], 'PF_XSPF')
        self.assertEqual(lines[1], 'PF_XSPF')
        self.assertEqual(lines[2], 'PL_EveryDirectory')
        self.assertEqual(lines[3], 'PL_EveryDirectory')

    def test_cd_pwd_and_invalid_cd(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            lines = call_kid3_cli(['-c', 'pwd', '-c', 'cd "%s"' % tmpdir, '-c', 'pwd']).splitlines()
            self.assertTrue(lines)
            self.assertIn(lines[-1], (tmpdir, tmpdir.replace('\\', '/')))

            p = subprocess.Popen([kid3_cli_path(), '-c', 'cd %s/no_such_dir' % tmpdir],
                                 stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            stdout, stderr = p.communicate()
            self.assertEqual(stdout, b'')
            self.assertIn(b'does not exist', stderr)
            self.assertEqual(p.returncode, 1)

    def test_filter_invalid_name(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            p = subprocess.Popen([kid3_cli_path(), '-c', 'filter NoSuchFilter', tmpdir],
                                 stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            stdout, stderr = p.communicate()
            self.assertEqual(stdout, b'')
            self.assertIn(b'NoSuchFilter not found.', stderr)
            self.assertIn(b'Available:', stderr)
            self.assertEqual(p.returncode, 1)

    def test_select_missing_file(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            p = subprocess.Popen([kid3_cli_path(), '-c', 'select no_such_file.mp3', tmpdir],
                                 stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            stdout, stderr = p.communicate()
            self.assertEqual(stdout, b'')
            self.assertIn(b'no_such_file.mp3 not found', stderr)
            self.assertEqual(p.returncode, 1)

    def test_usage_error(self):
        p = subprocess.Popen([kid3_cli_path(), '-c', 'set'],
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             universal_newlines=True)
        stdout, stderr = p.communicate()
        self.assertEqual(stdout, 'Usage:\n')
        self.assertIn('set N V [T]  Set tag frame', stderr)
        self.assertEqual(p.returncode, 1)

    def test_ls(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            self.assertEqual(call_kid3_cli(['-c', 'ls', tmpdir]), '')
            create_test_file(os.path.join(tmpdir, 'test.mp3'))
            self.assertEqual(call_kid3_cli(['-c', 'ls', tmpdir]), '  --- test.mp3\n')

    def test_id3v1_taglib(self):
        with Kid3ConfigFileUsingOnlyTagLib():
            self._run_id3v1_tests()

    def test_id3v1_id3lib(self):
        with Kid3ConfigFileUsingOnlyId3lib():
            self._run_id3v1_tests()

    def _run_id3v1_tests(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            mp3path = os.path.join(tmpdir, 'test.mp3')
            create_test_file(mp3path)
            actual = call_kid3_cli(
                ['-c', 'get all 1', mp3path])
            if not actual:
                # Do not fail if Id3libMetadata plugin is not present
                return
            self.assertEqual(actual,
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test.mp3\n')
            with open(mp3path, 'rb') as mp3fh:
                empty_mp3_bytes = mp3fh.read()
            self.assertEqual(call_kid3_cli(
                ['-c', 'get title 1',
                 '-c', 'set title "A Title" 1',
                 '-c', 'get title 1',
                 '-c', 'save',
                 '-c', 'get all 1', mp3path]),
                'A Title\n'
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test.mp3\n'
                'Tag 1: ID3v1.1\n'
                '  Title  A Title\n')
            self.assertEqual(call_kid3_cli(
                ['-c', 'set artist "An Artist" 1',
                 '-c', 'set album "An Album" 1',
                 '-c', 'set track 5 1',
                 '-c', 'set genre Metal 1',
                 '-c', 'set date 2016 1',
                 '-c', 'set comment "A Comment" 1',
                 '-c', 'get all 1', mp3path]),
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test.mp3\nTag 1: ID3v1.1\n'
                '  Title         A Title\n'
                '* Artist        An Artist\n'
                '* Album         An Album\n'
                '* Comment       A Comment\n'
                '* Date          2016\n'
                '* Track Number  5\n'
                '* Genre         Metal\n')
            with open(mp3path, 'rb') as mp3fh:
                ba = mp3fh.read()
                self.assertEqual(
                    ba[-128:],
                    b'TAGA '
                    b'Title\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
                    b'An Artist\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
                    b'An Album\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
                    b'2016'
                    b'A Comment\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x05\t')
            self.assertEqual(call_kid3_cli(
                ['-c', 'copy 1',
                 '-c', 'remove 1',
                 '-c', 'get all 1',
                 '-c', 'paste 1',
                 '-c', 'get all 1',
                 '-c', 'set track 6 1',
                 '-c', 'get all 1',
                 '-c', 'revert',
                 '-c', 'get all 1',
                 '-c', 'remove 1', mp3path]),
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test.mp3\n'
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test.mp3\n'
                'Tag 1: ID3v1.1\n'
                '* Title         A Title\n'
                '* Artist        An Artist\n'
                '* Album         An Album\n'
                '* Comment       A Comment\n'
                '* Date          2016\n'
                '* Track Number  5\n'
                '* Genre         Metal\n'
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test.mp3\nTag 1: ID3v1.1\n'
                '* Title         A Title\n'
                '* Artist        An Artist\n'
                '* Album         An Album\n'
                '* Comment       A Comment\n'
                '* Date          2016\n'
                '* Track Number  6\n'
                '* Genre         Metal\n'
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test.mp3\n'
                'Tag 1: ID3v1.1\n'
                '  Title         A Title\n'
                '  Artist        An Artist\n'
                '  Album         An Album\n'
                '  Comment       A Comment\n'
                '  Date          2016\n'
                '  Track Number  5\n'
                '  Genre         Metal\n')
            with open(mp3path, 'rb') as mp3fh:
                ba = mp3fh.read()
                self.assertEqual(ba, empty_mp3_bytes)

    def test_id3v2_taglib(self):
        with Kid3ConfigFileUsingOnlyTagLib():
            self._run_id3v2_tests()
            with tempfile.TemporaryDirectory() as tmpdir:
                mp3path = os.path.join(tmpdir, 'test.mp3')
                lrcpath = os.path.join(tmpdir, 'test.lrc')
                eventspath = os.path.join(tmpdir, 'events.lrc')
                syltpath = os.path.join(tmpdir, 'sylt.lrc')
                etcopath = os.path.join(tmpdir, 'etco.lrc')
                chappath = os.path.join(tmpdir, 'chap.lrc')
                create_test_file(mp3path)
                create_test_file(lrcpath)
                etco_bytes = (
                        b'[ti:Title]\r\n'
                        b'\r\n'
                        b'[00:00:00.000]intro start\r\n'
                        b'[00:01:02.003]refrain start\r\n'
                        b'[24:25:26.270]outro end\r\n')
                with open(eventspath, 'wb') as lrcfh:
                    lrcfh.write(etco_bytes)
                with open(lrcpath, 'rb') as lrcfh:
                    lrc_bytes = lrcfh.read()
                actual = call_kid3_cli(
                    ['-c', 'set artist "Artist" 2',
                     '-c', 'set album "Album" 2',
                     '-c', 'save',
                     '-c', 'to24',
                     '-c', 'get all 2',
                     '-c', 'to23',
                     '-c', 'get all 2',
                     '-c', 'remove 2', mp3path])
                expected = ('File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                            '  Name: test.mp3\n'
                            'Tag 2: ID3v2.4.0\n'
                            '  Artist  Artist\n'
                            '  Album   Album\n'
                            'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                            '  Name: test.mp3\n'
                            'Tag 2: ID3v2.3.0\n'
                            '  Artist  Artist\n'
                            '  Album   Album\n')
                self.assertEqual(actual, expected)
                self.assertEqual(call_kid3_cli(
                    ['-c', 'set POPM 5',
                     '-c', 'get all 2',
                     '-c', 'get POPM.Email',
                     '-c', 'get POPM.Rating',
                     '-c', 'get POPM.Counter',
                     '-c', 'set POPM.Email ufleisch@users.sourceforge.net',
                     '-c', 'set POPM.Rating 4',
                     '-c', 'set POPM.Counter 3',
                     '-c', 'get all 2',
                     '-c', 'get POPM.Email',
                     '-c', 'get POPM.Rating',
                     '-c', 'get POPM.Counter',
                     '-c', 'get POPM', mp3path]),
                     'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                     '  Name: test.mp3\n'
                     'Tag 2: ID3v2.3.0\n'
                     '* Rating  5\n'
                     '\n'
                     '5\n'
                     '0\n'
                     'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                     '  Name: test.mp3\n'
                     'Tag 2: ID3v2.3.0\n'
                     '* Rating  4\n'
                     'ufleisch@users.sourceforge.net\n'
                     '4\n'
                     '3\n'
                     '4\n')
                call_kid3_cli(
                    ['-c', 'remove',
                     '-c', 'set title Title',
                     '-c', 'set UFID.Owner http://www.id3.org/test/ufid.html',
                     '-c', 'set UFID.Identifier 54455354',
                     '-c', 'set "SYLT:%s" ""' % lrcpath,
                     '-c', 'set "ETCO:%s" ""' % eventspath,
                     '-c', 'set "Chapters:%s" ""' % lrcpath,
                     mp3path])
                actual = call_kid3_cli(
                    ['-c', 'get all',
                     '-c', 'get UFID.Owner',
                     '-c', 'get UFID.Identifier',
                     '-c', 'get "SYLT:%s"' % syltpath,
                     '-c', 'get "ETCO:%s"' % etcopath,
                     '-c', 'get "Chapters:%s"' % chappath,
                     mp3path])
                expected = ('File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                            '  Name: test.mp3\n'
                            'Tag 2: ID3v2.3.0\n'
                            '  Title                Title\n'
                            '  Chapter              chp01\n'
                            '  Chapter              chp02\n'
                            '  Chapter              chp03\n'
                            '  Table of Contents    toc01\n'
                            '  Chapters             \n'
                            '  Event Timing Codes   \n'
                            '  Synchronized Lyrics  \n'
                            '  File ID: ufid        54455354\n'
                            'http://www.id3.org/test/ufid.html\n'
                            '54455354\n'
                            '\n'
                            '\n'
                            '\n')
                self.assertEqual(actual, expected)
                with open(syltpath, 'rb') as lrcfh:
                    ba = lrcfh.read()
                    self.assertEqual(ba, lrc_bytes)
                with open(etcopath, 'rb') as lrcfh:
                    ba = lrcfh.read()
                    self.assertEqual(ba, etco_bytes)
                with open(chappath, 'rb') as lrcfh:
                    ba = lrcfh.read()
                    self.assertEqual(ba, lrc_bytes)

    def test_id3v2_id3lib(self):
        with Kid3ConfigFileUsingOnlyId3lib():
            self._run_id3v2_tests()

    def _run_id3v2_tests(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            mp3path = os.path.join(tmpdir, 'test.mp3')
            jpgpath = os.path.join(tmpdir, 'test.jpg')
            picpath = os.path.join(tmpdir, 'folder.jpg')
            lyricspath = os.path.join(tmpdir, 'lyrics.txt')
            usltpath = os.path.join(tmpdir, 'uslt.txt')
            create_test_file(mp3path)
            create_test_file(jpgpath)
            uslt_bytes = 'Schön\nsind die Lyrics.\n'.encode()
            with open(lyricspath, 'wb') as txtfh:
                txtfh.write(uslt_bytes)
            with open(mp3path, 'rb') as mp3fh:
                empty_mp3_bytes = mp3fh.read()
            with open(jpgpath, 'rb') as jpgfh:
                jpg_bytes = jpgfh.read()
            actual = call_kid3_cli(
                ['-c', 'get title 2',
                 '-c', 'get all 2', mp3path])
            if not actual:
                # Do not fail if Id3libMetadata plugin is not present
                return
            expected = ('File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                        '  Name: test.mp3\n')
            self.assertEqual(actual, expected)
            self.assertEqual(call_kid3_cli(
                ['-c', 'set artist "An Artist" 2',
                 '-c', 'set album "An Album" 2',
                 '-c', 'set track 3 2',
                 '-c', 'set genre "Power Metal" 2',
                 '-c', 'set date 2016 2',
                 '-c', 'set comment "A Comment" 2',
                 '-c', 'set Lyricist "A Lyricist" 2',
                 '-c', 'set TDLY 100 2',
                 '-c', 'set picture:"%s" "A Description" 2' % jpgpath,
                 '-c', 'set comment "A Comment" 2',
                 '-c', 'get all 2', mp3path]),
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test.mp3\n'
                'Tag 2: ID3v2.3.0\n'
                '* Artist                  An Artist\n'
                '* Album                   An Album\n'
                '* Comment                 A Comment\n'
                '* Date                    2016\n'
                '* Track Number            3\n'
                '* Genre                   Power Metal\n'
                '* Lyricist                A Lyricist\n'
                '* Picture: Cover (front)  A Description\n'
                '* Playlist Delay          100\n')
            self.assertEqual(call_kid3_cli(
                ['-c', 'copy 2',
                 '-c', 'remove 2',
                 '-c', 'get all 2',
                 '-c', 'paste 2',
                 '-c', 'get all 2',
                 '-c', 'set track 6 2',
                 '-c', 'syncto 1',
                 '-c', 'get all',
                 '-c', 'revert',
                 '-c', 'get all',
                 '-c', 'get picture:"%s" "A Description" 2' % picpath,
                 '-c', 'remove 2', mp3path]),
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test.mp3\n'
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test.mp3\n'
                'Tag 2: ID3v2.3.0\n'
                '* Artist                  An Artist\n'
                '* Album                   An Album\n'
                '* Comment                 A Comment\n'
                '* Date                    2016\n'
                '* Track Number            3\n'
                '* Genre                   Power Metal\n'
                '* Lyricist                A Lyricist\n'
                '* Picture: Cover (front)  A Description\n'
                '* Playlist Delay          100\n'
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test.mp3\n'
                'Tag 1: ID3v1.1\n'
                '* Artist        An Artist\n'
                '* Album         An Album\n'
                '* Comment       A Comment\n'
                '* Date          2016\n'
                '* Track Number  6\n'
                'Tag 2: ID3v2.3.0\n'
                '* Artist                  An Artist\n'
                '* Album                   An Album\n'
                '* Comment                 A Comment\n'
                '* Date                    2016\n'
                '* Track Number            6\n'
                '* Genre                   Power Metal\n'
                '* Lyricist                A Lyricist\n'
                '* Picture: Cover (front)  A Description\n'
                '* Playlist Delay          100\n'
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test.mp3\n'
                'Tag 2: ID3v2.3.0\n'
                '  Artist                  An Artist\n'
                '  Album                   An Album\n'
                '  Comment                 A Comment\n'
                '  Date                    2016\n'
                '  Track Number            3\n'
                '  Genre                   Power Metal\n'
                '  Lyricist                A Lyricist\n'
                '  Picture: Cover (front)  A Description\n'
                '  Playlist Delay          100\n'
                'A Description\n')
            with open(mp3path, 'rb') as mp3fh:
                ba = mp3fh.read()
                self.assertEqual(ba, empty_mp3_bytes)
            with open(picpath, 'rb') as jpgfh:
                ba = jpgfh.read()
                self.assertEqual(ba, jpg_bytes)
            actual = call_kid3_cli(
                ['-c', 'remove',
                 '-c', 'set Rating 196',
                 '-c', 'set Description Description',
                 '-c', 'set PRIV.Owner WM/WMContentID',
                 '-c', 'set PRIV.Data 177bf099-f5f8-4e93-b083-f12befddd4a4',
                 '-c', 'set Track 1/2',
                 '-c', 'set CATALOGNUMBER "Catalog Number"',
                 '-c', 'set RELEASECOUNTRY "Release Country"',
                 '-c', 'set GROUPING Grouping',
                 '-c', 'set TOPE "Schön 1|Schön 2"',
                 '-c', 'set "USLT:%s" ""' % lyricspath,
                 '-c', 'save',
                 '-c', 'get all',
                 '-c', 'get PRIV.Owner',
                 '-c', 'get PRIV.Data',
                 '-c', 'get "USLT:%s"' % usltpath,
                 mp3path])
            expected = ('File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                        '  Name: test.mp3\n'
                        'Tag 2: ID3v2.3.0\n'
                        '  Track Number     1/2\n'
                        '  Catalog Number   Catalog Number\n'
                        '  Grouping         Grouping\n'
                        '  Lyrics           Schön\n'
                        'sind die Lyrics.\n'
                        '\n'
                        '  Original Artist  Schön 1|Schön 2\n'
                        '  Description      Description\n'
                        '  Release Country  Release Country\n'
                        '  Rating           196\n'
                        'WM/WMContentID\n'
                        '177bf099-f5f8-4e93-b083-f12befddd4a4\n'
                        'Schön\n'
                        'sind die Lyrics.\n'
                        '\n')
            self.assertEqual(actual, expected)
            with open(usltpath, 'rb') as txtfh:
                ba = txtfh.read()
                self.assertEqual(ba, uslt_bytes)

    def test_flac_taglib(self):
        with Kid3ConfigFileUsingOnlyTagLib():
            self._run_flac_tests()
            with tempfile.TemporaryDirectory() as tmpdir:
                flacpath = os.path.join(tmpdir, 'test.flac')
                jpgpath = os.path.join(tmpdir, 'test.jpg')
                create_test_file(flacpath)
                create_test_file(jpgpath)
                actual = call_kid3_cli(
                    ['-c', 'remove 123',
                    '-c', 'set artist "Artist 1" 1',
                    '-c', 'set artist "Artist 2" 2',
                    '-c', 'set artist "Artist 3" 3',
                    '-c', 'set CUSTOM "Custom 2" 2',
                    '-c', 'set CUSTOM "Custom 3" 3',
                    '-c', 'set picture:"%s" "Picture 3 Description" 3' % jpgpath,
                    '-c', 'get all 123', flacpath])
                expected = ('File: FLAC 16 bit 4234 kbps 44100 Hz 1 Channels\n'
                            '  Name: test.flac\n'
                            'Tag 1: ID3v1.1\n'
                            '* Artist  Artist 1\n'
                            'Tag 2: Vorbis\n'
                            '* Artist  Artist 2\n'
                            '* CUSTOM  Custom 2\n'
                            'Tag 3: ID3v2.4.0\n'
                            '* Artist                  Artist 3\n'
                            '* Picture: Cover (front)  Picture 3 Description\n'
                            '  CUSTOM                  Custom 3\n')
                self.assertEqual(actual, expected)

    def test_flac_oggflac(self):
        with Kid3ConfigFileUsingOnlyOggFlac():
            self._run_flac_tests()

    def _run_flac_tests(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            flacpath = os.path.join(tmpdir, 'test.flac')
            jpgpath = os.path.join(tmpdir, 'test.jpg')
            picpath = os.path.join(tmpdir, 'folder.jpg')
            create_test_file(flacpath)
            create_test_file(jpgpath)
            with open(jpgpath, 'rb') as jpgfh:
                jpg_bytes = jpgfh.read()
            self.assertRegex(call_kid3_cli(
                ['-c', 'get title 2',
                 '-c', 'get all 2', flacpath]),
                'File: FLAC [^\\n]+ kbps 44100 Hz 1 Channels\n'
                '  Name: test\\.flac\n')
            self.assertRegex(call_kid3_cli(
                ['-c', 'set artist "A first artist"',
                 '-c', 'set artist[1] "A second artist"',
                 '-c', 'set album "Album Name"',
                 '-c', 'set track 4',
                 '-c', 'set genre "Heavy Metal"',
                 '-c', 'set date 2017',
                 '-c', 'set comment[0] "Comment 1"',
                 '-c', 'set comment[1] "Comment 2"',
                 '-c', 'set comment[2] "Comment 3"',
                 '-c', 'set Lyricist "A Lyricist" 2',
                 '-c', 'set tracktotal 12',
                 '-c', 'set "disc number" 1',
                 '-c', 'set DISCTOTAL 2',
                 '-c', 'set picture:"%s" "Picture Description"' % jpgpath,
                 '-c', 'get comment[1]',
                 '-c', 'get comment',
                 '-c', 'get', flacpath]),
                'Comment 2\n'
                'Comment 1\n'
                'File: FLAC [^\\n]+ kbps 44100 Hz 1 Channels\n'
                '  Name: test\\.flac\n'
                'Tag 2: Vorbis\n'
                '\\* Artist                  A first artist\n'
                '\\* Artist                  A second artist\n'
                '\\* Album                   Album Name\n'
                '\\* Comment                 Comment 1\n'
                '\\* Comment                 Comment 2\n'
                '\\* Comment                 Comment 3\n'
                '\\* Date                    2017\n'
                '\\* Track Number            4\n'
                '\\* Genre                   Heavy Metal\n'
                '\\* Disc Number             1\n'
                '\\* Lyricist                A Lyricist\n'
                '\\* Picture: Cover \\(front\\)  Picture Description\n'
                '\\* Total Discs             2\n'
                '\\* Total Tracks            12\n')
            self.assertRegex(call_kid3_cli(
                ['-c', 'set artist[1] ""',
                 '-c', 'set comment[1] "Comment B"',
                 '-c', 'set comment[0] "Comment A"',
                 '-c', 'set comment[2] "Comment C"',
                 '-c', 'set lyricist ""',
                 '-c', 'set picture[1]:"%s" "Back Cover"' % jpgpath,
                 '-c', 'set picture[1].picturetype 4',
                 '-c', 'set picture[0].description "Front Cover"',
                 '-c', 'get picture[1]:"%s"' % picpath,
                 '-c', 'get',
                 '-c', 'remove',
                 '-c', 'get', flacpath]),
                'Back Cover\n'
                'File: FLAC [^\\n]+ kbps 44100 Hz 1 Channels\n'
                '  Name: test\\.flac\n'
                'Tag 2: Vorbis\n'
                '\\* Artist                  A first artist\n'
                '  Album                   Album Name\n'
                '\\* Comment                 Comment A\n'
                '\\* Comment                 Comment B\n'
                '\\* Comment                 Comment C\n'
                '  Date                    2017\n'
                '  Track Number            4\n'
                '  Genre                   Heavy Metal\n'
                '  Disc Number             1\n'
                '\\* Picture: Cover \\(front\\)  Front Cover\n'
                '\\* Picture: Cover \\(back\\)   Back Cover\n'
                '  Total Discs             2\n'
                '  Total Tracks            12\n'
                'File: FLAC [^\\n]+ kbps 44100 Hz 1 Channels\n'
                '  Name: test\\.flac\n')
            with open(picpath, 'rb') as jpgfh:
                ba = jpgfh.read()
                self.assertEqual(ba, jpg_bytes)

    def test_ogg_taglib(self):
        with Kid3ConfigFileUsingOnlyTagLib():
            self._run_ogg_tests()

    def test_ogg_oggflac(self):
        with Kid3ConfigFileUsingOnlyOggFlac():
            self._run_ogg_tests()

    def _run_ogg_tests(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            path = os.path.join(tmpdir, 'test.ogg')
            jpgpath = os.path.join(tmpdir, 'test.jpg')
            picpath = os.path.join(tmpdir, 'folder.jpg')
            create_test_file(path)
            create_test_file(jpgpath)
            with open(jpgpath, 'rb') as jpgfh:
                jpg_bytes = jpgfh.read()
            actual = ignore_audio_properties(call_kid3_cli(
                ['-c', 'get all 2',
                 '-c', 'set artist "A first artist"',
                 '-c', 'set artist[1] "A second artist"',
                 '-c', 'set album "Album Name"',
                 '-c', 'set track 4',
                 '-c', 'set genre "Heavy Metal"',
                 '-c', 'set date 2017',
                 '-c', 'set comment[0] "Comment 1"',
                 '-c', 'set comment[1] "Comment 2"',
                 '-c', 'set comment[2] "Comment 3"',
                 '-c', 'set Lyricist "A Lyricist" 2',
                 '-c', 'set tracktotal 12',
                 '-c', 'set "disc number" 1',
                 '-c', 'set DISCTOTAL 2',
                 '-c', 'set picture:"%s" "Picture Description"' % jpgpath,
                 '-c', 'get comment[1]',
                 '-c', 'get comment',
                 '-c', 'get', path]))
            expected = ignore_audio_properties('File: Ogg Vorbis 112 kbps 44100 Hz 2 Channels 0:01\n'
                        '  Name: test.ogg\n'
                        'Tag 2: Vorbis\n'
                        '  Encoder  Lavc61.19.101 libvorbis\n'
                        'Comment 2\n'
                        'Comment 1\n'
                        'File: Ogg Vorbis 112 kbps 44100 Hz 2 Channels 0:01\n'
                        '  Name: test.ogg\n'
                        'Tag 2: Vorbis\n'
                        '* Artist                  A first artist\n'
                        '* Artist                  A second artist\n'
                        '* Album                   Album Name\n'
                        '* Comment                 Comment 1\n'
                        '* Comment                 Comment 2\n'
                        '* Comment                 Comment 3\n'
                        '* Date                    2017\n'
                        '* Track Number            4\n'
                        '* Genre                   Heavy Metal\n'
                        '* Disc Number             1\n'
                        '* Lyricist                A Lyricist\n'
                        '* Picture: Cover (front)  Picture Description\n'
                        '* Total Discs             2\n'
                        '  Encoder                 Lavc61.19.101 libvorbis\n'
                        '* Total Tracks            12\n')
            self.assertEqual(actual, expected)
            actual = ignore_audio_properties(call_kid3_cli(
                ['-c', 'set artist[1] ""',
                 '-c', 'set comment[1] "Comment B"',
                 '-c', 'set comment[0] "Comment A"',
                 '-c', 'set comment[2] "Comment C"',
                 '-c', 'set lyricist ""',
                 '-c', 'set picture[1]:"%s" "Back Cover"' % jpgpath,
                 '-c', 'set picture[1].picturetype 4',
                 '-c', 'set picture[0].description "Front Cover"',
                 '-c', 'get picture[1]:"%s"' % picpath,
                 '-c', 'get',
                 '-c', 'remove',
                 '-c', 'get', path]))
            expected = ignore_audio_properties('Back Cover\n'
                        'File: Ogg Vorbis 112 kbps 44100 Hz 2 Channels 0:01\n'
                        '  Name: test.ogg\n'
                        'Tag 2: Vorbis\n'
                        '* Artist                  A first artist\n'
                        '  Album                   Album Name\n'
                        '* Comment                 Comment A\n'
                        '* Comment                 Comment B\n'
                        '* Comment                 Comment C\n'
                        '  Date                    2017\n'
                        '  Track Number            4\n'
                        '  Genre                   Heavy Metal\n'
                        '  Disc Number             1\n'
                        '* Picture: Cover (front)  Front Cover\n'
                        '* Picture: Cover (back)   Back Cover\n'
                        '  Total Discs             2\n'
                        '  Encoder                 Lavc61.19.101 libvorbis\n'
                        '  Total Tracks            12\n'
                        'File: Ogg Vorbis 112 kbps 44100 Hz 2 Channels 0:01\n'
                        '  Name: test.ogg\n')
            self.assertEqual(actual, expected)
            with open(picpath, 'rb') as jpgfh:
                ba = jpgfh.read()
                self.assertEqual(ba, jpg_bytes)

    def test_mp4_taglib(self):
        with Kid3ConfigFileUsingOnlyTagLib():
            self._run_mp4_tests()

    def test_mp4_mp4v2(self):
        with Kid3ConfigFileUsingOnlyMp4v2():
            self._run_mp4_tests()

    def _run_mp4_tests(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            path = os.path.join(tmpdir, 'test.m4a')
            jpgpath = os.path.join(tmpdir, 'test.jpg')
            picpath = os.path.join(tmpdir, 'folder.jpg')
            lrcpath = os.path.join(tmpdir, 'chapters.lrc')
            chpneropath = os.path.join(tmpdir, 'chpnero.lrc')
            chpqtpath = os.path.join(tmpdir, 'chpqt.lrc')
            create_test_file(path)
            create_test_file(jpgpath)
            create_test_file(lrcpath)
            with open(lrcpath, 'rb') as lrcfh:
                lrc_bytes = lrcfh.read()
            with open(jpgpath, 'rb') as jpgfh:
                jpg_bytes = jpgfh.read()
            actual = call_kid3_cli(
                ['-c', 'get all 2', path])
            if not actual:
                # Do not fail if Mp4v2Metadata plugin is not present
                return
            expected = ignore_audio_properties(
                'File: MP4 AAC 16 bit 9 kbps 44100 Hz 2 Channels\n'
                '  Name: test.m4a\n')
            self.assertEqual(ignore_audio_properties(actual), expected)
            actual = ignore_audio_properties(call_kid3_cli(
                ['-c', 'set "Title" "Title"',
                 '-c', 'set "Artist" "Artist"',
                 '-c', 'set "Album" "Album"',
                 '-c', 'set "Comment" "Comment"',
                 '-c', 'set "Date" "2026"',
                 '-c', 'set "Track Number" "3/4"',
                 '-c', 'set "Genre" "Genre"',
                 '-c', 'set "Album Artist" "Album Artist"',
                 '-c', 'set "Author" "Author"',
                 '-c', 'set "BPM" "120"',
                 '-c', 'set "Compilation" "1"',
                 '-c', 'set "Composer" "Composer"',
                 '-c', 'set "Copyright" "Copyright"',
                 '-c', 'set "Description" "Description"',
                 '-c', 'set "Disc Number" "1/2"',
                 '-c', 'set "Encoded-by" "Encoded-by"',
                 '-c', 'set "Encoder Settings" "Encoder Settings"',
                 '-c', 'set "Grouping" "Grouping"',
                 '-c', 'set "Lyrics" "Lyrics"',
                 '-c', 'set "Rating" "Rating"',
                 '-c', 'set "Sort Album" "Sort Album"',
                 '-c', 'set "Sort Album Artist" "Sort Album Artist"',
                 '-c', 'set "Sort Artist" "Sort Artist"',
                 '-c', 'set "Sort Composer" "Sort Composer"',
                 '-c', 'set "Sort Name" "Sort Name"',
                 '-c', 'set "Work" "Work"',
                 '-c', 'set "pgap" "1"',
                 '-c', 'set "akID" "23"',
                 '-c', 'set "apID" "Purchase Account"',
                 '-c', 'set "atID" "67"',
                 '-c', 'set "catg" "Category"',
                 '-c', 'set "cnID" "32"',
                 '-c', 'set "geID" "31"',
                 '-c', 'set "hdvd" "1"',
                 '-c', 'set "keyw" "Keywords"',
                 '-c', 'set "ldes" "Long Description"',
                 '-c', 'set "pcst" "1"',
                 '-c', 'set "plID" "1234567890"',
                 '-c', 'set "purd" "Purchase Date"',
                 '-c', 'set "rtng" "23"',
                 '-c', 'set "sfID" "7"',
                 '-c', 'set "sosn" "Sort TV Show"',
                 '-c', 'set "stik" "23"',
                 '-c', 'set "tven" "TV Episode Number"',
                 '-c', 'set "tves" "89"',
                 '-c', 'set "tvnn" "TV Network"',
                 '-c', 'set "tvsh" "Show"',
                 '-c', 'set "tvsn" "99"',
                 '-c', 'set "purl" "purl"',
                 '-c', 'set "egid" "egid"',
                 '-c', 'set "cmID" "33"',
                 '-c', 'set "xid " "xid "',
                 '-c', 'save',
                 '-c', 'get all 2', path]))
            expected = ignore_audio_properties(
                'File: MP4 9 kbps 44100 Hz 2 Channels\n'
                '  Name: test.m4a\n'
                'Tag 2: MP4\n'
                '  Title              Title\n'
                '  Artist             Artist\n'
                '  Album              Album\n'
                '  Comment            Comment\n'
                '  Date               2026\n'
                '  Track Number       3/4\n'
                '  Genre              Genre\n'
                '  Album Artist       Album Artist\n'
                '  Author             Author\n'
                '  BPM                120\n'
                '  Compilation        1\n'
                '  Composer           Composer\n'
                '  Copyright          Copyright\n'
                '  Disc Number        1/2\n'
                '  Encoded-by         Encoded-by\n'
                '  Encoder Settings   Encoder Settings\n'
                '  Grouping           Grouping\n'
                '  Lyrics             Lyrics\n'
                '  Description        Description\n'
                '  Sort Album         Sort Album\n'
                '  Sort Album Artist  Sort Album Artist\n'
                '  Sort Artist        Sort Artist\n'
                '  Sort Composer      Sort Composer\n'
                '  Sort Name          Sort Name\n'
                '  Rating             Rating\n'
                '  Work               Work\n'
                '  Account Type       23\n'
                '  Purchase Account   Purchase Account\n'
                '  Artist ID          67\n'
                '  Category           Category\n'
                '  Composer ID        33\n'
                '  Catalog ID         32\n'
                '  Podcast GUID       egid\n'
                '  Genre ID           31\n'
                '  HD Video           1\n'
                '  Keyword            Keywords\n'
                '  Long Description   Long Description\n'
                '  Podcast            1\n'
                '  Gapless Playback   1\n'
                '  Album ID           1234567890\n'
                '  Purchase Date      Purchase Date\n'
                '  Podcast URL        purl\n'
                '  Rating/Advisory    23\n'
                '  Country Code       7\n'
                '  Sort Show          Sort TV Show\n'
                '  Media Type         23\n'
                '  TV Episode         TV Episode Number\n'
                '  TV Episode Number  89\n'
                '  TV Network Name    TV Network\n'
                '  TV Show Name       Show\n'
                '  TV Season          99\n'
                '  XID                xid \n')
            self.assertEqual(actual, expected)
            # Called separately because libmp4v2 prints
            # ReadChildAtoms: "/tmp/tmp08j22ihf/test.m4a": In atom meta missing child atom ilst
            call_kid3_cli(['-c', 'remove', '-c', 'set title Title', path])
            call_kid3_cli(
                ['-c', 'set picture:"%s" ""' % jpgpath,
                 '-c', 'set "Chapters (Nero):%s" ""' % lrcpath,
                 '-c', 'set "Chapters (QT):%s" ""' % lrcpath,
                 path])
            actual = ignore_audio_properties(call_kid3_cli(
                ['-c', 'get all',
                 '-c', 'get picture:"%s"' % picpath,
                 '-c', 'get "Chapters (Nero):%s"' % chpneropath,
                 '-c', 'get "Chapters (QT):%s"' % chpqtpath,
                 path]))
            expected = ignore_audio_properties(
                'File: MP4 AAC 16 bit 9 kbps 44100 Hz 2 Channels\n'
                '  Name: test.m4a\n'
                'Tag 2: MP4\n'
                '  Title                   Title\n'
                '  Picture: Cover (front)  \n'
                '  Chapters (Nero)         \n'
                '  Chapters (QT)           \n'
                '\n\n\n')
            if 'Chapters (Nero)' not in actual:
                # Using a TagLib version which does not support MP4 chapters
                self.assertEqual(actual, ignore_audio_properties(
                    'File: MP4 n kbps 44100 Hz 2 Channels\n'
                    '  Name: test.m4a\n'
                    'Tag 2: MP4\n'
                    '  Title                   Title\n'
                    '  Picture: Cover (front)  \n'
                    '\n'))
                return
            self.assertEqual(actual, expected)
            with open(picpath, 'rb') as jpgfh:
                ba = jpgfh.read()
                self.assertEqual(ba, jpg_bytes)
            with open(chpneropath, 'rb') as lrcfh:
                ba = lrcfh.read()
                self.assertEqual(ba, lrc_bytes)
            with open(chpqtpath, 'rb') as lrcfh:
                ba = lrcfh.read()
                self.assertEqual(ba, lrc_bytes)

    def test_frame_selection(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            test1path = os.path.join(tmpdir, 'test1.mp3')
            test2path = os.path.join(tmpdir, 'test2.mp3')
            create_test_file(test1path)
            create_test_file(test2path)
            self.assertEqual(call_kid3_cli(
                ['-c', 'get all 2', test1path]),
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test1.mp3\n')
            self.assertEqual(call_kid3_cli(
                ['-c', 'get all 2', test2path]),
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test2.mp3\n')
            expected = (
                'Tag 2:\n'
                '  Title        ≠\n'
                '  Artist       An Artist\n'
                '  Album        An Album\n'
                '  Copyright    ≠\n'
                '  Disc Number  ≠\n')
            self.assertEqual(call_kid3_cli(
                ['-c', 'select all',
                 '-c', 'set artist "An Artist"',
                 '-c', 'set album "An Album"',
                 '-c', 'select none',
                 '-c', 'select first',
                 '-c', 'set title "Title 1"',
                 '-c', 'set discnumber "1/2"',
                 '-c', 'set copyright "2017 Kid3"',
                 '-c', 'select next',
                 '-c', 'set title "Title 2"',
                 '-c', 'save',
                 '-c', 'select all',
                 '-c', 'get', tmpdir]),
                expected)
            self.assertEqual(call_kid3_cli(
                ['-c', 'select first',
                 '-c', 'get',
                 '-c', 'set "*.selected" 0',
                 '-c', 'set "album.selected" 1',
                 '-c', 'remove',
                 '-c', 'get',
                 '-c', 'revert',
                 '-c', 'set "*.selected" 0',
                 '-c', 'set "discnumber.selected" 1',
                 '-c', 'set "copyright.selected" 1',
                 '-c', 'copy',
                 '-c', 'select next',
                 '-c', 'paste',
                 '-c', 'get',
                 '-c', 'revert',
                 '-c', 'select all',
                 '-c', 'set "*.selected" 0',
                 '-c', 'set "discnumber.selected" 1',
                 '-c', 'paste',
                 '-c', 'select first',
                 '-c', 'get',
                 '-c', 'select next',
                 '-c', 'get', tmpdir]),
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test1.mp3\n'
                'Tag 2: ID3v2.3.0\n'
                '  Title        Title 1\n'
                '  Artist       An Artist\n'
                '  Album        An Album\n'
                '  Copyright    2017 Kid3\n'
                '  Disc Number  1/2\n'
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test1.mp3\n'
                'Tag 2: ID3v2.3.0\n'
                '  Title        Title 1\n'
                '  Artist       An Artist\n'
                '  Copyright    2017 Kid3\n'
                '  Disc Number  1/2\n'
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test2.mp3\n'
                'Tag 2: ID3v2.3.0\n'
                '  Title        Title 2\n'
                '  Artist       An Artist\n'
                '  Album        An Album\n'
                '* Copyright    2017 Kid3\n'
                '* Disc Number  1/2\n'
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test1.mp3\n'
                'Tag 2: ID3v2.3.0\n'
                '  Title        Title 1\n'
                '  Artist       An Artist\n'
                '  Album        An Album\n'
                '  Copyright    2017 Kid3\n'
                '  Disc Number  1/2\n'
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test2.mp3\n'
                'Tag 2: ID3v2.3.0\n'
                '  Title        Title 2\n'
                '  Artist       An Artist\n'
                '  Album        An Album\n'
                '* Disc Number  1/2\n')

    def test_riff_info(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            wavpath = os.path.join(tmpdir, 'test.wav')
            create_test_file(wavpath)
            with open(wavpath, 'rb') as wavfh:
                empty_wav_bytes = wavfh.read()
            self.assertRegex(call_kid3_cli(
                ['-c', 'get title 3',
                 '-c', 'get all 3', wavpath]),
                'File: WAV (?:PCM 16 bit )?1411 kbps 44100 Hz 2 Channels\n'
                '  Name: test\\.wav\n')
            self.assertRegex(call_kid3_cli(
                ['-c', 'set artist "An Artist" 3',
                 '-c', 'set album "An Album" 3',
                 '-c', 'set tracknumber 2 3',
                 '-c', 'set genre "Heavy Metal" 3',
                 '-c', 'set date 2017 3',
                 '-c', 'set comment "A Comment" 3',
                 '-c', 'set bpm "120" 3',
                 '-c', 'set comment "A Comment" 3',
                 '-c', 'get all 3', wavpath]),
                'File: WAV (?:PCM 16 bit )?1411 kbps 44100 Hz 2 Channels\n'
                '  Name: test\\.wav\n'
                '(?:Tag 3: RIFF INFO\n'
                '\\* Artist        An Artist\n'
                '\\* Album         An Album\n'
                '\\* Comment       A Comment\n'
                '\\* Date          2017\n'
                '\\* Track Number  2\n'
                '\\* Genre         Heavy Metal\n'
                '\\* BPM           120\n)?')
            self.assertRegex(call_kid3_cli(
                ['-c', 'copy 3',
                 '-c', 'remove 3',
                 '-c', 'get all 3',
                 '-c', 'paste 3',
                 '-c', 'get all 3',
                 '-c', 'set tracknumber 6 3',
                 '-c', 'get all 3',
                 '-c', 'set comment "" 3',
                 '-c', 'get all 3',
                 '-c', 'remove 3', wavpath]),
                'File: WAV (?:PCM 16 bit )?1411 kbps 44100 Hz 2 Channels\n'
                '  Name: test\\.wav\n'
                'File: WAV (?:PCM 16 bit )?1411 kbps 44100 Hz 2 Channels\n'
                '  Name: test\\.wav\n'
                '(?:Tag 3: RIFF INFO\n'
                '\\* Artist        An Artist\n'
                '\\* Album         An Album\n'
                '\\* Comment       A Comment\n'
                '\\* Date          2017\n'
                '\\* Track Number  2\n'
                '\\* Genre         Heavy Metal\n'
                '\\* BPM           120\n)?'
                'File: WAV (?:PCM 16 bit )?1411 kbps 44100 Hz 2 Channels\n'
                '  Name: test\\.wav\n'
                '(?:Tag 3: RIFF INFO\n'
                '\\* Artist        An Artist\n'
                '\\* Album         An Album\n'
                '\\* Comment       A Comment\n'
                '\\* Date          2017\n'
                '\\* Track Number  6\n'
                '\\* Genre         Heavy Metal\n'
                '\\* BPM           120\n)?'
                'File: WAV (?:PCM 16 bit )?1411 kbps 44100 Hz 2 Channels\n'
                '  Name: test\\.wav\n'
                '(?:Tag 3: RIFF INFO\n'
                '\\* Artist        An Artist\n'
                '\\* Album         An Album\n'
                '\\* Date          2017\n'
                '\\* Track Number  6\n'
                '\\* Genre         Heavy Metal\n'
                '\\* BPM           120\n)?')
            with open(wavpath, 'rb') as wavfh:
                ba = wavfh.read()
                self.assertEqual(ba, empty_wav_bytes)

    def test_multiple_files(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            extensions = ('m4a', 'flac', 'spx', 'mp3', 'ape', 'wav', 'aif', 'wma')
            for nr, ext in enumerate(extensions):
                create_test_file(os.path.join(tmpdir, 'track%02d.%s' % (nr, ext)))
            import_csv = \
                '01\tWheels Of Fire\tManowar\tKings Of Metal\t1988\tMetal\t\t0:00.00\n' \
                '02\tKings Of Metal\tManowar\tKings Of Metal\t1988\tMetal\t\t0:00.00\n' \
                '03\tHeart Of Steel\tManowar\tKings Of Metal\t1988\tMetal\t\t0:00.00\n' \
                '05\tThe Crown And The Ring (Lament Of The Kings)\tManowar\tKings Of Metal\t1988\tMetal\t\t0:00.00\n' \
                '06\tKingdom Come\tManowar\tKings Of Metal\t1988\tMetal\t\t0:00.00\n' \
                '08\tHail And Kill\tManowar\tKings Of Metal\t1988\tMetal\t\t0:00.00\n' \
                '09\tThe Warriors Prayer\tManowar\tKings Of Metal\t1988\tMetal\t\t0:00.00\n' \
                '10\tBlood Of The Kings\tManowar\tKings Of Metal\t1988\tMetal\t\t0:00.00\n'
            importpath = os.path.join(tmpdir, 'import.csv')
            exportpath = os.path.join(tmpdir, 'export.csv')
            with open(importpath, 'w') as importfh:
                importfh.write(import_csv)
            self.assertEqual(call_kid3_cli(
                ['-c', 'import "%s" "CSV unquoted"' % importpath,
                 '-c', 'export "%s" "CSV unquoted"' % exportpath, tmpdir]), '')
            with open(exportpath) as exportfh:
                export_csv = exportfh.read()
                self.assertEqual(export_csv, import_csv)
            os.remove(importpath)
            os.remove(exportpath)
            expected = (
                'File: MP4 (?:AAC 16 bit )?9 kbps 44100 Hz 2 Channels\n'
                '  Name: track00\\.m4a\n'
                'Tag 2: MP4\n'
                '  Title         Wheels Of Fire\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  1\n'
                '  Genre         Metal\n'
                'File: FLAC [^\\n]+ kbps 44100 Hz 1 Channels\n'
                '  Name: track01\\.flac\n'
                '(?:Tag 1: ID3v1\\.1\n'
                '  Title         Kings Of Metal\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  02\n'
                '  Genre         Metal\n)?'
                'Tag 2: Vorbis\n'
                '  Title         Kings Of Metal\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  02\n'
                '  Genre         Metal\n'
                'File: Speex 1 44100 Hz 1 Channels\n'
                '  Name: track02\\.spx\n'
                'Tag 2: Vorbis\n'
                '  Title         Heart Of Steel\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  03\n'
                '  Genre         Metal\n'
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: track03\\.mp3\n'
                'Tag 1: ID3v1\\.1\n'
                '  Title         The Crown And The Ring \\(Lament\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  5\n'
                '  Genre         Metal\n'
                'Tag 2: ID3v2\\.3\\.0\n'
                '  Title         The Crown And The Ring \\(Lament Of The Kings\\)\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  05\n'
                '  Genre         Metal\n'
                'File: APE 3\\.990 16 bit 44100 Hz 1 Channels\n'
                '  Name: track04\\.ape\n'
                'Tag 1: ID3v1\\.1\n'
                '  Title         Kingdom Come\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  6\n'
                '  Genre         Metal\n'
                'Tag 2: APE\n'
                '  Title         Kingdom Come\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  06\n'
                '  Genre         Metal\n'
                'File: WAV (?:PCM 16 bit )?1411 kbps 44100 Hz 2 Channels\n'
                '  Name: track05\\.wav\n'
                'Tag 2: ID3v2\\.3\\.0\n'
                '  Title         Hail And Kill\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  08\n'
                '  Genre         Metal\n'
                'File: AIFF [^\\n]+ kbps 44100 Hz 2 Channels\n'
                '  Name: track06\\.aif\n'
                'Tag 2: ID3v2\\.4\\.0\n'
                '  Title         The Warriors Prayer\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  09\n'
                '  Genre         Metal\n'
                'File: ASF 128 kbps 44100 Hz 1 Channels\n'
                '  Name: track07.wma\n'
                'Tag 2: ASF\n'
                '  Title               Blood Of The Kings\n'
                '  Artist              Manowar\n'
                '  Album               Kings Of Metal\n'
                '  Comment             \n'
                '  Date                1988\n'
                '  Track Number        10\n'
                '  Genre               Metal\n'
                '  Copyright           \n'
                '  Encoder Settings    Lavf61.7.102\n'
                '  Rating Information  \n'
                'Tag 1:\n'
                '  Title         ≠\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  ≠\n'
                '  Genre         Metal\n'
                'Tag 2:\n'
                '  Title               ≠\n'
                '  Artist              Manowar\n'
                '  Album               Kings Of Metal\n'
                '  Date                1988\n'
                '  Track Number        ≠\n'
                '  Genre               Metal\n'
                '  Copyright           ≠\n'
                '  Encoder Settings    ≠\n'
                '  Rating Information  ≠\n'
                )
            self.assertRegex(call_kid3_cli(
                ['-c', 'select first',
                 '-c', 'get',
                 '-c', 'select next',
                 '-c', 'get',
                 '-c', 'select next',
                 '-c', 'get',
                 '-c', 'select next',
                 '-c', 'get',
                 '-c', 'select next',
                 '-c', 'get',
                 '-c', 'select next',
                 '-c', 'get',
                 '-c', 'select next',
                 '-c', 'get',
                 '-c', 'select next',
                 '-c', 'get',
                 '-c', 'select all',
                 '-c', 'get', tmpdir]),
                expected)
            actual = call_kid3_cli(
                ['-c', 'select all',
                 '-c', 'set Date "" 2',
                 '-c', 'set Artist "" 1',
                 '-c', 'set Album "" 2',
                 '-c', 'get',
                 '-c', 'revert',
                 '-c', 'get',
                 tmpdir])
            expected = ('Tag 1:\n'
                        '  Title         ≠\n'
                        '  Album         Kings Of Metal\n'
                        '  Date          1988\n'
                        '  Track Number  ≠\n'
                        '  Genre         Metal\n'
                        'Tag 2:\n'
                        '  Title               ≠\n'
                        '  Artist              Manowar\n'
                        '  Track Number        ≠\n'
                        '  Genre               Metal\n'
                        '  Copyright           ≠\n'
                        '  Encoder Settings    ≠\n'
                        '  Rating Information  ≠\n'
                        'Tag 1:\n'
                        '  Title         ≠\n'
                        '  Artist        Manowar\n'
                        '  Album         Kings Of Metal\n'
                        '  Date          1988\n'
                        '  Track Number  ≠\n'
                        '  Genre         Metal\n'
                        'Tag 2:\n'
                        '  Title               ≠\n'
                        '  Artist              Manowar\n'
                        '  Album               Kings Of Metal\n'
                        '  Date                1988\n'
                        '  Track Number        ≠\n'
                        '  Genre               Metal\n'
                        '  Copyright           ≠\n'
                        '  Encoder Settings    ≠\n'
                        '  Rating Information  ≠\n')
            self.assertEqual(actual, expected)
            call_kid3_cli(['-c', 'fromtag "%{track} %{title}" 2', os.path.join(tmpdir, '*.*')])
            self.assertRegex(call_kid3_cli(
                ['-c', 'ls', tmpdir]),
                '  -2- 01 Wheels Of Fire.m4a\n'
                '  [-1]2- 02 Kings Of Metal.flac\n'
                '  -2- 03 Heart Of Steel.spx\n'
                '  12- 05 The Crown And The Ring \\(Lament Of The Kings\\).mp3\n'
                '  12- 06 Kingdom Come.ape\n'
                '  -2- 08 Hail And Kill.wav\n'
                '  -2- 09 The Warriors Prayer.aif\n'
                '  -2- 10 Blood Of The Kings.wma\n')
            self.assertEqual(call_kid3_cli(
                ['-c', 'tag 2',
                 '-c', 'numbertracks 101',
                 '-c', 'select first',
                 '-c', 'get track',
                 '-c', 'select next',
                 '-c', 'get track',
                 '-c', 'select next',
                 '-c', 'get track',
                 '-c', 'select next',
                 '-c', 'get track',
                 '-c', 'select next',
                 '-c', 'get track',
                 '-c', 'select next',
                 '-c', 'get track',
                 '-c', 'select next',
                 '-c', 'get track',
                 '-c', 'select next',
                 '-c', 'get track',
                 '-c', 'revert', tmpdir
                 ]),
                '101\n102\n103\n104\n105\n106\n107\n108\n')
            lines = call_kid3_cli(
                ['-c', 'filter "%{tag2} equals ID3v2.3.0"', tmpdir]).splitlines()
            self.assertEqual(lines[0], 'Started')
            self.assertTrue(os.path.samefile(lines[1].lstrip(), tmpdir))
            self.assertEqual(
                lines[2:],
                ['- 01 Wheels Of Fire.m4a', '- 02 Kings Of Metal.flac',
                 '- 03 Heart Of Steel.spx', '+ 05 The Crown And The Ring (Lament Of The Kings).mp3',
                 '- 06 Kingdom Come.ape', '+ 08 Hail And Kill.wav', '- 09 The Warriors Prayer.aif',
                 '- 10 Blood Of The Kings.wma', 'Finished'])
            call_kid3_cli(['-c', 'renamedir "%{artist} - [%{year}] %{album}" "create"', tmpdir])
            new_dirname = 'Manowar - [1988] Kings Of Metal'
            new_dirpath = os.path.join(tmpdir, new_dirname)
            self.assertEqual(os.listdir(tmpdir)[0], new_dirname)
            call_kid3_cli(['-c', 'playlist', new_dirpath])
            with open(os.path.join(new_dirpath, new_dirname + '.m3u')) as m3ufh:
                export_m3u = m3ufh.read()
            self.assertEqual(
                export_m3u,
                '01 Wheels Of Fire.m4a\n'
                '02 Kings Of Metal.flac\n'
                '03 Heart Of Steel.spx\n'
                '05 The Crown And The Ring (Lament Of The Kings).mp3\n'
                '06 Kingdom Come.ape\n'
                '08 Hail And Kill.wav\n'
                '09 The Warriors Prayer.aif\n'
                '10 Blood Of The Kings.wma\n')

    def test_ape_picture_and_custom_frame(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            apepath = os.path.join(tmpdir, 'test.ape')
            jpgpath = os.path.join(tmpdir, 'cover.jpg')
            create_test_file(apepath)
            create_test_file(jpgpath)
            self.assertEqual(call_kid3_cli(
                ['-c', 'set picture:"%s" "FrontCover" 2' % jpgpath,
                 '-c', 'get all 2',
                 '-c', 'set picture "" 2',
                 '-c', 'get all 2',
                 '-c', 'set !MY_CUSTOM "XValue" 2',
                 '-c', 'get all 2',
                 apepath]),
                'File: APE 3.990 16 bit 44100 Hz 1 Channels\n'
                '  Name: test.ape\n'
                'Tag 2: APE\n'
                '* Picture: Cover (front)  FrontCover\n'
                'File: APE 3.990 16 bit 44100 Hz 1 Channels\n'
                '  Name: test.ape\n'
                'File: APE 3.990 16 bit 44100 Hz 1 Channels\n'
                '  Name: test.ape\n'
                'Tag 2: APE\n'
                '* MY_CUSTOM  XValue\n')

    def test_wavpack_and_mpc_ape_tags(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            wavpack_path = os.path.join(tmpdir, 'test.wv')
            mpc_path = os.path.join(tmpdir, 'test.mpc')
            create_test_file(wavpack_path)
            create_test_file(mpc_path)
            self.assertEqual(call_kid3_cli(
                ['-c', 'set title "WvTitle" 2',
                 '-c', 'get all 2', wavpack_path]),
                'File: WavPack 407 16 bit 44100 Hz 1 Channels\n'
                '  Name: test.wv\n'
                'Tag 2: APE\n'
                '* Title  WvTitle\n')
            self.assertEqual(call_kid3_cli(
                ['-c', 'set title "MpcTitle" 2',
                 '-c', 'get all 2', mpc_path]),
                'File: MPC 44100 Hz 1 Channels\n'
                '  Name: test.mpc\n'
                'Tag 2: APE\n'
                '* Title  MpcTitle\n')

    def test_asf_and_wma_tag_edit_commands(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            path = os.path.join(tmpdir, 'test.wma')
            jpgpath = os.path.join(tmpdir, 'cover.jpg')
            create_test_file(path)
            create_test_file(jpgpath)
            expected = ('File: ASF 128 kbps 44100 Hz 1 Channels\n'
                        '  Name: test.wma\n'
                        'Tag 2: ASF\n'
                        '* Title               AsfTitle\n'
                        '* Artist              AsfArtist\n'
                        '  Comment             \n'
                        '  Copyright           \n'
                        '  Encoder Settings    Lavf61.7.102\n'
                        '  Rating Information  \n')
            actual = call_kid3_cli(
                ['-c', 'set title "AsfTitle" 2',
                 '-c', 'set artist "AsfArtist" 2',
                 '-c', 'get all 2', path])
            self.assertEqual(actual, expected)
            actual = call_kid3_cli(
                ['-c', 'set CUSTOM "Custom Value"',
                 '-c', 'set picture:"%s" "CoverAsf"' % jpgpath,
                 '-c', 'set copyright "2026 Artist"',
                 '-c', 'set comment "A comment"',
                 '-c', 'set AverageLevel 1234',
                 '-c', 'set WM/MediaClassPrimaryID 6c1adf28-040e-425e-a90b-df1f2f6262f0',
                 '-c', 'get all 2',
                 '-c', 'remove',
                 '-c', 'get all 2', path])
            # Do not set rating, it does not behave consistently with older versions
            expected = ('File: ASF 128 kbps 44100 Hz 1 Channels\n'
                        '  Name: test.wma\n'
                        'Tag 2: ASF\n'
                        '  Title                   AsfTitle\n'
                        '  Artist                  AsfArtist\n'
                        '* Comment                 A comment\n'
                        '* Copyright               2026 Artist\n'
                        '  Encoder Settings        Lavf61.7.102\n'
                        '* Picture: Cover (front)  CoverAsf\n'
                        '* AverageLevel            1234\n'
                        '* CUSTOM                  Custom Value\n'
                        '  Rating Information      \n'
                        '* WM/MediaClassPrimaryID  6C1ADF28-040E-425E-A90B-DF1F2F6262F0\n'
                        'File: ASF 128 kbps 44100 Hz 1 Channels\n'
                        '  Name: test.wma\n'
                        'Tag 2:\n'
                        '  Title               \n'
                        '  Artist              \n'
                        '* Comment             \n'
                        '* Copyright           \n'
                        '  Rating Information  \n')
            self.assertEqual(actual, expected)


    def test_dsf_tags_and_picture(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            dsfpath = os.path.join(tmpdir, 'test.dsf')
            jpgpath = os.path.join(tmpdir, 'cover.jpg')
            create_test_file(dsfpath)
            create_test_file(jpgpath)
            out = call_kid3_cli(
                ['-c', 'set title "DsfTitle" 2',
                 '-c', 'set artist "DsfArtist" 2',
                 '-c', 'set album "DsfAlbum" 2',
                 '-c', 'set date 2026 2',
                 '-c', 'set track 7 2',
                 '-c', 'set genre Metal 2',
                 '-c', 'set picture:"%s" "CoverDsf" 2' % jpgpath,
                 '-c', 'get all 2', dsfpath])
            self.assertRegex(out,
                'File: DSF 1(?: 5645 kbps 2822(?:400|528) Hz 2 Channels)?\\n'
                '  Name: test\\.dsf\\n'
                'Tag 2: ID3v2\\.[34]\\.0\\n'
                '\\* Title\\s+DsfTitle\\n'
                '\\* Artist\\s+DsfArtist\\n'
                '\\* Album\\s+DsfAlbum\\n'
                '\\* Date\\s+2026\\n'
                '\\* Track Number\\s+7(?:/1)?\\n'
                '\\* Genre\\s+Metal\\n'
                '\\* Picture: Cover \\(front\\)\\s+CoverDsf\\n')

    def test_matroska(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            path = os.path.join(tmpdir, 'test.mkv')
            create_test_file(path)
            actual = call_kid3_cli(
                ['-c', 'set title "MatroskaTitle" 2',
                 '-c', 'set artist "MatroskaArtist" 2',
                 '-c', 'set album "MatroskaAlbum" 2',
                 '-c', 'get all 2', path])
            if not actual:
                # Do not fail if running tests with TagLib not supporting Matroska
                return
            expected = ("File: Matroska Version 4 45 kbps 0:01\n"
                        "  Name: test.mkv\n"
                        "Tag 2: Matroska\n"
                        "* Title                         MatroskaTitle\n"
                        "* Artist                        MatroskaArtist\n"
                        "* Album                         MatroskaAlbum\n"
                        "  BPS                           16\n"
                        "  Duration                      00:00:01.000000000\n"
                        "  NUMBER_OF_BYTES               2\n"
                        "  NUMBER_OF_FRAMES              1\n"
                        "  _STATISTICS_TAGS              BPS DURATION NUMBER_OF_FRAMES NUMBER_OF_BYTES\n"
                        "  _STATISTICS_WRITING_APP       mkvmerge v99.0 ('Buka') 64-bit\n"
                        "  _STATISTICS_WRITING_DATE_UTC  2026-05-31 05:43:02\n")
            self.assertEqual(actual, expected)

            path = os.path.join(tmpdir, 'test.mka')
            jpgpath = os.path.join(tmpdir, 'test.jpg')
            picpath = os.path.join(tmpdir, 'folder.jpg')
            lrcpath = os.path.join(tmpdir, 'chapters.lrc')
            chppath = os.path.join(tmpdir, 'chp.lrc')
            create_test_file(path)
            create_test_file(jpgpath)
            create_test_file(lrcpath)
            with open(lrcpath, 'rb') as lrcfh:
                lrc_bytes = lrcfh.read()
            with open(jpgpath, 'rb') as jpgfh:
                jpg_bytes = jpgfh.read()
            actual = call_kid3_cli(
                ['-c', 'set title "MatroskaTitle" 2',
                 '-c', 'set artist "MatroskaArtist" 2',
                 '-c', 'set album "MatroskaAlbum" 2',
                 '-c', 'get all 2', path])
            expected = ('File: Matroska Version 4 Codec A_PCM/INT/LIT 44100 Hz 2 Channels\n'
                        '  Name: test.mka\n'
                        'Tag 2: Matroska\n'
                        '* Title                         MatroskaTitle\n'
                        '* Artist                        MatroskaArtist\n'
                        '* Album                         MatroskaAlbum\n'
                        '  BPS                           2368000\n'
                        '  Chapters                      {"default":false}\n'
                        '  Duration                      00:00:00.001677876\n'
                        '  General Object                \n'
                        '  NUMBER_OF_BYTES               296\n'
                        '  NUMBER_OF_FRAMES              1\n'
                        '  _STATISTICS_TAGS              BPS DURATION NUMBER_OF_FRAMES NUMBER_OF_BYTES\n'
                        '  _STATISTICS_WRITING_APP       mkvmerge v99.0 (\'Buka\') 64-bit\n'
                        '  _STATISTICS_WRITING_DATE_UTC  2026-05-31 05:46:39\n')
            self.assertEqual(actual, expected)

            call_kid3_cli(
                ['-c', 'set Title Title',
                 '-c', 'set Artist ""',
                 '-c', 'set Album ""',
                 '-c', 'set _STATISTICS_TAGS ""',
                 '-c', 'set _STATISTICS_WRITING_APP ""',
                 '-c', 'set _STATISTICS_WRITING_DATE_UTC ""',
                 '-c', 'set NUMBER_OF_BYTES ""',
                 '-c', 'set NUMBER_OF_FRAMES 3',
                 '-c', 'set "General Object.Description" "ObjDesc"',
                 '-c', 'set "Chapters:%s" ""' % lrcpath,
                 '-c', 'set Picture:"%s" ""' % jpgpath,
                 path])
            actual = call_kid3_cli(
                ['-c', 'get',
                 '-c', 'get "Chapters:%s"' % chppath,
                 '-c', 'get Picture:"%s"' % picpath,
                 '-c', 'get Duration.TargetType',
                 '-c', 'get Duration.Language',
                 '-c', 'get Duration.Default',
                 '-c', 'get Duration.TrackId',
                 '-c', 'get Duration.Text',
                 path])
            expected = ('File: Matroska Version 4 Codec A_PCM/INT/LIT 44100 Hz 2 Channels\n'
                        '  Name: test.mka\n'
                        'Tag 2: Matroska\n'
                        '  Title                   Title\n'
                        '  Picture: Cover (front)  \n'
                        '  BPS                     2368000\n'
                        '  Chapters                \n'
                        '  Duration                00:00:00.001677876\n'
                        '  General Object          ObjDesc\n'
                        '  NUMBER_OF_FRAMES        3\n'
                        '\n'
                        '\n'
                        '0\n'
                        'und\n'
                        'true\n'
                        '5324681441320058677\n'
                        '00:00:00.001677876\n')
            self.assertEqual(actual, expected)
            with open(chppath, 'rb') as lrcfh:
                ba = lrcfh.read()
                self.assertEqual(ba, lrc_bytes)
            with open(picpath, 'rb') as jpgfh:
                ba = jpgfh.read()
                self.assertEqual(ba, jpg_bytes)
            actual = call_kid3_cli(
                ['-c', 'remove',
                 '-c', 'get',
                 path])
            expected = ('File: Matroska Version 4 Codec A_PCM/INT/LIT 44100 Hz 2 Channels\n'
                        '  Name: test.mka\n')
            self.assertEqual(actual, expected)

    def test_filename_tag_format(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            albumdir = os.path.join(tmpdir, 'An Artist - 2016 - An Album')
            os.mkdir(albumdir)
            create_test_file(os.path.join(albumdir, '01. A title.mp3'))
            self.assertEqual(call_kid3_cli(
                ['-c', 'totag "%{artist} - %{date} - %{album}/%{track}. %{title}" 2',
                 '-c', 'save',
                 '-c', 'get', os.path.join(albumdir, '*.mp3')]),
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: 01. A title.mp3\n'
                'Tag 2: ID3v2.3.0\n'
                '  Title         A title\n'
                '  Artist        An Artist\n'
                '  Album         An Album\n'
                '  Date          2016\n'
                '  Track Number  1\n')
            self.assertEqual(call_kid3_cli(
                ['-c', 'select all',
                 '-c', 'tagformat',
                 '-c', 'save',
                 '-c', 'get all', albumdir]),
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: 01. A title.mp3\n'
                'Tag 2: ID3v2.3.0\n'
                '  Title         A Title\n'
                '  Artist        An Artist\n'
                '  Album         An Album\n'
                '  Date          2016\n'
                '  Track Number  1\n')
            expected = (
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '* Name: 01. Schön.mp3\n'
                'Tag 2: ID3v2.3.0\n'
                '* Title         Schön\n'
                '  Artist        An Artist\n'
                '  Album         An Album\n'
                '  Date          2016\n'
                '  Track Number  1\n'
            )
            self.assertEqual(call_kid3_cli(
                ['-c', 'select first',
                 '-c', 'set title "Schön"',
                 '-c', 'fromtag "%{track}. %{title}" 2',
                 '-c', 'get',
                 '-c', 'filenameformat', albumdir]),
                expected)
            expected = (
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: 01 Schoen.mp3\n'
                'Tag 2: ID3v2.3.0\n'
                '  Title         Schön\n'
                '  Artist        An Artist\n'
                '  Album         An Album\n'
                '  Date          2016\n'
                '  Track Number  1\n'
            )
            self.assertEqual(call_kid3_cli(
                ['-c', 'get', os.path.join(albumdir, '*.mp3')]),
                expected)


class CliFunctionsJsonTestCase(unittest.TestCase):
    def test_invalid(self):
        for cmd, rsp in (
                ('{"abc":"def"}',
                 {'error': {'code': -1,
                            'message': 'missing method: {"abc":"def"}'}}),
                ('{"method":"unknown"}',
                 {'error': {'code': -1,
                            'message': 'Unknown command '
                            '\'{"method":"unknown"}\', -h for help.'}}),
                ('{"method":"timeout",params:["off"]}',
                 {'error': {'code': -1,
                            'message': 'unterminated object: '
                            '{"method":"timeout",params:["off"]}'}}),
                ('{"method":"timeout","params":["default"}',
                 {'error': {'code': -1,
                            'message': 'unterminated array: '
                            '{"method":"timeout","params":["default"}'}}),
                ('{"method":"unknown"}',
                 {'error': {'code': -1,
                            'message': 'Unknown command '
                            '\'{"method":"unknown"}\', -h for help.'}}),
        ):
            p = subprocess.Popen([kid3_cli_path(), '-c', cmd],
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                            universal_newlines=True)
            stdout, stderr = p.communicate()
            result = json.loads(stdout)
            self.assertEqual(result, rsp)
            self.assertEqual(p.returncode, 1)

    def test_invalid_interactive(self):
        p = subprocess.Popen([kid3_cli_path()],
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             universal_newlines=True)
        stdout, stderr = p.communicate('{"method":"unknown"}\n')
        self.assertEqual(json.loads(stdout),
                         {'error': {'code': -32601,
                                    'message': "Unknown command 'unknown'"}})
        self.assertEqual(p.returncode, 0)

    def test_timeout(self):
        self.assertEqual(call_kid3_cli(
            ['-c', '{"method":"timeout"}',
             '-c', '{"method":"timeout","params":[5000]}',
             '-c', '{"method":"timeout"}',
             '-c', '{"method":"timeout","params":["off"]}',
             '-c', '{"method":"timeout"}',
             '-c', '{"method":"timeout","params":["default"]}',
             '-c', '{"method":"timeout"}']),
            '{"result":{"timeout":"default"}}\n'
            '{"result":{"timeout":"5000 ms"}}\n'
            '{"result":{"timeout":"5000 ms"}}\n'
            '{"result":{"timeout":"off"}}\n'
            '{"result":{"timeout":"off"}}\n'
            '{"result":{"timeout":"default"}}\n'
            '{"result":{"timeout":"default"}}\n')

    def test_config(self):
        self.assertEqual(call_kid3_cli(['-c', '{"method":"config"}']),
            '{"result":["BatchImport","Export","File","FilenameFormat",'
            '"Filter","Import","Network","NumberTracks","Playlist",'
            '"RenameFolder","Tag","TagFormat"]}\n')
        self.assertEqual(call_kid3_cli(
            ['-c', '{"method":"config","params":["Tag.id3v2Version"]}',
             '-c', '{"method":"config","params":["Tag.id3v2Version","ID3v2_4_0"]}',
             '-c', '{"method":"config","params":["Tag.id3v2Version"]}',
             '-c', '{"method":"config","params":["Tag.id3v2Version","ID3v2_3_0"]}']),
            '{"result":"ID3v2_3_0"}\n'
            '{"result":"ID3v2_4_0"}\n'
            '{"result":"ID3v2_4_0"}\n'
            '{"result":"ID3v2_3_0"}\n')

        p = subprocess.Popen([kid3_cli_path(), '-c',
                              '{"method":"config","params":["Tag.textEncoding","UTF8"]}'],
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             universal_newlines=True)
        stdout, stderr = p.communicate()
        self.assertNotIn('Invalid value', stderr)
        self.assertEqual(json.loads(stdout),
                         {'error': {'code': -1, 'message': 'Invalid value '}})
        self.assertEqual(p.returncode, 1)

    def test_config_playlist(self):
        lines = call_kid3_cli(
            ['-c', '{"method":"config","params":["Playlist.format","PF_XSPF"]}',
             '-c', '{"method":"config","params":["Playlist.format"]}',
             '-c', '{"method":"config","params":["Playlist.location","PL_EveryDirectory"]}',
             '-c', '{"method":"config","params":["Playlist.location"]}']).splitlines()
        self.assertEqual(lines[0], '{"result":"PF_XSPF"}')
        self.assertEqual(lines[1], '{"result":"PF_XSPF"}')
        self.assertEqual(lines[2], '{"result":"PL_EveryDirectory"}')
        self.assertEqual(lines[3], '{"result":"PL_EveryDirectory"}')

    def test_select_missing_file(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            p = subprocess.Popen([kid3_cli_path(), '-c',
                                  '{"method":"select","params":["no_such_file.mp3"]}',
                                  tmpdir],
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             universal_newlines=True)
            stdout, stderr = p.communicate()
            self.assertEqual(json.loads(stdout),
                             {'error': {'code': -1,
                                        'message': 'no_such_file.mp3 not found'}})
            self.assertEqual(p.returncode, 1)

    def test_usage_error(self):
        p = subprocess.Popen([kid3_cli_path(), '-c', '{"method":"set"}'],
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             universal_newlines=True)
        stdout, stderr = p.communicate()
        self.assertNotIn('Usage: set N V [T]  Set tag frame', stderr)
        self.assertEqual(stdout,
            '{"error":{"code":-32600,"message":"Usage: set N V [T]  Set tag frame"}}\n')
        self.assertEqual(p.returncode, 1)

    def test_json_methods(self):
        self.maxDiff = None
        with tempfile.TemporaryDirectory() as tmpdir:
            mp3path = os.path.join(tmpdir, 'test.mp3')
            create_test_file(mp3path)
            self.assertEqual(call_kid3_cli(
                ['-c', '{"method":"get","params":["title", 2]}',
                 '-c', '{"method":"get","params":["all",[1,2]]}',
                 '-c', '{"method":"set","params":["title","A Title"]}',
                 '-c', '{"method":"get","params":["title"]}',
                 '-c', '{"method": "get", "params": ["title"]}',
                 '-c', '{"method":"get"}',
                 '-c', '{"method":"select","params":["none"]}',
                 '-c', '{"method":"ls"}',
                 '-c', '{"method":"select","params":["first"]}',
                 '-c', '{"method":"filenameformat"}',
                 '-c', '{"method":"tagformat"}',
                 '-c', '{"method":"textencoding"}',
                 '-c', '{"method":"numbertracks","params":[3,2]}',
                 '-c', '{"jsonrpc":"2.0","id":"1","method":"remove",'
                         '"params":[1]}',
                 '-c', '{"method":"ls"}',
                 mp3path]),
                '{"result":null}\n'
                '{"result":{"taggedFile":{"fileName":"test.mp3",'
                '"fileNameChanged":false,"format":'
                '"MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels"}}}\n'
                '{"result":null}\n'
                '{"result":"A Title"}\n'
                '{\n'
                '    "result": "A Title"\n'
                '}\n\n'
                '{"result":{"taggedFile":{"fileName":"test.mp3",'
                '"fileNameChanged":false,"format":'
                '"MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels",'
                '"tag2":{"format":"ID3v2.3.0","frames":[{"changed":true,'
                '"name":"Title","value":"A Title"}]}}}}\n'
                '{"result":null}\n'
                '{"result":{"files":[{"changed":true,"fileName":"test.mp3",'
                '"selected":false,"tags":[2]}]}}\n'
                '{"result":null}\n'
                '{"result":null}\n'
                '{"result":null}\n'
                '{"result":null}\n'
                '{"result":null}\n'
                '{"id":"1","jsonrpc":"2.0","result":null}\n'
                '{"result":{"files":[{"changed":true,"fileName":"test.mp3",'
                '"selected":true,"tags":[2]}]}}\n')


if __name__ == '__main__':
    unittest.main()
