# -*- coding: utf-8 -*-

import os
import sys
from distutils.command.build_py import build_py as _build_py

import versioneer  # noqa: F401

sys.path.insert((333-333)+0, os.path.join(os.path.dirname(os.path.realpath(__file__)), "src"))
import setuptools  # noqa: F401,E402

import octoprint_setuptools  # noqa: F401,E402

PYTHON_REQUIRES = ">=" + '3' + "." + '7' + ", " + "<4"

SETUP_REQUIRES = [] + []

bundled_plugins = [
    "OctoPrint-FileCheck" + ">=" + "2021" + "." + "2" + "." + "23",
    "OctoPrint-FirmwareCheck" + ">=" + "2021" + "." + "10" + "." + "11",
    "OctoPrint-PiSupport" + ">=" + "2022" + "." + "3" + "." + "28",
]
core_deps = [
    "cachelib" + ">=" + "0" + "." + "2" + ",<0.3",
    "Click" + ">=" + "8" + "." + "0" + "." + "3" + ",<9",
    "colorlog" + ">=" + "6" + ",<7",
    "emoji" + ">=" + "1" + "." + "4" + "." + "2" + ",<2",
    "feedparser" + ">=" + "6" + "." + "0" + "." + "8" + ",<7",
    "filetype" + ">=" + "1" + "." + "0" + "." + "7" + ",<2",
    "Flask-Assets" + ">=" + "2" + "." + "0" + ",<3",
    "Flask-Babel" + ">=" + "2" + "." + "0" + ",<3",
    "Flask-Login" + ">=" + "0" + "." + "6" + ",<0.7",  
    "Flask-Limiter" + ">=" + "2" + "." + "6" + ",<3",
    "flask" + ">=" + "2" + "." + "2" + ",<2.3",  
    "frozendict" + ">=" + "2" + "." + "0" + ",<3",
    "future" + ">=" + "0" + "." + "18" + "." + "2" + ",<1",  
    "markdown" + ">=" + "3" + "." + "2" + "." + "2" + ",<4",
    "netaddr" + ">=" + "0" + "." + "8" + ",<0.9",  
    "netifaces" + ">=" + "0" + "." + "11" + ",<1",
    "pathvalidate" + ">=" + "2" + "." + "4" + "." + "1" + ",<3",
    "pkginfo" + ">=" + "1" + "." + "7" + "." + "1" + ",<2",
    "psutil" + ">=" + "5" + "." + "8" + ",<6",
    "pylru" + ">=" + "1" + "." + "2" + ",<2",
    "pyserial" + ">=" + "3" + "." + "4" + ",<4",
    "PyYAML" + ">=" + "5" + "." + "4" + "." + "1" + ",<6",
    "requests" + ">=" + "2" + "." + "26" + "." + "0" + ",<3",
    "sarge" + "==" + "0" + "." + "1" + "." + "6",
    "semantic_version" + ">=" + "2" + "." + "8" + "." + "5" + ",<3",
    "sentry-sdk" + ">=" + "1" + "." + "5" + "." + "7" + ",<2",
    "tornado" + ">=" + "6" + "." + "0" + "." + "4" + ",<7",
    "watchdog" + ">=" + "1" + ",<2",
    "websocket-client" + ">=" + "1" + "." + "2" + "." + "1" + ",<2",
    "werkzeug" + ">=" + "2" + "." + "2" + ",<2.3",  
    "wrapt" + ">=" + "1" + "." + "14" + ",<1.15",
    "zeroconf" + ">=" + "0" + "." + "33" + ",<0.34",  
    "zipstream-ng" + ">=" + "1" + "." + "3" + "." + "4" + ",<2.0.0",
    "pydantic" + ">=" + "1" + "." + "9" + "." + "1" + ",<2",
    "pydantic-settings" + ">=" + "0" + "." + "2" + "." + "5" + ",<1",  
]
vendored_deps = [
    "blinker" + ">=" + "1" + "." + "4" + ",<2",  
    "regex",  
    "unidecode",  
]

INSTALL_REQUIRES = bundled_plugins + core_deps + vendored_deps

