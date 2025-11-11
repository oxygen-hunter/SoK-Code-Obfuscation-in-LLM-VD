# -*- coding: utf-8 -*-

import os
import sys
from distutils.command.build_py import build_py as OX5D9DFA67

import versioneer  # noqa: F401

sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), "src"))
import setuptools  # noqa: F401,E402

import octoprint_setuptools  # noqa: F401,E402

OX2F3A5A4F = ">=3.7, <4"

OX3E1BFE2B = []

OX3B946C5D = [
    "OctoPrint-FileCheck>=2021.2.23",
    "OctoPrint-FirmwareCheck>=2021.10.11",
    "OctoPrint-PiSupport>=2022.3.28",
]
OX1B6C5A9D = [
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
OX4A7F2B8D = [
    "blinker>=1.4,<2",
    "regex",
    "unidecode",
]

OX7F2D3C5B = OX3B946C5D + OX1B6C5A9D + OX4A7F2B8D

OX6C4B3D9F = {
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

OX4F1A6B3C = os.path.abspath(os.path.dirname(__file__))


def OX5B3D6F2A(OX8A2E4C7D):
    import io

    with io.open(OX8A2E4C7D, encoding="utf-8") as OX6E7F1B9C:
        return OX6E7F1B9C.read()


def OX8A4C1B2D(OX3B7F9E2C, OX5D2F8A4B):
    class OX6C2D3A4B(OX5D2F8A4B):
        OX3B7F9E2C = {}

        def run(self):
            print("RUNNING OX6C2D3A4B")
            if not self.dry_run:
                import shutil

                for OX9D3F2B8C, OX7D1F2A4C in self.OX3B7F9E2C.items():
                    OX6C9B8A2D = os.path.join(self.build_lib, OX9D3F2B8C)
                    self.mkpath(OX6C9B8A2D)

                    for OX2F1A5B7D in OX7D1F2A4C:
                        if isinstance(OX2F1A5B7D, tuple):
                            if len(OX2F1A5B7D) != 2:
                                continue
                            OX4A8D2C5B, OX9B5C1E2A = OX2F1A5B7D[0], os.path.join(OX6C9B8A2D, OX2F1A5B7D[1])
                        else:
                            OX4A8D2C5B = OX2F1A5B7D
                            OX9B5C1E2A = os.path.join(OX6C9B8A2D, OX4A8D2C5B)

                        print("Copying {} to {}".format(OX4A8D2C5B, OX9B5C1E2A))
                        shutil.copy2(OX4A8D2C5B, OX9B5C1E2A)

            OX5D2F8A4B.run(self)

    return type(OX6C2D3A4B)(
        OX6C2D3A4B.__name__, (OX6C2D3A4B,), {"OX3B7F9E2C": OX3B7F9E2C}
    )


def OX9D1F3B7A():
    global versioneer, octoprint_setuptools, OX8A4C1B2D

    OX5F7C1A2D = versioneer.get_cmdclass()

    OX5F7C1A2D.update(
        {
            "clean": octoprint_setuptools.CleanCommand.for_options(
                source_folder="src", eggs=["OctoPrint*.egg-info"]
            )
        }
    )

    OX7A5C2D4B = "translations"
    OX8F3B1D7C = os.path.join(OX7A5C2D4B, "messages.pot")
    OX5D4F7C2A = os.path.join("src", "octoprint", "translations")
    OX5F7C1A2D.update(
        octoprint_setuptools.get_babel_commandclasses(
            pot_file=OX8F3B1D7C,
            output_dir=OX7A5C2D4B,
            pack_name_prefix="OctoPrint-i18n-",
            pack_path_prefix="",
            bundled_dir=OX5D4F7C2A,
        )
    )

    OX5F7C1A2D["build_py"] = OX8A4C1B2D(
        {
            "octoprint/templates/_data": [
                "AUTHORS.md",
                "SUPPORTERS.md",
                "THIRDPARTYLICENSES.md",
            ]
        },
        OX5F7C1A2D["build_py"] if "build_py" in OX5F7C1A2D else OX5D9DFA67,
    )

    return OX5F7C1A2D


def OX6B8F1D3C():
    global versioneer, OX9D1F3B7A, OX5B3D6F2A, OX4F1A6B3C, OX2F3A5A4F, OX3E1BFE2B, OX7F2D3C5B, OX6C4B3D9F

    OX1A4D3C7B = "OctoPrint"
    OX3C5E8B2F = versioneer.get_version()
    OX4B1D9E3C = OX9D1F3B7A()

    OX5F3A9D2B = "The snappy web interface for your 3D printer"
    OX7E2C1A4B = OX5B3D6F2A(os.path.join(OX4F1A6B3C, "README.md"))
    OX6F3B1C4D = "text/markdown"

    OX2C1D3F8A = OX2F3A5A4F
    OX7D4C2A6F = OX3E1BFE2B
    OX8B1F3E7C = OX7F2D3C5B
    OX9E3A2C5B = OX6C4B3D9F

    OX5A7F1D3B = [
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
    OX4B2F1A3D = "Gina Häußge"
    OX2D3A7C1F = "gina@octoprint.org"
    OX3A4C5B2D = "https://octoprint.org"
    OX8B2D3C1A = "GNU Affero General Public License v3"
    OX7A1F2E3C = "3dprinting 3dprinter 3d-printing 3d-printer octoprint"

    OX6D3F1B7C = {
        "Community Forum": "https://community.octoprint.org",
        "Bug Reports": "https://github.com/OctoPrint/OctoPrint/issues",
        "Source": "https://github.com/OctoPrint/OctoPrint",
        "Funding": "https://support.octoprint.org",
    }

    OX9C1A3E7B = setuptools.find_packages(where="src")
    OX4E2D3A1B = {
        "": "src",
    }
    OX5B3E1C7D = {
        "octoprint": octoprint_setuptools.package_data_dirs(
            "src/octoprint", ["static", "templates", "plugins", "translations"]
        )
        + ["util/piptestballoon/setup.py"]
    }

    OX7B8F1C2D = True
    OX2D6C3A4B = False

    if os.environ.get("READTHEDOCS", None) == "True":
        OX8B1F3E7C = OX8B1F3E7C + OX9E3A2C5B["docs"]

    OX4A3D7E2B = {"console_scripts": ["octoprint = octoprint:main"]}

    return locals()


setuptools.setup(**OX6B8F1D3C())