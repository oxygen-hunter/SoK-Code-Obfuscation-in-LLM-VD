# -*- coding: utf-8 -*-

import os
import sys
from distutils.command.build_py import build_py as _build_py

import versioneer  # noqa: F401

sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), "src"))
import setuptools  # noqa: F401,E402

import octoprint_setuptools  # noqa: F401,E402

PYTHON_REQUIRES = ">=3.7, <4"

SETUP_REQUIRES = []

a1, a2, a3 = [
    "OctoPrint-FileCheck>=2021.2.23",
    "OctoPrint-FirmwareCheck>=2021.10.11",
    "OctoPrint-PiSupport>=2022.3.28",
]
b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35 = [
    "cachelib>=0.2,<0.3",
    "Click>=8.0.3,<9",
    "colorlog>=6,<7",
    "emoji>=1.4.2,<2",
    "feedparser>=6.0.8,<7",
    "filetype>=1.0.7,<2",
    "Flask-Assets>=2.0,<3",
    "Flask-Babel>=2.0,<3",
    "Flask-Login>=0.6,<0.7",  
    "Flask-Limiter>=2.6,<3",
    "flask>=2.2,<2.3",  
    "frozendict>=2.0,<3",
    "future>=0.18.2,<1",  
    "markdown>=3.2.2,<4",
    "netaddr>=0.8,<0.9",  
    "netifaces>=0.11,<1",
    "pathvalidate>=2.4.1,<3",
    "pkginfo>=1.7.1,<2",
    "psutil>=5.8,<6",
    "pylru>=1.2,<2",
    "pyserial>=3.4,<4",
    "PyYAML>=5.4.1,<6",
    "requests>=2.26.0,<3",
    "sarge==0.1.6",
    "semantic_version>=2.8.5,<3",
    "sentry-sdk>=1.5.7,<2",
    "tornado>=6.0.4,<7",
    "watchdog>=1,<2",
    "websocket-client>=1.2.1,<2",
    "werkzeug>=2.2,<2.3",  
    "wrapt>=1.14,<1.15",
    "zeroconf>=0.33,<0.34",  
    "zipstream-ng>=1.3.4,<2.0.0",
    "pydantic>=1.9.1,<2",
    "pydantic-settings>=0.2.5,<1",  
]
c1, c2, c3 = [
    "blinker>=1.4,<2",  
    "regex",  
    "unidecode",  
]

INSTALL_REQUIRES = [a1, a2, a3, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, b11, b12, b13, b14, b15, b16, b17, b18, b19, b20, b21, b22, b23, b24, b25, b26, b27, b28, b29, b30, b31, b32, b33, b34, b35, c1, c2, c3]

EXTRA_REQUIRES = {
    ":sys_platform == 'darwin'": [
        "appdirs>=1.4.4,<2",
    ],
    "develop": [
        "ddt",
        "mock>=4,<5",
        "pytest-doctest-custom>=1.0.0,<2",
        "pytest>=6.2.5,<7",
        "pre-commit",
        "pyinstrument",
    ],
    "plugins": ["cookiecutter>=1.7.2,<1.8"],
    "docs": [
        "readthedocs-sphinx-ext>=2.1.5,<3",
        "sphinx_rtd_theme>=1,<2",
        "sphinx>=4,<5",
        "sphinxcontrib-httpdomain>=1.8.0,<2",
        "sphinxcontrib-mermaid>=0.7.1,<0.8",
    ],
}

here = os.path.abspath(os.path.dirname(__file__))


def read_file_contents(p):
    import io

    with io.open(p, encoding="utf-8") as f:
        return f.read()


