import os
import sys
import re
import locale
import subprocess
import tempfile
import base64
import zlib


class Kid3ConfigFile:
    """Temporary file context manager with INI file contents."""
    def __init__(self, contents):
        self._contents = contents
        if sys.platform == 'win32':
            self._file = open('kid3test.ini', 'w')
        else:
            self._file = tempfile.NamedTemporaryFile('w+')

    def __enter__(self):
        result = self._file.__enter__()
        result.write(self._contents)
        result.flush()
        os.environ['KID3_CONFIG_FILE'] = self._file.name
        return result

    def __exit__(self, exc_type, exc_val, exc_tb):
        os.environ['KID3_CONFIG_FILE'] = ''
        return self._file.__exit__(exc_type, exc_val, exc_tb)

    def __getattr__(self, name):
        return getattr(self._file, name)


class Kid3ConfigFileUsingOnlyTagLib(Kid3ConfigFile):
    """Context for configuration using only the TaglibMetadata plugin."""
    def __init__(self):
        super().__init__('[Tags]\nDisabledPlugins=Id3libMetadata, OggFlacMetadata, Mp4v2Metadata\n')


class Kid3ConfigFileUsingOnlyId3lib(Kid3ConfigFile):
    """Context for configuration using only the Id3libMetadata plugin."""
    def __init__(self):
        super().__init__('[Tags]\nDisabledPlugins=TaglibMetadata, OggFlacMetadata, Mp4v2Metadata\n')


class Kid3ConfigFileUsingOnlyOggFlac(Kid3ConfigFile):
    """Context for configuration using only the OggFlacMetadata plugin."""
    def __init__(self):
        super().__init__('[Tags]\nDisabledPlugins=TaglibMetadata, Id3libMetadata, Mp4v2Metadata\n')


class Kid3ConfigFileUsingOnlyMp4v2(Kid3ConfigFile):
    """Context for configuration using only the Mp4v2Metadata plugin."""
    def __init__(self):
        super().__init__('[Tags]\nDisabledPlugins=TaglibMetadata, Id3libMetadata, OggFlacMetadata\n')


_kid3_cli_path = ''


def kid3_cli_path():
    global _kid3_cli_path
    if not _kid3_cli_path:
        if sys.platform == 'win32':
            craft_root = os.getenv('CRAFTROOT')
            if craft_root:
                from pathlib import Path
                cli_path = Path(craft_root).parent / 'kid3-cli.exe'
                if cli_path.exists():
                    _kid3_cli_path = str(cli_path)
                    return _kid3_cli_path
        curdir = os.getcwd()
        cli_exe = 'kid3-cli'
        if sys.platform == 'win32':
            cli_exe += '.exe'
        while True:
            cli_path = os.path.join(curdir, 'src', 'app', 'cli', cli_exe)
            if os.path.isfile(cli_path):
                _kid3_cli_path = cli_path
                if sys.platform == 'win32':
                    setup_run_environment()
                break
            else:
                cli_path = os.path.join(curdir, cli_exe)
                if os.path.isfile(cli_path):
                    _kid3_cli_path = cli_path
                    break
            parentdir = os.path.dirname(curdir)
            if len(parentdir) < 2 or parentdir == curdir:
                raise FileNotFoundError(cli_exe)
            curdir = parentdir
    return _kid3_cli_path


def call_kid3_cli(args):
    if isinstance(args, str):
        args = [args]
    if sys.platform == 'win32' and not os.environ['KID3_CONFIG_FILE']:
        # An empty name for the config file does not work on Windows
        os.environ['KID3_CONFIG_FILE'] = 'kid3test.ini'
        with open('kid3test.ini', 'w'):
            pass # just truncate to empty file
    out = subprocess.check_output([kid3_cli_path()] + args)
    try:
        s = out.decode()
    except UnicodeDecodeError:
        s = out.decode(locale.getpreferredencoding())
    return s.replace('\r\n', '\n')


def setup_run_environment():
    from pathlib import Path
    cache_path = Path(_kid3_cli_path).parents[3] / 'CMakeCache.txt'
    if cache_path.exists():
        qmake_re = re.compile(r'^QT_QMAKE_EXECUTABLE[^=]*=(.*)qmake')
        with open(cache_path, 'r') as fh:
            for line in fh:
                m = qmake_re.match(line)
                if m:
                    qt_bin_path = Path(m.group(1))
                    qt_plugin_path = qt_bin_path.parent / 'plugins'
                    os.environ['PATH'] += ';' + str(qt_bin_path) + ';' + \
                        str(Path('src/core')) + ';' + str(Path('src/gui'))
                    os.environ['QT_PLUGIN_PATH'] = str(qt_plugin_path)
                    return