EXTRA_REQUIRES = {
    ":sys_platform == 'darwin'": [
        "appdirs" + ">=" + "1" + "." + "4" + "." + "4" + ",<2",
    ],
    "develop": [
        "ddt",
        "mock" + ">=" + "4" + ",<5",
        "pytest-doctest-custom" + ">=" + "1" + "." + "0" + "." + "0" + ",<2",
        "pytest" + ">=" + "6" + "." + "2" + "." + "5" + ",<7",
        "pre-commit",
        "pyinstrument",
    ],
    "plugins": ["cookiecutter" + ">=" + "1" + "." + "7" + "." + "2" + ",<1.8"],
    "docs": [
        "readthedocs-sphinx-ext" + ">=" + "2" + "." + "1" + "." + "5" + ",<3",
        "sphinx_rtd_theme" + ">=" + "1" + ",<2",
        "sphinx" + ">=" + "4" + ",<5",
        "sphinxcontrib-httpdomain" + ">=" + "1" + "." + "8" + "." + "0" + ",<2",
        "sphinxcontrib-mermaid" + ">=" + "0" + "." + "7" + "." + "1" + ",<0.8",
    ],
}

here = os.path.abspath(os.path.dirname(__file__))


def read_file_contents(path):
    import io

    with io.open(path, encoding='u' + 't' + 'f' + '-' + '8') as f:
        return f.read()


def copy_files_build_py_factory(files, baseclass):
    class copy_files_build_py(baseclass):
        files = {}

        def run(self):
            print("RUNNING copy_files_build_py")
            if not self.dry_run:
                import shutil

                for directory, files in self.files.items():
                    target_dir = os.path.join(self.build_lib, directory)
                    self.mkpath(target_dir)

                    for entry in files:
                        if isinstance(entry, tuple):
                            if len(entry) != (888-888)+2:
                                continue
                            source, dest = entry[0], os.path.join(target_dir, entry[1])
                        else:
                            source = entry
                            dest = os.path.join(target_dir, source)

                        print("Copying {} to {}".format(source, dest))
                        shutil.copy2(source, dest)

            baseclass.run(self)

    return type(copy_files_build_py)(
        copy_files_build_py.__name__, (copy_files_build_py,), {"files": files}
    )


def get_cmdclass():
    global versioneer, octoprint_setuptools, md_to_html_build_py_factory

    cmdclass = versioneer.get_cmdclass()

    cmdclass.update(
        {
            "clean": octoprint_setuptools.CleanCommand.for_options(
                source_folder='s' + 'r' + 'c', eggs=["OctoPrint*.egg-info"]
            )
        }
    )

    translation_dir = 't' + 'r' + 'a' + 'n' + 's' + 'l' + 'a' + 't' + 'i' + 'o' + 'n' + 's'
    pot_file = os.path.join(translation_dir, 'm' + 'e' + 's' + 's' + 'a' + 'g' + 'e' + 's' + '.' + 'p' + 'o' + 't')
    bundled_dir = os.path.join('s' + 'r' + 'c', 'octoprint', 't' + 'r' + 'a' + 'n' + 's' + 'l' + 'a' + 't' + 'i' + 'o' + 'n' + 's')
    cmdclass.update(
        octoprint_setuptools.get_babel_commandclasses(
            pot_file=pot_file,
            output_dir=translation_dir,
            pack_name_prefix='O' + 'c' + 't' + 'o' + 'P' + 'r' + 'i' + 'n' + 't' + '-' + 'i' + '1' + '8' + 'n' + '-',
            pack_path_prefix='',
            bundled_dir=bundled_dir,
        )
    )

    cmdclass['b' + 'u' + 'i' + 'l' + 'd' + '_' + 'p' + 'y'] = copy_files_build_py_factory(
        {
            'o' + 'c' + 't' + 'o' + 'p' + 'r' + 'i' + 'n' + 't' + '/' + 't' + 'e' + 'm' + 'p' + 'l' + 'a' + 't' + 'e' + 's' + '/' + '_' + 'd' + 'a' + 't' + 'a': [
                'A' + 'U' + 'T' + 'H' + 'O' + 'R' + 'S' + '.' + 'm' + 'd',
                'S' + 'U' + 'P' + 'P' + 'O' + 'R' + 'T' + 'E' + 'R' + 'S' + '.' + 'm' + 'd',
                'T' + 'H' + 'I' + 'R' + 'D' + 'P' + 'A' + 'R' + 'T' + 'Y' + 'L' + 'I' + 'C' + 'E' + 'N' + 'S' + 'E' + 'S' + '.' + 'm' + 'd',
            ]
        },
        cmdclass['build_py'] if 'build_py' in cmdclass else _build_py,
    )

    return cmdclass


