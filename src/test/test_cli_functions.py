#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import unittest
import sys
import os
import subprocess
import tempfile
import platform
import json
from kid3testsupport import kid3_cli_path, call_kid3_cli, create_test_file


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
        self.assertRegex(full_help, r'^kid3-cli (\d\.\d\.\d|git\d{8}) \(c\) \d{4} Urs Fleisch')
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

    def test_cd_pwd(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            curdir = os.getcwd()
            lines = call_kid3_cli(
                ['-c', 'pwd',
                 '-c', 'cd "' + tmpdir + '"', '-c', 'pwd',
                 '-c', 'cd ', '-c', 'pwd',
                 '-c', 'cd "' + curdir + '"', '-c', 'pwd']).splitlines()
            self.assertTrue(os.path.samefile(lines[0], curdir))
            self.assertTrue(os.path.samefile(lines[1], tmpdir))
            self.assertTrue(os.path.samefile(lines[2], os.path.expanduser('~')))
            self.assertTrue(os.path.samefile(lines[3], curdir))

    def test_ls(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            self.assertEqual(call_kid3_cli(['-c', 'ls', tmpdir]), '')
            create_test_file(os.path.join(tmpdir, 'test.mp3'))
            self.assertEqual(call_kid3_cli(['-c', 'ls', tmpdir]), '  --- test.mp3\n')

    def test_id3v1(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            mp3path = os.path.join(tmpdir, 'test.mp3')
            create_test_file(mp3path)
            with open(mp3path, 'rb') as mp3fh:
                empty_mp3_bytes = mp3fh.read()
            self.assertEqual(call_kid3_cli(
                ['-c', 'get title 1',
                 '-c', 'set title "A Title" 1',
                 '-c', 'get title 1',
                 '-c', 'get all 1',
                 '-c', 'save', mp3path]),
                'A Title\n'
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test.mp3\n'
                'Tag 1: ID3v1.1\n'
                '* Title  A Title\n')
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

    def test_id3v2(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            mp3path = os.path.join(tmpdir, 'test.mp3')
            jpgpath = os.path.join(tmpdir, 'test.jpg')
            picpath = os.path.join(tmpdir, 'folder.jpg')
            create_test_file(mp3path)
            create_test_file(jpgpath)
            with open(mp3path, 'rb') as mp3fh:
                empty_mp3_bytes = mp3fh.read()
            with open(jpgpath, 'rb') as jpgfh:
                jpg_bytes = jpgfh.read()
            self.assertEqual(call_kid3_cli(
                ['-c', 'get title 2',
                 '-c', 'get all 2', mp3path]),
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test.mp3\n')
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
                 '-c', 'to24',
                 '-c', 'get all 2',
                 '-c', 'to23',
                 '-c', 'get picture:"%s" "A Description" 2' % picpath,
                 '-c', 'get all 2',
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
                'File: MPEG 1 Layer 3 64 kbps 44100 Hz 1 Channels\n'
                '  Name: test.mp3\n'
                'Tag 2: ID3v2.4.0\n'
                '  Artist                  An Artist\n'
                '  Album                   An Album\n'
                '  Comment                 A Comment\n'
                '  Date                    2016\n'
                '  Track Number            3\n'
                '  Genre                   Power Metal\n'
                '  Lyricist                A Lyricist\n'
                '  Picture: Cover (front)  A Description\n'
                '  Playlist Delay          100\n'
                'A Description\n'
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
                '  Playlist Delay          100\n')
            with open(mp3path, 'rb') as mp3fh:
                ba = mp3fh.read()
                self.assertEqual(ba, empty_mp3_bytes)
            with open(picpath, 'rb') as jpgfh:
                ba = jpgfh.read()
                self.assertEqual(ba, jpg_bytes)
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

    def test_flac(self):
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
            if sys.platform == 'win32':
                expected = expected.replace('≠', '?')
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
            extensions = ('m4a', 'flac', 'spx', 'mp3', 'ape', 'wav', 'opus', 'aif')
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
                '  Track Number  2\n'
                '  Genre         Metal\n)?'                
                'Tag 2: Vorbis\n'
                '  Title         Kings Of Metal\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  2\n'
                '  Genre         Metal\n'
                'File: Speex 1 44100 Hz 1 Channels\n'
                '  Name: track02\\.spx\n'
                'Tag 2: Vorbis\n'
                '  Title         Heart Of Steel\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  3\n'
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
                '  Track Number  5\n'
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
                '  Track Number  6\n'
                '  Genre         Metal\n'
                'File: WAV (?:PCM 16 bit )?1411 kbps 44100 Hz 2 Channels\n'
                '  Name: track05\\.wav\n'
                'Tag 2: ID3v2\\.3\\.0\n'
                '  Title         Hail And Kill\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  8\n'
                '  Genre         Metal\n'
                'File: Opus 1 48000 Hz 1 Channels\n'
                '  Name: track06\\.opus\n'
                'Tag 2: Vorbis\n'
                '  Title         The Warriors Prayer\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  9\n'
                '  Genre         Metal\n'
                'File: AIFF [^\\n]+ kbps 44100 Hz 2 Channels\n'
                '  Name: track07\\.aif\n'
                'Tag 2: ID3v2\\.4\\.0\n'
                '  Title         Blood Of The Kings\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  10\n'
                '  Genre         Metal\n'
                'Tag 1:\n'
                '  Title         ≠\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  ≠\n'
                '  Genre         Metal\n'
                'Tag 2:\n'
                '  Title         ≠\n'
                '  Artist        Manowar\n'
                '  Album         Kings Of Metal\n'
                '  Date          1988\n'
                '  Track Number  ≠\n'
                '  Genre         Metal\n')
            if sys.platform == 'win32':
                expected = expected.replace('≠', '\\?')
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
            call_kid3_cli(['-c', 'fromtag "%{track} %{title}" 2', os.path.join(tmpdir, '*.*')])
            self.assertRegex(call_kid3_cli(
                ['-c', 'ls', tmpdir]),
                '  -2- 01 Wheels Of Fire.m4a\n'
                '  [-1]2- 02 Kings Of Metal.flac\n'
                '  -2- 03 Heart Of Steel.spx\n'
                '  12- 05 The Crown And The Ring \\(Lament Of The Kings\\).mp3\n'
                '  12- 06 Kingdom Come.ape\n'
                '  -2- 08 Hail And Kill.wav\n'
                '  -2- 09 The Warriors Prayer.opus\n'
                '  -2- 10 Blood Of The Kings.aif\n')
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
                 '- 06 Kingdom Come.ape', '+ 08 Hail And Kill.wav', '- 09 The Warriors Prayer.opus',
                 '- 10 Blood Of The Kings.aif', 'Finished'])
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
                '09 The Warriors Prayer.opus\n'
                '10 Blood Of The Kings.aif\n')

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
            if sys.platform == 'win32' and platform.release() == 'XP':
                expected = expected.replace('ö', '?')
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
            if sys.platform == 'win32' and platform.release() == 'XP':
                expected = expected.replace('ö', '?')
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

    def test_json_methods(self):
        self.maxDiff=None
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
