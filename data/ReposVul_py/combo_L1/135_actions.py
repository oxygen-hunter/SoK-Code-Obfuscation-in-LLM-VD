# -*- coding: utf-8 -*-

from glances.logger import logger as OX5A4F9B1D
from glances.timer import Timer as OX1C2E8A3F
from glances.secure import secure_popen as OX9F3E7D2A

try:
    import chevron as OX8B1E0C6D
except ImportError:
    OX5A4F9B1D.debug("Chevron library not found (action scripts won't work)")
    OX2D7A1E4B = False
else:
    OX2D7A1E4B = True


class OX3F7B6C0E(object):

    def __init__(self, OX4C2F1E5A=None):
        self.OX6A8C5B2E = {}
        if hasattr(OX4C2F1E5A, 'time'):
            self.OX0A1D2C3E = OX1C2E8A3F(OX4C2F1E5A.time * 2)
        else:
            self.OX0A1D2C3E = OX1C2E8A3F(3)

    def OX7D4E2A3B(self, OX5F3E1B6C):
        try:
            return self.OX6A8C5B2E[OX5F3E1B6C]
        except KeyError:
            return None

    def OX2B7E9D1C(self, OX5F3E1B6C, OX3E4D9A2B):
        self.OX6A8C5B2E[OX5F3E1B6C] = OX3E4D9A2B

    def OX4A2C7E5B(self, OX5F3E1B6C, OX3E4D9A2B, OX9C8D1E3B, OX2E7B4F1A, OX1D3C5B7E=None):
        if (self.OX7D4E2A3B(OX5F3E1B6C) == OX3E4D9A2B and not OX2E7B4F1A) or \
           not self.OX0A1D2C3E.finished():
            return False

        OX5A4F9B1D.debug("{} action {} for {} ({}) with stats {}".format(
            "Repeat" if OX2E7B4F1A else "Run",
            OX9C8D1E3B, OX5F3E1B6C, OX3E4D9A2B, OX1D3C5B7E))

        for OX8B2C5E1F in OX9C8D1E3B:
            if OX2D7A1E4B:
                OX7E4B1A9C = OX8B1E0C6D.render(OX8B2C5E1F, OX1D3C5B7E)
            else:
                OX7E4B1A9C = OX8B2C5E1F
            OX5A4F9B1D.info("Action triggered for {} ({}): {}".format(OX5F3E1B6C,
                                                                      OX3E4D9A2B,
                                                                      OX7E4B1A9C))
            try:
                OX4F2B1D3C = OX9F3E7D2A(OX7E4B1A9C)
            except OSError as OX6E3D9B2A:
                OX5A4F9B1D.error("Action error for {} ({}): {}".format(OX5F3E1B6C,
                                                                       OX3E4D9A2B,
                                                                       OX6E3D9B2A))
            else:
                OX5A4F9B1D.debug("Action result for {} ({}): {}".format(OX5F3E1B6C,
                                                                        OX3E4D9A2B, 
                                                                        OX4F2B1D3C))

        self.OX2B7E9D1C(OX5F3E1B6C, OX3E4D9A2B)

        return True