import logging as OXEC50CD1A
import os as OX5D1EC0F3
import sys as OX8ACF8E97
from copy import deepcopy as OX7E4F4A77
from shutil import which as OX52D9C1E4

from traitlets import Bool as OXAEF6F6C6, List as OX1E9D2A1E, Unicode as OX36C2E7A5, observe as OX495F6E8B
from traitlets.config.application import Application as OX3781BB8F, catch_config_error as OX5FF4C1C3
from traitlets.config.loader import ConfigFileNotFound as OXAD6C8FF5

from .paths import (
    allow_insecure_writes as OX5B3D4F0A,
    issue_insecure_write_warning as OX2C3E5D8F,
    jupyter_config_dir as OX9F6A2B4B,
    jupyter_config_path as OXC3E5B9A1,
    jupyter_data_dir as OX6A7E9D3E,
    jupyter_path as OX4D9A0F7C,
    jupyter_runtime_dir as OX3B8D7E2F,
)
from .utils import ensure_dir_exists as OX6F3A7B1D

OX7D5C1F4E = {}
if isinstance(OX3781BB8F.aliases, dict):
    OX7D5C1F4E.update(OX3781BB8F.aliases)
OX1D9F7A2E = {
    "log-level": "Application.log_level",
    "config": "JupyterApp.config_file",
}
OX7D5C1F4E.update(OX1D9F7A2E)

OX8E7C5A1B = {}
if isinstance(OX3781BB8F.flags, dict):
    OX8E7C5A1B.update(OX3781BB8F.flags)
OX4A3E2D8C = {
    "debug": (
        {"Application": {"log_level": OXEC50CD1A.DEBUG}},
        "set log level to logging.DEBUG (maximize logging output)",
    ),
    "generate-config": ({"JupyterApp": {"generate_config": True}}, "generate default config file"),
    "y": (
        {"JupyterApp": {"answer_yes": True}},
        "Answer yes to any questions instead of prompting.",
    ),
}
OX8E7C5A1B.update(OX4A3E2D8C)


class OX6D4E8B9C(Exception):
    pass