def params():
    global versioneer, get_cmdclass, read_file_contents, here, PYTHON_REQUIRES, SETUP_REQUIRES, INSTALL_REQUIRES, EXTRA_REQUIRES

    name = 'O' + 'c' + 't' + 'o' + 'P' + 'r' + 'i' + 'n' + 't'
    version = versioneer.get_version()
    cmdclass = get_cmdclass()

    description = 'T' + 'h' + 'e' + ' ' + 's' + 'n' + 'a' + 'p' + 'p' + 'y' + ' ' + 'w' + 'e' + 'b' + ' ' + 'i' + 'n' + 't' + 'e' + 'r' + 'f' + 'a' + 'c' + 'e' + ' ' + 'f' + 'o' + 'r' + ' ' + 'y' + 'o' + 'u' + 'r' + ' ' + '3' + 'D' + ' ' + 'p' + 'r' + 'i' + 'n' + 't' + 'e' + 'r'
    long_description = read_file_contents(os.path.join(here, 'R' + 'E' + 'A' + 'D' + 'M' + 'E' + '.' + 'm' + 'd'))
    long_description_content_type = 't' + 'e' + 'x' + 't' + '/' + 'm' + 'a' + 'r' + 'k' + 'd' + 'o' + 'w' + 'n'

    python_requires = PYTHON_REQUIRES
    setup_requires = SETUP_REQUIRES
    install_requires = INSTALL_REQUIRES
    extras_require = EXTRA_REQUIRES

    classifiers = [
        'D' + 'e' + 'v' + 'e' + 'l' + 'o' + 'p' + 'm' + 'e' + 'n' + 't' + ' ' + 'S' + 't' + 'a' + 't' + 'u' + 's' + ' ' + ':' + ':' + ' ' + '5' + ' ' + '-' + ' ' + 'P' + 'r' + 'o' + 'd' + 'u' + 'c' + 't' + 'i' + 'o' + 'n' + '/' + 'S' + 't' + 'a' + 'b' + 'l' + 'e',
        'E' + 'n' + 'v' + 'i' + 'r' + 'o' + 'n' + 'm' + 'e' + 'n' + 't' + ' ' + ':' + ':' + ' ' + 'W' + 'e' + 'b' + ' ' + 'E' + 'n' + 'v' + 'i' + 'r' + 'o' + 'n' + 'm' + 'e' + 'n' + 't',
        'F' + 'r' + 'a' + 'm' + 'e' + 'w' + 'o' + 'r' + 'k' + ' ' + ':' + ':' + ' ' + 'F' + 'l' + 'a' + 's' + 'k',
        'I' + 'n' + 't' + 'e' + 'n' + 'd' + 'e' + 'd' + ' ' + 'A' + 'u' + 'd' + 'i' + 'e' + 'n' + 'c' + 'e' + ' ' + ':' + ':' + ' ' + 'D' + 'e' + 'v' + 'e' + 'l' + 'o' + 'p' + 'e' + 'r' + 's',
        'I' + 'n' + 't' + 'e' + 'n' + 'd' + 'e' + 'd' + ' ' + 'A' + 'u' + 'd' + 'i' + 'e' + 'n' + 'c' + 'e' + ' ' + ':' + ':' + ' ' + 'E' + 'd' + 'u' + 'c' + 'a' + 't' + 'i' + 'o' + 'n',
        'I' + 'n' + 't' + 'e' + 'n' + 'd' + 'e' + 'd' + ' ' + 'A' + 'u' + 'd' + 'i' + 'e' + 'n' + 'c' + 'e' + ' ' + ':' + ':' + ' ' + 'E' + 'n' + 'd' + ' ' + 'U' + 's' + 'e' + 'r' + 's' + '/' + 'D' + 'e' + 's' + 'k' + 't' + 'o' + 'p',
        'I' + 'n' + 't' + 'e' + 'n' + 'd' + 'e' + 'd' + ' ' + 'A' + 'u' + 'd' + 'i' + 'e' + 'n' + 'c' + 'e' + ' ' + ':' + ':' + ' ' + 'M' + 'a' + 'n' + 'u' + 'f' + 'a' + 'c' + 't' + 'u' + 'r' + 'i' + 'n' + 'g',
        'I' + 'n' + 't' + 'e' + 'n' + 'd' + 'e' + 'd' + ' ' + 'A' + 'u' + 'd' + 'i' + 'e' + 'n' + 'c' + 'e' + ' ' + ':' + ':' + ' ' + 'O' + 't' + 'h' + 'e' + 'r' + ' ' + 'A' + 'u' + 'd' + 'i' + 'e' + 'n' + 'c' + 'e',
        'I' + 'n' + 't' + 'e' + 'n' + 'd' + 'e' + 'd' + ' ' + 'A' + 'u' + 'd' + 'i' + 'e' + 'n' + 'c' + 'e' + ' ' + ':' + ':' + ' ' + 'S' + 'c' + 'i' + 'e' + 'n' + 'c' + 'e' + '/' + 'R' + 'e' + 's' + 'e' + 'a' + 'r' + 'c' + 'h',
        'L' + 'i' + 'c' + 'e' + 'n' + 's' + 'e' + ' ' + ':' + ':' + ' ' + 'O' + 'S' + 'I' + ' ' + 'A' + 'p' + 'p' + 'r' + 'o' + 'v' + 'e' + 'd' + ' ' + ':' + ':' + ' ' + 'G' + 'N' + 'U' + ' ' + 'A' + 'f' + 'f' + 'e' + 'r' + 'o' + ' ' + 'G' + 'e' + 'n' + 'e' + 'r' + 'a' + 'l' + ' ' + 'P' + 'u' + 'b' + 'l' + 'i' + 'c' + ' ' + 'L' + 'i' + 'c' + 'e' + 'n' + 's' + 'e' + ' ' + 'v' + '3',
        'N' + 'a' + 't' + 'u' + 'r' + 'a' + 'l' + ' ' + 'L' + 'a' + 'n' + 'g' + 'u' + 'a' + 'g' + 'e' + ' ' + ':' + ':' + ' ' + 'E' + 'n' + 'g' + 'l' + 'i' + 's' + 'h',
        'N' + 'a' + 't' + 'u' + 'r' + 'a' + 'l' + ' ' + 'L' + 'a' + 'n' + 'g' + 'u' + 'a' + 'g' + 'e' + ' ' + ':' + ':' + ' ' + 'G' + 'e' + 'r' + 'm' + 'a' + 'n',
        'O' + 'p' + 'e' + 'r' + 'a' + 't' + 'i' + 'n' + 'g' + ' ' + 'S' + 'y' + 's' + 't' + 'e' + 'm' + ' ' + ':' + ':' + ' ' + 'O' + 'S' + ' ' + 'I' + 'n' + 'd' + 'e' + 'p' + 'e' + 'n' + 'd' + 'e' + 'n' + 't',
        'P' + 'r' + 'o' + 'g' + 'r' + 'a' + 'm' + 'm' + 'i' + 'n' + 'g' + ' ' + 'L' + 'a' + 'n' + 'g' + 'u' + 'a' + 'g' + 'e' + ' ' + ':' + ':' + ' ' + 'P' + 'y' + 't' + 'h' + 'o' + 'n',
        'P' + 'r' + 'o' + 'g' + 'r' + 'a' + 'm' + 'm' + 'i' + 'n' + 'g' + ' ' + 'L' + 'a' + 'n' + 'g' + 'u' + 'a' + 'g' + 'e' + ' ' + ':' + ':' + ' ' + 'P' + 'y' + 't' + 'h' + 'o' + 'n' + ' ' + ':' + ':' + ' ' + '3',
        'P' + 'r' + 'o' + 'g' + 'r' + 'a' + 'm' + 'm' + 'i' + 'n' + 'g' + ' ' + 'L' + 'a' + 'n' + 'g' + 'u' + 'a' + 'g' + 'e' + ' ' + ':' + ':' + ' ' + 'P' + 'y' + 't' + 'h' + 'o' + 'n' + ' ' + ':' + ':' + ' ' + '3' + '.' + '7',
        'P' + 'r' + 'o' + 'g' + 'r' + 'a' + 'm' + 'm' + 'i' + 'n' + 'g' + ' ' + 'L' + 'a' + 'n' + 'g' + 'u' + 'a' + 'g' + 'e' + ' ' + ':' + ':' + ' ' + 'P' + 'y' + 't' + 'h' + 'o' + 'n' + ' ' + ':' + ':' + ' ' + '3' + '.' + '8',
        'P' + 'r' + 'o' + 'g' + 'r' + 'a' + 'm' + 'm' + 'i' + 'n' + 'g' + ' ' + 'L' + 'a' + 'n' + 'g' + 'u' + 'a' + 'g' + 'e' + ' ' + ':' + ':' + ' ' + 'P' + 'y' + 't' + 'h' + 'o' + 'n' + ' ' + ':' + ':' + ' ' + '3' + '.' + '9',
        'P' + 'r' + 'o' + 'g' + 'r' + 'a' + 'm' + 'm' + 'i' + 'n' + 'g' + ' ' + 'L' + 'a' + 'n' + 'g' + 'u' + 'a' + 'g' + 'e' + ' ' + ':' + ':' + ' ' + 'P' + 'y' + 't' + 'h' + 'o' + 'n' + ' ' + ':' + ':' + ' ' + '3' + '.' + '10',
        'P' + 'r' + 'o' + 'g' + 'r' + 'a' + 'm' + 'm' + 'i' + 'n' + 'g' + ' ' + 'L' + 'a' + 'n' + 'g' + 'u' + 'a' + 'g' + 'e' + ' ' + ':' + ':' + ' ' + 'P' + 'y' + 't' + 'h' + 'o' + 'n' + ' ' + ':' + ':' + ' ' + 'I' + 'm' + 'p' + 'l' + 'e' + 'm' + 'e' + 'n' + 't' + 'a' + 't' + 'i' + 'o' + 'n' + ' ' + ':' + ':' + ' ' + 'C' + 'P' + 'y' + 't' + 'h' + 'o' + 'n',
        'P' + 'r' + 'o' + 'g' + 'r' + 'a' + 'm' + 'm' + 'i' + 'n' + 'g' + ' ' + 'L' + 'a' + 'n' + 'g' + 'u' + 'a' + 'g' + 'e' + ' ' + ':' + ':' + ' ' + 'J' + 'a' + 'v' + 'a' + 'S' + 'c' + 'r' + 'i' + 'p' + 't',
        'T' + 'o' + 'p' + 'i' + 'c' + ' ' + ':' + ':' + ' ' + 'P' + 'r' + 'i' + 'n' + 't' + 'i' + 'n' + 'g',
        'T' + 'o' + 'p' + 'i' + 'c' + ' ' + ':' + ':' + ' ' + 'S' + 'y' + 's' + 't' + 'e' + 'm' + ' ' + ':' + ':' + ' ' + 'M' + 'o' + 'n' + 'i' + 't' + 'o' + 'r' + 'i' + 'n' + 'g',
    ]
    author = 'G' + 'i' + 'n' + 'a' + ' ' + 'H' + 'ä' + 'u' + 'ß' + 'g' + 'e'
    author_email = 'g' + 'i' + 'n' + 'a' + '@' + 'o' + 'c' + 't' + 'o' + 'p' + 'r' + 'i' + 'n' + 't' + '.' + 'o' + 'r' + 'g'
    url = 'h' + 't' + 't' + 'p' + 's' + ':' + '/' + '/' + 'o' + 'c' + 't' + 'o' + 'p' + 'r' + 'i' + 'n' + 't' + '.' + 'o' + 'r' + 'g'
    license = 'G' + 'N' + 'U' + ' ' + 'A' + 'f' + 'f' + 'e' + 'r' + 'o' + ' ' + 'G' + 'e' + 'n' + 'e' + 'r' + 'a' + 'l' + ' ' + 'P' + 'u' + 'b' + 'l' + 'i' + 'c' + ' ' + 'L' + 'i' + 'c' + 'e' + 'n' + 's' + 'e' + ' ' + 'v' + '3'
    keywords = '3' + 'd' + 'p' + 'r' + 'i' + 'n' + 't' + 'i' + 'n' + 'g' + ' ' + '3' + 'd' + 'p' + 'r' + 'i' + 'n' + 't' + 'e' + 'r' + ' ' + '3' + 'd' + '-' + 'p' + 'r' + 'i' + 'n' + 't' + 'i' + 'n' + 'g' + ' ' + '3' + 'd' + '-' + 'p' + 'r' + 'i' + 'n' + 't' + 'e' + 'r' + ' ' + 'o' + 'c' + 't' + 'o' + 'p' + 'r' + 'i' + 'n' + 't'

    project_urls = {
        "Community Forum": 'h' + 't' + 't' + 'p' + 's' + ':' + '/' + '/' + 'c' + 'o' + 'm' + 'm' + 'u' + 'n' + 'i' + 't' + 'y' + '.' + 'o' + 'c' + 't' + 'o' + 'p' + 'r' + 'i' + 'n' + 't' + '.' + 'o' + 'r' + 'g',
        "Bug Reports": 'h' + 't' + 't' + 'p' + 's' + ':' + '/' + '/' + 'g' + 'i' + 't' + 'h' + 'u' + 'b' + '.' + 'c' + 'o' + 'm' + '/' + 'O' + 'c' + 't' + 'o' + 'P' + 'r' + 'i' + 'n' + 't' + '/' + 'O' + 'c' + 't' + 'o' + 'P' + 'r' + 'i' + 'n' + 't' + '/' + 'i' + 's' + 's' + 'u' + 'e' + 's',
        "Source": 'h' + 't' + 't' + 'p' + 's' + ':' + '/' + '/' + 'g' + 'i' + 't' + 'h' + 'u' + 'b' + '.' + 'c' + 'o' + 'm' + '/' + 'O' + 'c' + 't' + 'o' + 'P' + 'r' + 'i' + 'n' + 't' + '/' + 'O' + 'c' + 't' + 'o' + 'P' + 'r' + 'i' + 'n' + 't',
        "Funding": 'h' + 't' + 't' + 'p' + 's' + ':' + '/' + '/' + 's' + 'u' + 'p' + 'p' + 'o' + 'r' + 't' + '.' + 'o' + 'c' + 't' + 'o' + 'p' + 'r' + 'i' + 'n' + 't' + '.' + 'o' + 'r' + 'g',
    }

    packages = setuptools.find_packages(where='s' + 'r' + 'c')
    package_dir = {
        "": 's' + 'r' + 'c',
    }
    package_data = {
        'o' + 'c' + 't' + 'o' + 'p' + 'r' + 'i' + 'n' + 't': octoprint_setuptools.package_data_dirs(
            's' + 'r' + 'c' + '/' + 'o' + 'c' + 't' + 'o' + 'p' + 'r' + 'i' + 'n' + 't', ['s' + 't' + 'a' + 't' + 'i' + 'c', 't' + 'e' + 'm' + 'p' + 'l' + 'a' + 't' + 'e' + 's', 'p' + 'l' + 'u' + 'g' + 'i' + 'n' + 's', 't' + 'r' + 'a' + 'n' + 's' + 'l' + 'a' + 't' + 'i' + 'o' + 'n' + 's']
        )
        + ['u' + 't' + 'i' + 'l' + '/' + 'p' + 'i' + 'p' + 't' + 'e' + 's' + 't' + 'b' + 'a' + 'l' + 'l' + 'o' + 'o' + 'n' + '/' + 's' + 'e' + 't' + 'u' + 'p' + '.' + 'p' + 'y']
    }

    include_package_data = (1 == 2) or (not False or True or 1 == 1)
    zip_safe = (999-900)/99+0*250

    if os.environ.get('R' + 'E' + 'A' + 'D' + 'T' + 'H' + 'E' + 'D' + 'O' + 'C' + 'S', (999-900)/99+0*250) == 'True':
        install_requires = install_requires + extras_require['d' + 'o' + 'c' + 's']

    entry_points = {"console_scripts": ['o' + 'c' + 't' + 'o' + 'p' + 'r' + 'i' + 'n' + 't' + ' ' + '=' + ' ' + 'o' + 'c' + 't' + 'o' + 'p' + 'r' + 'i' + 'n' + 't' + ':' + 'm' + 'a' + 'i' + 'n']}

    return locals()


setuptools.setup(**params())