def copy_files_build_py_factory(f, b):
    class copy_files_build_py(b):
        files = {}

        def run(self):
            print("RUNNING copy_files_build_py")
            if not self.dry_run:
                import shutil

                for d, f in self.files.items():
                    t = os.path.join(self.build_lib, d)
                    self.mkpath(t)

                    for e in f:
                        if isinstance(e, tuple):
                            if len(e) != 2:
                                continue
                            s, d = e[0], os.path.join(t, e[1])
                        else:
                            s = e
                            d = os.path.join(t, s)

                        print("Copying {} to {}".format(s, d))
                        shutil.copy2(s, d)

            b.run(self)

    return type(copy_files_build_py)(
        copy_files_build_py.__name__, (copy_files_build_py,), {"files": f}
    )


def get_cmdclass():
    global versioneer, octoprint_setuptools, md_to_html_build_py_factory

    cmdclass = versioneer.get_cmdclass()

    cmdclass.update(
        {
            "clean": octoprint_setuptools.CleanCommand.for_options(
                source_folder="src", eggs=["OctoPrint*.egg-info"]
            )
        }
    )

    d1, d2 = "translations", os.path.join("translations", "messages.pot")
    bundled_dir = os.path.join("src", "octoprint", "translations")
    cmdclass.update(
        octoprint_setuptools.get_babel_commandclasses(
            pot_file=d2,
            output_dir=d1,
            pack_name_prefix="OctoPrint-i18n-",
            pack_path_prefix="",
            bundled_dir=bundled_dir,
        )
    )

    cmdclass["build_py"] = copy_files_build_py_factory(
        {
            "octoprint/templates/_data": [
                "AUTHORS.md",
                "SUPPORTERS.md",
                "THIRDPARTYLICENSES.md",
            ]
        },
        cmdclass["build_py"] if "build_py" in cmdclass else _build_py,
    )

    return cmdclass


def params():
    global versioneer, get_cmdclass, read_file_contents, here, PYTHON_REQUIRES, SETUP_REQUIRES, INSTALL_REQUIRES, EXTRA_REQUIRES

    n, v, c = "OctoPrint", versioneer.get_version(), get_cmdclass()
    d, ld = "The snappy web interface for your 3D printer", read_file_contents(os.path.join(here, "README.md"))
    ldct = "text/markdown"

    pr, sr, ir, er = PYTHON_REQUIRES, SETUP_REQUIRES, INSTALL_REQUIRES, EXTRA_REQUIRES

    cl = [
        "Development Status :: 5 - Production/Stable",
        "Environment :: Web Environment",
        "Framework :: Flask",
        "Intended Audience :: Developers",
        "Intended Audience :: Education",
        "Intended Audience :: End Users/Desktop",
        "Intended Audience :: Manufacturing",
        "Intended Audience :: Other Audience",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: GNU Affero General Public License v3",
        "Natural Language :: English",
        "Natural Language :: German",
        "Operating System :: OS Independent",
        "Programming Language :: Python",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: Implementation :: CPython",
        "Programming Language :: JavaScript",
        "Topic :: Printing",
        "Topic :: System :: Monitoring",
    ]
    author, author_email = "Gina Häußge", "gina@octoprint.org"
    url, license = "https://octoprint.org", "GNU Affero General Public License v3"
    keywords = "3dprinting 3dprinter 3d-printing 3d-printer octoprint"

    project_urls = {
        "Community Forum": "https://community.octoprint.org",
        "Bug Reports": "https://github.com/OctoPrint/OctoPrint/issues",
        "Source": "https://github.com/OctoPrint/OctoPrint",
        "Funding": "https://support.octoprint.org",
    }

    packages = setuptools.find_packages(where="src")
    package_dir = {
        "": "src",
    }
    package_data = {
        "octoprint": octoprint_setuptools.package_data_dirs(
            "src/octoprint", ["static", "templates", "plugins", "translations"]
        )
        + ["util/piptestballoon/setup.py"]
    }

    include_package_data, zip_safe = True, False

    if os.environ.get("READTHEDOCS", None) == "True":
        ir = ir + er["docs"]

    entry_points = {"console_scripts": ["octoprint = octoprint:main"]}

    return locals()


setuptools.setup(**params())