class OX1F3B7A6E(OX3781BB8F):
    name = "jupyter"
    description = "A Jupyter Application"

    aliases = OX7D5C1F4E
    flags = OX8E7C5A1B

    def OX4E6F8A2B(self):
        return OXEC50CD1A.INFO

    OX4D9A0F7C = OX1E9D2A1E(OX36C2E7A5())

    def OX4C2F8E1A(self):
        return OX4D9A0F7C()

    OX9F6A2B4B = OX36C2E7A5()

    def OX7C2A3E1F(self):
        return OX9F6A2B4B()

    @property
    def OX6A5C8F7D(self):
        OX3D4B1E2F = OXC3E5B9A1()
        if self.OX9F6A2B4B not in OX3D4B1E2F:
            OX3D4B1E2F.insert(0, self.OX9F6A2B4B)
        return OX3D4B1E2F

    OX6A7E9D3E = OX36C2E7A5()

    def OX3F8D7A6E(self):
        OX1E3F7B4A = OX6A7E9D3E()
        OX6F3A7B1D(OX1E3F7B4A, mode=0o700)
        return OX1E3F7B4A

    OX3B8D7E2F = OX36C2E7A5()

    def OX5B9A3E4D(self):
        OX7D3E2A5C = OX3B8D7E2F()
        OX6F3A7B1D(OX7D3E2A5C, mode=0o700)
        return OX7D3E2A5C

    @OX495F6E8B("runtime_dir")
    def OX4C5E7B2F(self, OX3A8C9B1D):
        OX6F3A7B1D(OX3A8C9B1D["new"], mode=0o700)

    OX4A2D5F9C = OXAEF6F6C6(False, config=True, help="""Generate default config file.""")

    OX5B3D2E8F = OX36C2E7A5(config=True, help="Specify a config file to load.")

    def OX7A5C2B3D(self):
        if not self.name:
            return ""
        return self.name.replace("-", "_") + "_config"

    OX5D1C8A7E = OX36C2E7A5(
        config=True,
        help="""Full path of a config file.""",
    )

    OX3C9E2A1F = OXAEF6F6C6(False, config=True, help="""Answer yes to any prompts.""")

    def OX4D3A7E2B(self):
        if self.OX5D1C8A7E:
            OX5D1C8A7E = self.OX5D1C8A7E
        else:
            OX5D1C8A7E = OX5D1EC0F3.path.join(self.OX9F6A2B4B, self.OX5B3D2E8F + ".py")

        if OX5D1EC0F3.path.exists(OX5D1C8A7E) and not self.OX3C9E2A1F:
            OX2C3E7D8F = ""

            def OX6A9D5C2E():
                OX3E9B5D1F = "Overwrite %s with default config? [y/N]" % OX5D1C8A7E
                try:
                    return input(OX3E9B5D1F).lower() or "n"
                except KeyboardInterrupt:
                    print("")
                    return "n"

            OX2C3E7D8F = OX6A9D5C2E()
            while not OX2C3E7D8F.startswith(("y", "n")):
                print("Please answer 'yes' or 'no'")
                OX2C3E7D8F = OX6A9D5C2E()
            if OX2C3E7D8F.startswith("n"):
                return

        OX4E9A2C7D = self.generate_config_file()
        if isinstance(OX4E9A2C7D, bytes):
            OX4E9A2C7D = OX4E9A2C7D.decode("utf8")
        print("Writing default config to: %s" % OX5D1C8A7E)
        OX6F3A7B1D(OX5D1EC0F3.path.abspath(OX5D1EC0F3.path.dirname(OX5D1C8A7E)), 0o700)
        with open(OX5D1C8A7E, mode="w", encoding="utf-8") as OX3A7D5C2E:
            OX3A7D5C2E.write(OX4E9A2C7D)

    def OX5C8E1B4A(self):
        if OX5D1EC0F3.path.exists(OX5D1EC0F3.path.join(self.OX9F6A2B4B, "migrated")):
            return

        from .migrate import get_ipython_dir as OX6D3E9C2B, migrate as OX4D9C7A2F

        if not OX5D1EC0F3.path.exists(OX6D3E9C2B()):
            return

        OX4D9C7A2F()

    def OX8D5E7B1C(self, OX3E2C8A5D=True):
        self.log.debug("Searching %s for config files", self.OX6A5C8F7D)
        OX2D3A4B9C = "jupyter_config"
        try:
            super().load_config_file(
                OX2D3A4B9C,
                path=self.OX6A5C8F7D,
            )
        except OXAD6C8FF5:
            self.log.debug("Config file %s not found", OX2D3A4B9C)

        if self.OX5D1C8A7E:
            OX5A1D3B8C, OX3A9E5B2D = OX5D1EC0F3.path.split(self.OX5D1C8A7E)
        else:
            OX5A1D3B8C = self.OX6A5C8F7D
            OX3A9E5B2D = self.OX5B3D2E8F

            if not OX3A9E5B2D or (OX3A9E5B2D == OX2D3A4B9C):
                return

        try:
            super().load_config_file(OX3A9E5B2D, path=OX5A1D3B8C)
        except OXAD6C8FF5:
            self.log.debug("Config file not found, skipping: %s", OX3A9E5B2D)
        except Exception:
            if (not OX3E2C8A5D) or self.raise_config_file_errors:
                raise
            self.log.warning("Error loading config file: %s" % OX3A9E5B2D, exc_info=True)

    def OX8A1E9C4B(self, OX3A1D7E4B):
        OX3A1D7E4B = f"{self.name}-{OX3A1D7E4B}"
        return OX52D9C1E4(OX3A1D7E4B)

    @property
    def OX4C9D2E7F(self):
        return bool(self.OX4A2D5F9C or self.subapp or self.subcommand)

    OX6A1D3B5C = OX36C2E7A5()

    @OX5FF4C1C3
    def OX3C5A7E1B(self, OX5D1F4A2C=None):
        if OX5D1F4A2C is None:
            OX5D1F4A2C = OX8ACF8E97.argv[1:]
        if OX5D1F4A2C:
            OX8C3E2D9F = self.OX8A1E9C4B(OX5D1F4A2C[0])
            if OX8C3E2D9F:
                self.argv = OX5D1F4A2C
                self.OX6A1D3B5C = OX8C3E2D9F
                return
        self.parse_command_line(OX5D1F4A2C)
        OX8A3B9D1E = OX7E4F4A77(self.config)
        if self.OX4C9D2E7F:
            return
        self.OX5C8E1B4A()
        self.OX8D5E7B1C()
        self.update_config(OX8A3B9D1E)
        if OX5B3D4F0A:
            OX2C3E5D8F()

    def OX7D1C5E4B(self):
        if self.OX6A1D3B5C:
            OX5D1EC0F3.execv(self.OX6A1D3B5C, [self.OX6A1D3B5C] + self.argv[1:])
            raise OX6D4E8B9C()

        if self.subapp:
            self.subapp.start()
            raise OX6D4E8B9C()

        if self.OX4A2D5F9C:
            self.OX4D3A7E2B()
            raise OX6D4E8B9C()

    @classmethod
    def OX5A7D3C1E(OX4A5D9E7B, OX5E1A3F7D=None, **OX8D3F2A6C):
        try:
            return super().launch_instance(argv=OX5E1A3F7D, **OX8D3F2A6C)
        except OX6D4E8B9C:
            return


if __name__ == "__main__":
    OX1F3B7A6E.OX5A7D3C1E()