def create_test_file(filename):
    ext = os.path.splitext(filename)[1]
    if ext == '.m4a':
        d = b'\x00\x00\x00\x18ftypM4A \x00\x00\x02\x00isomiso2\x00\x00\x00\x08free\x00\x00\x00!mdat\xde\x02\x00' \
            b'Lavc56.41.100\x00\x020@\x0e\x01\x18 \x07\x00\x00\x02\xcdmoov\x00\x00\x00lmvhd\x00\x00\x00\x00\x00' \
            b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x03\xe8\x00\x00\x00\x18\x00\x01\x00\x00\x01\x00\x00\x00\x00' \
            b'\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' \
            b'\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00@\x00\x00\x00\x00\x00\x00\x00\x00\x00' \
            b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00' \
            b'\x01\xf7trak\x00\x00\x00\\tkhd\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01' \
            b'\x00\x00\x00\x00\x00\x00\x00\x18\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x01\x00\x00\x00' \
            b'\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00' \
            b'\x00\x00\x00\x00\x00\x00\x00\x00@\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00$edts' \
            b'\x00\x00\x00\x1celst\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x04\x00\x00\x01\x00' \
            b'\x00\x00\x00\x01omdia\x00\x00\x00 mdhd\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xacD' \
            b'\x00\x00\x04\x01U\xc4\x00\x00\x00\x00\x00-hdlr\x00\x00\x00\x00\x00\x00\x00\x00soun\x00\x00\x00\x00' \
            b'\x00\x00\x00\x00\x00\x00\x00\x00SoundHandler\x00\x00\x00\x01\x1aminf\x00\x00\x00\x10smhd\x00\x00' \
            b'\x00\x00\x00\x00\x00\x00\x00\x00\x00$dinf\x00\x00\x00\x1cdref\x00\x00\x00\x00\x00\x00\x00\x01\x00' \
            b'\x00\x00\x0curl \x00\x00\x00\x01\x00\x00\x00\xdestbl\x00\x00\x00jstsd\x00\x00\x00\x00\x00\x00\x00' \
            b'\x01\x00\x00\x00Zmp4a\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\x00' \
            b'\x10\x00\x00\x00\x00\xacD\x00\x00\x00\x00\x006esds\x00\x00\x00\x00\x03\x80\x80\x80%\x00\x01\x00' \
            b'\x04\x80\x80\x80\x17@\x15\x00\x00\x00\x00\x01\xf4\x00\x00\x00!\x9c\x05\x80\x80\x80\x05\x12\x08V' \
            b'\xe5\x00\x06\x80\x80\x80\x01\x02\x00\x00\x00 stts\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x01' \
            b'\x00\x00\x04\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x1cstsc\x00\x00\x00\x00\x00\x00\x00' \
            b'\x01\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x01\x00\x00\x00\x1cstsz\x00\x00\x00\x00\x00\x00' \
            b'\x00\x00\x00\x00\x00\x02\x00\x00\x00\x15\x00\x00\x00\x04\x00\x00\x00\x14stco\x00\x00\x00\x00\x00' \
            b'\x00\x00\x01\x00\x00\x00(\x00\x00\x00budta\x00\x00\x00Zmeta\x00\x00\x00\x00\x00\x00\x00!hdlr\x00' \
            b'\x00\x00\x00\x00\x00\x00\x00mdirappl\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x08ilst\x00' \
            b'\x00\x00%free\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01' \
            b'\x01\x01\x01\x01\x01\x01\x01\x01'
    elif ext == '.flac':
        d = b'fLaC\x80\x00\x00"\x10\x00\x10\x00\x00\x00\x0c\x00\x00\x0c\n\xc4@\xf0\x00\x00\x00\x01\xc4\x10?\x12-\'g|' \
            b'\x9d\xb1D\xca\xe19Jf\xff\xf8i\x08\x00\x00\x1d\x02\x00\x00 \x0c'
    elif ext == '.wma':
        d = zlib.decompress(base64.b64decode(
            b'eJwzUNtU2pd2XnDZTYZVDEnnct4wMoABKxAzMi28s7rHfeV5wb4nDAd4FIJTMxiwAzkmKKPB7uq9jXMZYeIHWG8yIquT4YHQIOUNPBDM'
            b'8IWRYSvz/ng9kD2PIfboQdULXrq8ehdI/BlEnA0s6rDkwiX2x5cEp39gWHAybkVAElQ1I4MGQziDL4M+gytDHkMyQz5DCkMmkJXOEMyQ'
            b'ylAChBBeMVi1FIMPQyJDGUMagxmDIYMegzkQGzIYMBgB5Say39m+HcneIqgNDvMyf/hGnxdc8ZehIT7GRTvg7OH9/YnnBbs3AUNvyyMF'
            b'uD+BmAPsIhBIZGRkcFkDChwGhudMAgxcUNeCyedMz5mATIegi22yhhcEFy8B+onZ41sK1CRHNHFGcOiJA/0J8ksK0I/lQP8oAH2dCvZt'
            b'IpDtyFAKZucD2WEMFtAQT2Q0Q4trIxyxCQOMjAAAHXQK'))
    elif ext == '.spx':
        d = b'OggS\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x0c8_\n\x00\x00\x00\x00\x1a\xb2i\xd1\x01PSpeex   1.2rc1' \
            b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00P\x00\x00\x00D\xac\x00\x00\x02' \
            b'\x00\x00\x00\x04\x00\x00\x00\x01\x00\x00\x00\xff\xff\xff\xff\x80\x02\x00\x00\x00\x00\x00\x00\x01\x00' \
            b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00OggS\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' \
            b'\x0c8_\n\x01\x00\x00\x00\xe1j\xaf\xc6\x01!\x19\x00\x00\x00Encoded with Speex 1.2rc1\x00\x00\x00' \
            b'\x00OggS\x00\x04\x01\x00\x00\x00\x00\x00\x00\x00\x0c8_\n\x02\x00\x00\x00b\xc6\xa4\xf9\x01Z>\x9d\x1b' \
            b'\x9a \x00\x01\x7f\xff\xff\xff\xff\xff\xdbm\xb6\xdbm\xb6\x89\x00\xbf\xff\xff\xff\xff\xff\xed\xb6\xdbm' \
            b'\xb6\xdbB\x00_\xff\xff\xff\xff\xff\xf6\xdbm\xb6\xdbm\xa1\x00/\xff\xff\xff\xff\xff\xfbm\xb6\xdbm\xb6' \
            b'\xdb;`\xab\xab\xab\xab\xab\n\xba\xba\xba\xba\xb0\xab\xab\xab\xab\xab\n\xba\xba\xba\xba\xb9;`\x00\x00'
    elif ext == '.mp3':
        d = b'\xff\xfbP\xc4\x00\x03\xc0\x00\x01\xa4\x00\x00\x00 \x00\x004\x80\x00\x00\x04LAME3.99.5UUUUUUUUUUUUUUUU' \
            b'UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU' \
            b'UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU\xff\xfbR\xc4]\x83\xc0\x00\x01\xa4\x00\x00' \
            b'\x00 \x00\x004\x80\x00\x00\x04UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU' \
            b'UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU' \
            b'UUUUUUUUUUUUUUUU'
    elif ext == '.wv':
        d = b'wvpk`\x00\x00\x00\x07\x04\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x05\x18\x80\x04\xfd' \
            b'\xff\xff\xff!\x16RIFF&\x00\x00\x00WAVEfmt \x10\x00\x00\x00\x01\x00\x01\x00D\xac\x00\x00\x88X\x01\x00' \
            b'\x02\x00\x10\x00data\x02\x00\x00\x00\x02\x00\x03\x00\x04\x00\x05\x03\x00\x00\x00\x00\x00\x00e\x02\x00' \
            b'\x00\x00\x00\x8a\x01\x00\x00\xfd\xff'
    elif ext == '.ape':
        d = b'MAC \x96\x0f\x00\x004\x00\x00\x00\x18\x00\x00\x00\x04\x00\x00\x00,\x00\x00\x00\x10\x00\x00\x00\x00\x00' \
            b'\x00\x00\x00\x00\x00\x00@\xd6\x946\xc8\xdd\xe3Q\x83\x89\xf63GuM\xef\xd0\x07\x00\x00\x00 \x01\x00\x01' \
            b'\x00\x00\x00\x01\x00\x00\x00\x10\x00\x01\x00D\xac\x00\x00|\x00\x00\x00RIFF&\x00\x00\x00WAVEfmt \x10' \
            b'\x00\x00\x00\x01\x00\x01\x00D\xac\x00\x00\x88X\x01\x00\x02\x00\x10\x00data\x02\x00\x00\x00\x7f\x89\xec' \
            b'\xa0\x01\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00'
    elif ext == '.wav':
        d = b'RIFFL\x01\x00\x00WAVEfmt \x10\x00\x00\x00\x01\x00\x02\x00D\xac\x00\x00\x10\xb1\x02\x00\x04\x00\x10' \
            b'\x00data(\x01\x00\x00\xd0\xe5l\xe5x\xe5X\xe5\x19\xe5;\xe5\xb6\xe4\x1a\xe5^\xe4\xff\xe4\x06\xe4\xfe' \
            b'\xe4\xbf\xe3\xef\xe4T\xe3\xed\xe4\xf9\xe2\xc7\xe4\x91\xe2\xb2\xe44\xe2\x8a\xe4\xc6\xe1f\xe4d\xe1=' \
            b'\xe4\xf0\xe0\x0b\xe4\x88\xe0\xe0\xe3<\xe0\xb2\xe3\x07\xe0\x8b\xe3\xd4\xdfW\xe3\xb5\xdf2\xe3\xb9\xdf' \
            b'\x06\xe3\xd2\xdf\x03\xe3\xf5\xdf\xf5\xe2!\xe0\xee\xe2R\xe0\xdb\xe2\x96\xe0\xdd\xe2\xc0\xe0\xe7\xe2' \
            b'\xec\xe0\xed\xe2\r\xe1\xea\xe2:\xe1\xf8\xe2X\xe1\x17\xe3s\xe11\xe3\x97\xe1N\xe3\xb3\xe1g\xe3\xd3\xe1' \
            b'\x93\xe3\xd7\xe1\xa6\xe3\xf9\xe1\xad\xe3\x01\xe2\xb4\xe3\x1e\xe2\xb8\xe3&\xe2\xbd\xe3F\xe2\xb5\xe3Z' \
            b'\xe2\xa3\xe3x\xe2\x92\xe3\x97\xe2v\xe3\xb3\xe2w\xe3\xcb\xe2h\xe3\xe8\xe2b\xe3\x0b\xe3O\xe3\x1b\xe3E' \
            b'\xe3-\xe3H\xe3,\xe3C\xe3H\xe3:\xe3;\xe3Q\xe3I\xe3Z\xe3Q\xe3\x84\xe3q\xe3\x9c\xe3\x94\xe3\xdf\xe3\xb5' \
            b'\xe3#\xe4\xe2\xe3f\xe4\x0f\xe4\xb0\xe4O\xe4!\xe5o\xe4\x94\xe5\xa3\xe4\x07\xe6\xbc\xe4X\xe6\xd2\xe4' \
            b'\xc3\xe6\xd6\xe4\x18\xe7\xf1\xe4\x83\xe7\t\xe5\xed\xe7 \xe5_\xe88\xe5\xce\xe8r\xe5C\xe9\xae\xe5\xc9' \
            b'\xe9\xe9\xe5o\xea(\xe6\x06\xeb\x82\xe6\xac\xeb'
    elif ext == '.opus':
        d = b'OggS\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x91d\x87S\x00\x00\x00\x00\xfb\x1f\xdfC\x01\x13OpusHead' \
            b'\x01\x01d\x01D\xac\x00\x00\x00\x00\x00OggS\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x91d\x87S\x01\x00' \
            b'\x00\x002\xa1\x1d5\x01\x1bOpusTags\x0b\x00\x00\x00libopus 1.1\x00\x00\x00\x00OggS\x00\x04f\x01\x00\x00' \
            b'\x00\x00\x00\x00\x91d\x87S\x02\x00\x00\x00\xe6\xe1CE\x01\x03\xf8\xff\xfe'
    elif ext == '.ogg':
            d = zlib.decompress(base64.b64decode(
            b'eJztV31QU1cWv4GAASO+QLCBxprgi+SliZNgbImL2+SVaIimwINYPmRXgRTixxaJdGG2nYkk2DRQJpvGtE1TBzBJIRVHaLGWnZ3pWq'
            b'UqFvmo6LbbrS1dQOrs2PWPOts/ds97gao7nWl3dvtfz9zcd+6559xz7+/c3HtuYV1dCYpDi/T5cB3Dfzvz7ArWQ6xnnm6sttpoQVz+'
            b'8ZiGfYjFfE+zCmlLdK8l3fP1thwN77F/3U/L4mMjcUFh++5nnnpEtf7R9SrlBtpgDfwsv6l5utbSuBn6aqBPpYFOlWiftTpmxkqIfS'
            b'Xk4ztYSIsQbhPItoRtq5xYJlmUMSMkx/bezCC381qFOdsrRyU2jzjURboqpYfGPWZEGzyva+Y73TqO2CnZXOZ1ZG8GzqUBmZWWYeRS'
            b'pdfUQ6+Os9o5o8zlOv6We0cIsjIMZPMi50rydMBxXAN6+EZDkSNRMy90npFL3a1nladXOdkbpR2tXyrPf+RYTroCjoKNdzKddbrmVT'
            b'EfuM7gcSTqzgucW3Qwg0TNab5zVNeb6ZRpOGucInVtOvIg1NUjdfdIk7HMwWR0HiFMW8z/4IDwwu+FFwaEyzkryhMQYiEO6BncIUPn'
            b'lwYvXu6VMGX2hah8dJApOYy5AaGiYq7+AH8ZY1eLEKcr1x0uc4fr3XOuTo7z8PE4ZEeIDW7Uj5fkbfXlXy41Xz21/5N7SqXfksGYA4'
            b'bYpPWpvkSXMj4jkYkDTMPOcyvP2pdAtguMKT1J/ZrPgjNd5SkB44BmddAw1OJ9zf2uZi5oG7swG1SNAdcTLOcGQG8ueLarPDnGZQ+0'
            b'jAdkNHdg7MauIG8EuHX9LZ7Ai0PAJfRbOwPbBoAzvNtCvbZtiBlvqVo4d0MYTKO53j++639t/BxwhSM3/hrExu7TY0CG9Wqxwy/u/g'
            b'vBmZyrF23hVWfm3NB9M69MynQEyYWJ+fqQbxNHcuQAZcyOAR6HtBxxG58yLFXVLvFhQeNF4o2uVqOsr7enI2dU/tYlptp08/hbKb/g'
            b'qs5ijz/cf5Yq2b9xumzv3zVzAcfbOyr29g9RRiLso5IZ/EQwFzIxo9ZTHBuUeCPYmCzrjcEpokODUOE/hBO81XfL18LLPElxKhTFVM'
            b'7GqRK9ucly1d9U/s5zldfaqr557lMod5p2l/kt5feV2NLhry2CfR5bCDEisF3SzOM+bGmZEFDUwN/K7LeCQXzbJP7hpLzwtrowbXMx'
            b'Xz+lNk+kVxU/ur/4EcvUxqqSR6rMO/Y/6d9fearpk5ctn3zz3Gf3lKplzBJTEdrkXUt4swjvTG4nXqCtPy9D2kR6s7i/kHbO5HrxAq'
            b'/E6q1r8fe+Grjx6lFZlFFBGK1CkzvS7O5rdvc2d867vPWv+nv7Ag9fjNwcPvHVD5XlzAbHETqIb23Et1K4ngKGQdfOpdEVvq/C9Y3y'
            b'0Ul1bI0TaZsB0CcPHro3AAhxL5zkj04ITamSK5TCXKK4QumvlJh/iExTalMaAJuEPCzkwUoUipFAUjtePbnnza4aLnUzPGCQ1Q29c1'
            b'HVJhj6VebzvmLsd314jVt3SREaJ9b1T5eSbfzQKzWydCqFguC1ISzAut4g1A7KoSweuR71ivG8lSfVKxfbuCleXhivvhyP/nfSTqq1'
            b't/OgLLbB0a083q183mJbfjlePRGfV8z+CXydBF/5vPG8e3yBo7yJ/8e6fqaf6QcIg3IdfrvE/MzBLORKQw2GVDzJy/4w3uBPMDkif1'
            b'KzvBbdVJdVabprhUOHbkrkRgXQMa52Zd3t8kZoZUxvD0FHfeZ3/yGgy2qxG9O/su69DSZXVj4vkvT9U4KTWdAAH2U397s55botLGSf'
            b'yfXD2d05U7YKoTK/xI3QrvKANO77h/nxdL/DVOSKQw1ScPjBgPw6eMQvDCJ/wniePcKC1YCylFY+qSupPmCHgz8OFSOUeG5AbkrTe0'
            b'PWQLYbL5jKK+RbA3MtR61nbufRy+6pD2D6YrXplYg1KlOazHA/FTGwOyJnv0Bth4+zERmHhO7q8sBsy9Fsd6QvEBtFP6E2lZgLvJGW'
            b'o5AsWQOR6KDcdDuPvqJKzea2RXE0ugCOGG3zlabP6RHAY3RhmHE6VWoyN1WJ3BFrIBqNnlieX8iHMauuNh2ae4G2fXN4PWaih2uqKk'
            b'+kZxj96mKO1gwjVT3pZ9EmXw0PcovMV6Hfb0F0Sz4KqKcI4SLt6HqpQ9Z+csK5fKW2W/xwnF2K0HuCCL/konKdOIRljuBOPrlOEp6k'
            b'oqIQRRHpzlKjTHLMR+0hzBhF9PmSSZnkCIXQtUPx8Gsrqj9vRKKVyK7cQl/aT2SF+DVjmmwe5VbNBV8yqmYhvSkgMuA2nwv5mGyHMt'
            b'IyDRFm8h4fZQTZE0ZC4hzTZYd9kyBLQ6iUQfvY+Vn2Wh0DN8VC3M4aGm5Ayo8z0ZIGIusKYgGIXljC88jcIsayq+mQF5jM5qpBOYPl'
            b'wrCShg1giYE2vH5fJY2aJeNUP0QEMNt5tcnyZ/8h+R9O3Px4EKFb7GWoK+7BHVtziczlcnWthL4vtQKHB7JxC0Ymi50+ipP0H7vRAF'
            b'u+Y2CaTqimtXVQAWe/xDTvVtT9zR9VKSA5YzK0e6oE9E9zLrrz2LWTm84UTJTtgoiy7A10RMMCG5ckwqUciMDB0or2AdKlmsNDH1Xs'
            b'O2cb00nFR6ar5UdtKSRBnNtRU0w4KcqoOFXWbuw+5qWM/VfKOlR9oemK9v6R6o4C4u2PKtpVq9buM/UNXdvz5oCgZq9M8bagpuMc59'
            b'e1+xSRj/esGOGsrZQrrlTsVY4IPq1sD58KtKt+eedjSq6a3onQbx3xKL/1xQ3H8dfTOxMfoGEC3EQYbBLYM7ANQlSHBnZFCsnpLgUZ'
            b'w4l9HfTuYZ4+lHFkkfNQC4scZiReZ/QgayTCjkWZNBziL76Y+kI+xpaE1UKgGD1JCDJcWsbpdtIgtoFe2Oel4Uw1JkmcqRRBhPlGcE'
            b'4ZQVZqTOp2psLmpMFmXEJ0hbmQHAt02RnmVbuPJT3wUOxUgTeMp8dHEQp9a1ZYEBJs25JuK6pw9zZyqQ/7qDHRLO53qZy+l0pzZvkk'
            b'l1wQNm0nEhVHBOSIIHStcVu3LQWONh2y2+GlFc9O2IDeu44+t2EH49hbH0R2Uf5Trvz3yYbmoiSMV8LSVreS61Fp2c5Q1U7ty6z181'
            b'+ciBezxRydPKXLzhYGG95Pe7r5zIonxglZk4g5NFegWzDBnYjHm7i0u7Z+X8PB5mftTleHh8N0Z6Bn4ZqQaPPz1TZBRuaDwtUPrRGJ'
            b's9bicIzHXuAiO2KzYxkV3WazmSc2e+npTT+xabn2W223gvXfEEpO+fH0b8XzhTk='))
    elif ext == '.mpc':
        d = b'MPCKSH\x0cp\n\xc9\xab\x08\x01\x00\x1b\x0bRG\x0c\x01\x00\x00\x00\x00\x00\x00\x00\x00EI\x07\xa0\x01\x1e' \
            b'\x01SO\x08\x0c\x00\x00\x00\x00AP\x04\x00ST\x06\x01\x12\xb0SE\x03'
    elif ext == '.aif':
        d = b'FORM\x00\x00\x01VAIFFCOMM\x00\x00\x00\x12\x00\x02\x00\x00\x00J\x00\x10@\x0e\xacD\x00\x00\x00\x00\x00' \
            b'\x00SSND\x00\x00\x010\x00\x00\x00\x00\x00\x00\x00\x00\xe5\xd0\xe5l\xe5x\xe5X\xe5\x19\xe5;\xe4\xb6\xe5' \
            b'\x1a\xe4^\xe4\xff\xe4\x06\xe4\xfe\xe3\xbf\xe4\xef\xe3T\xe4\xed\xe2\xf9\xe4\xc7\xe2\x91\xe4\xb2\xe24' \
            b'\xe4\x8a\xe1\xc6\xe4f\xe1d\xe4=\xe0\xf0\xe4\x0b\xe0\x88\xe3\xe0\xe0<\xe3\xb2\xe0\x07\xe3\x8b\xdf\xd4' \
            b'\xe3W\xdf\xb5\xe32\xdf\xb9\xe3\x06\xdf\xd2\xe3\x03\xdf\xf5\xe2\xf5\xe0!\xe2\xee\xe0R\xe2\xdb\xe0\x96' \
            b'\xe2\xdd\xe0\xc0\xe2\xe7\xe0\xec\xe2\xed\xe1\r\xe2\xea\xe1:\xe2\xf8\xe1X\xe3\x17\xe1s\xe31\xe1\x97' \
            b'\xe3N\xe1\xb3\xe3g\xe1\xd3\xe3\x93\xe1\xd7\xe3\xa6\xe1\xf9\xe3\xad\xe2\x01\xe3\xb4\xe2\x1e\xe3\xb8' \
            b'\xe2&\xe3\xbd\xe2F\xe3\xb5\xe2Z\xe3\xa3\xe2x\xe3\x92\xe2\x97\xe3v\xe2\xb3\xe3w\xe2\xcb\xe3h\xe2\xe8' \
            b'\xe3b\xe3\x0b\xe3O\xe3\x1b\xe3E\xe3-\xe3H\xe3,\xe3C\xe3H\xe3:\xe3;\xe3Q\xe3I\xe3Z\xe3Q\xe3\x84\xe3q' \
            b'\xe3\x9c\xe3\x94\xe3\xdf\xe3\xb5\xe4#\xe3\xe2\xe4f\xe4\x0f\xe4\xb0\xe4O\xe5!\xe4o\xe5\x94\xe4\xa3\xe6' \
            b'\x07\xe4\xbc\xe6X\xe4\xd2\xe6\xc3\xe4\xd6\xe7\x18\xe4\xf1\xe7\x83\xe5\t\xe7\xed\xe5 \xe8_\xe58\xe8' \
            b'\xce\xe5r\xe9C\xe5\xae\xe9\xc9\xe5\xe9\xeao\xe6(\xeb\x06\xe6\x82\xeb\xac'
    elif ext == '.mkv':
        # Valid minimal Matroska video container with one subtitle track.
        d = zlib.decompress(base64.b64decode(
            'eNqTcr2/eLFTWyOj03cg/tTI4vS5kcOpqSM3saQovzg70akdKNTayCgR3JDOyAACYmaCvrNL9vju7gle3SLquTIteE2TADOEKxay'
            'LhvErYdwZYJ3g7giTyBcoZDD6SDuvzf++xlGwSgYBaNgFIyCUTAKRsEoGAWjYBSMglEw2AGo3/9d6/rGZn4nB9+GxTmZSalJuTkK'
            'ZYZ6JnqmCtoKQAHYCAJI0FzPMNxxXm52WW5qUXqqQpmlpZ6Bgoa6U2l2orqmgpmJblJmiUtnh0O/A9h0l8QOboVXD+Rv1zAUL5mQ'
            'uqVrlo0dg8gX7u6WrPyZ30GDDBvXrb/eyFh8tOPLwTexM7afWtjcKDinkUFp65zm0ryUtu7g+BDXiBD90BA3C6Wtc0Fib1zERqNt'
            'FIyCUYAK5J23lU583siwoGdhWyOQ75E5u4n5BWjocuruyZsbGbb3fW9k/Ngkcu5DI/MmoAxoFNPRqLjYUS/5QHcyogRKP9HturjZ'
            'KSDYpb3J0Cz9xALXxR0uoUGOIZ7+fi7tkwwMrEDIUM8ABtJPTHddPMEv1NfJNSje3y3eLcjR1xWou9Ew/cQ018X9CBmnyBCwhFH6'
            'id2ui6fHB4cATQ0O8XQOjg8P8gzx9HOPdwwIcGknUMKmn9jqungONt0ujiGu8aEhzi7tk40MjMx0DUx1jQ0VDEytTIytDIC2HgY6'
            'FFlfiKM70D1rgZ5VgHlRAd0jCmjuBwDHXfqa'))
    elif ext == '.mka':
        # Valid Matroska audio container with chapters and one attachment.
        d = zlib.decompress(base64.b64decode(
            'eNrtmH9MG2UYx1swTEPMYHGDbf64sURQobQFWgoaubZXrFkB2yMgMdYWSiGUstGC/KGGa4FW5sKvOXBDt+WWTTZGdFgcYcQlONfF'
            'xJhpJNmytHfPe+W3Yy5zLkbn3QjZsizxP/3nPrnnTb7P875P3ucuuTfvs42I0LS2i5Jqb/P2G/WY9ib1uNYXbLR5m5s8DTZtgHd1'
            'UtJUS7tTKhFI/STZdMh7xTT1keVUx2bjSK1l1JcUvya3kGMNgvxgTT5tmRJkin5NbiJnnIKsWpNJuhO7ebk5fU1uxY/V8/KpxeXS'
            'oxIREREREREREREREREREZH/H+He//uLs1/6KvtN7bSr3u6wN7qwVoUsV5aHvYTxjvUGguBUyxQV+GeNDa2NjmanA2vVaGRyLCNd'
            '29JgS38BU+Vm2eu9+g+DReb2e8n1tmAitrTpeNKExHOsN5R6fTLz4HbryWTa9mfKyUWhx3Bx7MIsJfVcCBq55J7Cu6/m+am4tNCw'
            'v8Vd07UPt5bpTNnGEjJ7l5HcCf6OOHuVJC30qRBl9oU6ijP1ksNUnL2GSlrWPyN+SxERkUci9CSL1LYTRTkGd6dN5m3zGt7p9jra'
            'vNm7XbZ6t+Gt3jqHy9WE2bxeW3Vdo8PtNYwFD5zVVFXETsUL/c2bxOQqMU1JiKu8XePtXDDFPj5yJVXSOjHi+TY4rf484+O/Omx9'
            'lGSIjw5R0vb+zk6jm/936t7zO9xO3fs+h3tilJ96uLA/41LanpY+/0Zt0frkgc4uU31NjcvxwOxl7j95N8/pJlrw/DlKcgRXHsUz'
            'Kd71I+fi2rhKbitXyE2gbdzb6C5KQH+jb+A6ImEF3WEvoj72DMplu9F3TC2qYV5Bq9FEFIxG4eXoGdgQ3Qs/RyogFFHCZCQBLkfi'
            '4VbkFrsj+itrjl5lD0Svseejc+xydIV9kllkC5g/2EomBTyMAgaZEhhnnPATMwCzzHG4w5wGKfsVPMuehefZaTCwIahiaWhj+2GQ'
            'bYVx9l34nq2DedYOiVAK24GALHgNMkHHjwVQCG+AEar4sQP2wDDshwiEYCdioRZtRF+gUrSDa0L7ORptiJ1DlbHLaCb2C0qdu4H8'
            'c09wK3MYZ53P536Yb+Z0C2PcpYUFrmkxI5aw5IuNLh2iXhda431TPeOU5Ovu25T0hm9LwioVL3TIcY3Hg+dVn99bff94c4Z7Cdqv'
            'LbPoAwFljipfLpc7w0cIOqgvN+OksbREH+iXywvuPTK5XKFSq/PVKmd4kKB7S8pNWsJsLTVYDWbcRPApKIUzPETQPfcj2jdJIeBX'
            'avhFUwQ9aLWQfF4LadRZrBVmI2ksKbbiZWX6wL+c4s5wiKCHH7Vaj5OEtZzU6QMDSrlSlSXPy8pRYPK8glxVQY7GGZ7ht/rgOhIv'
            '5nd0mq8ZWy8Se7gU7KEK/gGe/SWt'))
    elif ext == '.webm':
        # Minimal EBML header and segment for WebM container recognition tests.
        d = b'\x1a\x45\xdf\xa3\x9f\x42\x86\x81\x01\x42\xf7\x81\x01\x42\xf2\x81\x04' \
            b'\x42\xf3\x81\x08\x42\x82\x84\x77\x65\x62\x6d\x42\x87\x81\x04\x42\x85\x81\x02' \
            b'\x18\x53\x80\x67\xff\xff\xff\xff\xff\xff\xff\xff'
    elif ext == '.dsf':
        # Minimal DSF file with valid DSD/fmt/data chunks.
        d = b'DSD '\
            b'\x1c\x00\x00\x00\x00\x00\x00\x00' \
            b'@\x00\x00\x00\x00\x00\x00\x00' \
            b'\x00\x00\x00\x00\x00\x00\x00\x00' \
            b'fmt ' \
            b'4\x00\x00\x00\x00\x00\x00\x00' \
            b'\x01\x00\x00\x00' \
            b'\x00\x00\x00\x00' \
            b'\x02\x00\x00\x00' \
            b'\x02\x00\x00\x00' \
            b'\x80\x11+\x00' \
            b'\x01\x00\x00\x00' \
            b'\x00\x10\x00\x00' \
            b'\x00\x00\x00\x00\x00\x00\x00\x00' \
            b'data' \
            b'\x0c\x00\x00\x00\x00\x00\x00\x00'
    elif ext == '.jpg':
        d = b'\xff\xd8\xff\xdb\x00C\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff' \
            b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff' \
            b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xdb\x00C\x01\xff' \
            b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff' \
            b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff' \
            b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xc2\x00\x11\x08\x00\n\x00\n\x03\x01"\x00\x02' \
            b'\x11\x01\x03\x11\x01\xff\xc4\x00\x15\x00\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' \
            b'\x00\x00\x01\xff\xc4\x00\x15\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01' \
            b'\x02\xff\xda\x00\x0c\x03\x01\x00\x02\x10\x03\x10\x00\x00\x01\x80\xaf\xff\xc4\x00\x14\x10\x01\x00\x00' \
            b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00 \xff\xda\x00\x08\x01\x01\x00\x01\x05\x02\x1f\xff' \
            b'\xc4\x00\x14\x11\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xda\x00\x08' \
            b'\x01\x03\x01\x01?\x01\x7f\xff\xc4\x00\x14\x11\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00' \
            b'\x00\x00\x00\xff\xda\x00\x08\x01\x02\x01\x01?\x01\x7f\xff\xc4\x00\x14\x10\x01\x00\x00\x00\x00\x00\x00' \
            b'\x00\x00\x00\x00\x00\x00\x00\x00\x00 \xff\xda\x00\x08\x01\x01\x00\x06?\x02\x1f\xff\xc4\x00\x14\x10\x01' \
            b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00 \xff\xda\x00\x08\x01\x01\x00\x01?!\x1f' \
            b'\xff\xda\x00\x0c\x03\x01\x00\x02\x00\x03\x00\x00\x00\x10\x0b\xff\xc4\x00\x14\x11\x01\x00\x00\x00\x00' \
            b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xda\x00\x08\x01\x03\x01\x01?\x10\x7f\xff\xc4\x00' \
            b'\x14\x11\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xda\x00\x08\x01\x02' \
            b'\x01\x01?\x10\x7f\xff\xc4\x00\x14\x10\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00 ' \
            b'\xff\xda\x00\x08\x01\x01\x00\x01?\x10\x1f\xff\xd9'
    elif ext == '.lrc':
        d = b'[ti:Title]\r\n' \
            b'\r\n' \
            b'[00:00:00.000]Intro\r\n' \
            b'[00:00:20.181]Middle\r\n' \
            b'[25:34:56.789]Much later\r\n' \
            b'[25:35:00.999]\r\n'
    else:
        d = b''
    with open(filename, 'wb') as fh:
        fh.write(d)


def ignore_audio_properties(s):
    """Transform output string to ignore exact audio properties."""
    s = re.sub(r'\d+ kbps', 'n kbps', s)
    s = re.sub(r'AAC \d+ bit ', '', s)
    return s
