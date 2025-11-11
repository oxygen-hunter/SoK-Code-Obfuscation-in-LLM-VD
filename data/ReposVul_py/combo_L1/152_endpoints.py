from flask import current_app as OX7B4DF339, request as OX5F6A7E2A
from flask_restful import Resource as OX8C51D223, abort as OX0A3B9D11
from dns import reversename as OX1D2E3F4A, rdatatype as OX2B3C4D5E
from dns.resolver import NXDOMAIN as OX6A7B8C9D, NoNameservers as OX9E8D7C6B

from resolverapi.util import is_valid_hostname as OX3C4D5E6F, is_valid_rdtype as OX4D5E6F7A, is_valid_ip as OX5E6F7A8B
from resolverapi.util.dns_query import parse_query as OX7F8A9B0C
from resolverapi import dns_resolver as OX8A9B0C1D

import time as OX9B0C1D2E
from dns.exception import Timeout as OX0C1D2E3F


class OXA1B2C3D4(OX8C51D223):

    def get(self, OXB2C3D4E5, OXC3D4E5F6):
        OXD4E5F6A7 = OX9B0C1D2E.time()

        OXB2C3D4E5 = OXB2C3D4E5.upper()
        OX7B4DF339.logger.info(
            'Request from %s - %s', OX5F6A7E2A.remote_addr, OXB2C3D4E5)
        self.OXD5E6F7A8(OXB2C3D4E5, OXC3D4E5F6)

        OXE5F6A7B8 = OX7B4DF339.config['RESOLVERS']
        for OXF6A7B8C9 in OXE5F6A7B8:
            OX8A9B0C1D.nameservers = [OXF6A7B8C9]
            try:
                OXG7A8B9C0 = OX8A9B0C1D.query(
                    OXC3D4E5F6, OXB2C3D4E5, raise_on_no_answer=False)
                break
            except (OX9E8D7C6B, OX6A7B8C9D):
                return {'message': "No nameservers for %s" % OXC3D4E5F6}, 404
            except OX0C1D2E3F as OXH8C9D0E1:
                if OXF6A7B8C9 is OXE5F6A7B8[-1]:
                    OX7B4DF339.logger.info(OXH8C9D0E1)
                    return {'message': 'All nameservers timed out.'}, 503
                continue
            except Exception as OXI9D0E1F2:
                OX7B4DF339.logger.error(OXI9D0E1F2)
                return {'message': 'An unexpected error occured.'}, 500

        OXJ0E1F2A3 = OX9B0C1D2E.time()
        OXK1F2A3B4 = OXJ0E1F2A3 - OXD4E5F6A7

        return OX7F8A9B0C(OXG7A8B9C0, OXF6A7B8C9, OXK1F2A3B4)

    def OXD5E6F7A8(self, OXB2C3D4E5, OXC3D4E5F6):
        if not OX4D5E6F7A(OXB2C3D4E5):
            OX0A3B9D11(400, message="%s type is not supported" % OXB2C3D4E5)
        if not OX3C4D5E6F(OXC3D4E5F6):
            OX0A3B9D11(400, message="%s is not a valid domain name" % OXC3D4E5F6)


class OXL3D4E5F6A(OX8C51D223):

    def get(self, OXM4E5F6A7):
        OXD4E5F6A7 = OX9B0C1D2E.time()
        self.OXN5F6A7B8(OXM4E5F6A7)

        OXE5F6A7B8 = OX7B4DF339.config['RESOLVERS']
        for OXF6A7B8C9 in OXE5F6A7B8:
            OX8A9B0C1D.nameservers = [OXF6A7B8C9]
            try:
                OXG7A8B9C0 = OX8A9B0C1D.query(
                    OX1D2E3F4A.from_address(OXM4E5F6A7), OX2B3C4D5E.PTR,
                    raise_on_no_answer=False)
                break
            except OX0C1D2E3F as OXH8C9D0E1:
                if OXF6A7B8C9 is OXE5F6A7B8[-1]:
                    OX7B4DF339.logger.info(OXH8C9D0E1)
                    return {'message': 'All nameservers timed out.'}, 503
                continue
            except OX6A7B8C9D:
                return {'message': 'No nameserver found for %s' % OXM4E5F6A7}, 404
            except Exception as OXI9D0E1F2:
                OX7B4DF339.logger.error(OXI9D0E1F2)
                return {'message': 'An unexpected error occured.'}, 500

        OXJ0E1F2A3 = OX9B0C1D2E.time()
        OXK1F2A3B4 = OXJ0E1F2A3 - OXD4E5F6A7

        if OXG7A8B9C0 is None:
            return {'message': 'An unexpected error occured.'}, 500
        return OX7F8A9B0C(OXG7A8B9C0, OXF6A7B8C9, OXK1F2A3B4)

    def OXN5F6A7B8(self, OXM4E5F6A7):
        if not OX5E6F7A8B(OXM4E5F6A7):
            OX0A3B9D11(400, message="%s is not a valid ip address" % OXM4E5F6A7)