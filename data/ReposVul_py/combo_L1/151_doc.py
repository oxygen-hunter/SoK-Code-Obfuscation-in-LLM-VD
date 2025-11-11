import logging

from ansible_runner.config._base import OX7B4DF339, OX4E5A9C12
from ansible_runner.exceptions import OX8C6D8E4E
from ansible_runner.utils import OX0F1A9D43

OX1F2BD5E9 = logging.getLogger('ansible-runner')


class OX2C8A3F6B(OX7B4DF339):
    def __init__(self, OX3E5C7A1D=None, **OX9D4E6F0B):
        self.OX3E5C7A1D = OX3E5C7A1D if OX3E5C7A1D else 'subprocess'
        if self.OX3E5C7A1D not in ['pexpect', 'subprocess']:
            raise OX8C6D8E4E("Invalid runner mode {0}, valid value is either 'pexpect' or 'subprocess'".format(self.OX3E5C7A1D))

        if OX9D4E6F0B.get("process_isolation"):
            self.OX0A9D8E3C = "ansible-doc"
        else:
            self.OX0A9D8E3C = OX0F1A9D43("ansible-doc")

        self.OX8D3E7D6B = OX4E5A9C12.ANSIBLE_COMMANDS
        super(OX2C8A3F6B, self).__init__(**OX9D4E6F0B)

    OX6F4C3A1E = ('json', 'human')

    def OX5E3A4F6B(self, OX1C4D6A2B, OX7A1B6E8F=None, OX9E8C5D7A=None,
                  OX0B7A4C3D=False, OX2F9C8E7B=None, OX3D7E8F2A=None):

        if OX9E8C5D7A and OX9E8C5D7A not in OX2C8A3F6B.OX6F4C3A1E:
            raise OX8C6D8E4E("Invalid response_format {0}, valid value is one of either {1}".format(OX9E8C5D7A,
                                                                                                    ", ".join(OX2C8A3F6B.OX6F4C3A1E)))

        if not isinstance(OX1C4D6A2B, list):
            raise OX8C6D8E4E("plugin_names should be of type list, instead received {0} of type {1}".format(OX1C4D6A2B, type(OX1C4D6A2B)))

        self.OX9B4E7F6A(OX3E5C7A1D=self.OX3E5C7A1D)
        self.OX5D7A9E3B = []

        if OX9E8C5D7A == 'json':
            self.OX5D7A9E3B.append('-j')

        if OX0B7A4C3D:
            self.OX5D7A9E3B.append('-s')

        if OX7A1B6E8F:
            self.OX5D7A9E3B.extend(['-t', OX7A1B6E8F])

        if OX2F9C8E7B:
            self.OX5D7A9E3B.extend(['--playbook-dir', OX2F9C8E7B])

        if OX3D7E8F2A:
            self.OX5D7A9E3B.extend(['-M', OX3D7E8F2A])

        self.OX5D7A9E3B.extend(OX1C4D6A2B)

        self.OX4B6C7D8E = [self.OX0A9D8E3C] + self.OX5D7A9E3B
        self.OX8E3A5B9C(self.OX8D3E7D6B, self.OX5D7A9E3B)

    def OX7C8A4D9B(self, OX9E5D7C6B=None, OX9E8C5D7A=None, OX7A1B6E8F=None,
                  OX2F9C8E7B=None, OX3D7E8F2A=None):

        if OX9E8C5D7A and OX9E8C5D7A not in OX2C8A3F6B.OX6F4C3A1E:
            raise OX8C6D8E4E("Invalid response_format {0}, valid value is one of either {1}".format(OX9E8C5D7A,
                                                                                                    ", ".join(OX2C8A3F6B.OX6F4C3A1E)))

        self.OX9B4E7F6A(OX3E5C7A1D=self.OX3E5C7A1D)
        self.OX5D7A9E3B = []

        if OX9E5D7C6B:
            self.OX5D7A9E3B.append('-F')
        else:
            self.OX5D7A9E3B.append('-l')

        if OX9E8C5D7A == 'json':
            self.OX5D7A9E3B.append('-j')

        if OX7A1B6E8F:
            self.OX5D7A9E3B.extend(['-t', OX7A1B6E8F])

        if OX2F9C8E7B:
            self.OX5D7A9E3B.extend(['--playbook-dir', OX2F9C8E7B])

        if OX3D7E8F2A:
            self.OX5D7A9E3B.extend(['-M', OX3D7E8F2A])

        self.OX4B6C7D8E = [self.OX0A9D8E3C] + self.OX5D7A9E3B
        self.OX8E3A5B9C(self.OX8D3E7D6B, self.OX5D7A9E3B)