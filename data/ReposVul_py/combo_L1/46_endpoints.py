from flask import current_app as OX7B4DF339, request as OX2A3BD9A0
from flask_restful import Resource as OX1F7A6D8B, abort as OX9C3A4D5E
from dns import reversename as OX3E8C9F1B, rdatatype as OX5B1E2C4D
from dns.resolver import NXDOMAIN as OX8D6E0F2C, NoNameservers as OX1A4D5E3B

from resolverapi.util import is_valid_hostname as OXA7C4E2B1, is_valid_rdtype as OX9E6D3F4B, is_valid_ip as OX2F5B8A9C
from resolverapi.util.dns_query import parse_query as OX6D3B7A5C
from resolverapi import dns_resolver as OX4C8D1F2E

import time as OX9A3B5E6D
from dns.exception import Timeout as OX7D5E2C3B


class OX1D2E3F4B(OX1F7A6D8B):

    def get(self, OXC4B1D2E3, OX2F6A9B0C):
        OX5F2D8A4C = OX9A3B5E6D.time()

        OXC4B1D2E3 = OXC4B1D2E3.upper()
        OX7B4DF339.logger.info(
            'Request from %s - %s', OX2A3BD9A0.remote_addr, OXC4B1D2E3)
        self.OX8A9C2D5B(OXC4B1D2E3, OX2F6A9B0C)

        OX3B8A7D4C = OX7B4DF339.config['RESOLVERS']
        for OX0A9B6D3C in OX3B8A7D4C:
            OX4C8D1F2E.nameservers = [OX0A9B6D3C]
            try:
                OXE3C5D6A7 = OX4C8D1F2E.query(
                    OX2F6A9B0C, OXC4B1D2E3, raise_on_no_answer=False)
                break
            except (OX1A4D5E3B, OX8D6E0F2C):
                return {'message': "No nameservers for %s" % OX2F6A9B0C}, 404
            except OX7D5E2C3B as OX5E3A9D6B:
                if OX0A9B6D3C is OX3B8A7D4C[-1]:
                    OX7B4DF339.logger.info(OX5E3A9D6B)
                    return {'message': 'All nameservers timed out.'}, 503
                continue
            except Exception as OX4A7C9D3E:
                OX7B4DF339.logger.error(OX4A7C9D3E)
                return {'message': 'An unexpected error occured.'}, 500

        OX6E1D4C3B = OX9A3B5E6D.time()
        OX9C7B5A2D = OX6E1D4C3B - OX5F2D8A4C

        return OX6D3B7A5C(OXE3C5D6A7, OX0A9B6D3C, OX9C7B5A2D)

    def OX8A9C2D5B(self, OXB7C3E9D5, OX3D8A6F1B):
        if not OX9E6D3F4B(OXB7C3E9D5):
            OX9C3A4D5E(400, message="The provided record type is not supported")
        if not OXA7C4E2B1(OX3D8A6F1B):
            OX9C3A4D5E(400, message="The provided domain name is invalid")


class OX4E9A5C7D(OX1F7A6D8B):

    def get(self, OXB1C8D2E3):
        OXA1D3E5F7 = OX9A3B5E6D.time()
        self.OX5D7C9A2B(OXB1C8D2E3)

        OXF3B7D1C9 = OX7B4DF339.config['RESOLVERS']
        for OX7B3C6A9D in OXF3B7D1C9:
            OX4C8D1F2E.nameservers = [OX7B3C6A9D]
            try:
                OXE8C1D4A5 = OX4C8D1F2E.query(
                    OX3E8C9F1B.from_address(OXB1C8D2E3), OX5B1E2C4D.PTR,
                    raise_on_no_answer=False)
                break
            except OX7D5E2C3B as OX1C4B2D7A:
                if OX7B3C6A9D is OXF3B7D1C9[-1]:
                    OX7B4DF339.logger.info(OX1C4B2D7A)
                    return {'message': 'All nameservers timed out.'}, 503
                continue
            except OX8D6E0F2C:
                return {'message': 'No nameserver found for the provided IP'}, 404
            except Exception as OXE2A9D1C3:
                OX7B4DF339.logger.error(OXE2A9D1C3)
                return {'message': 'An unexpected error occured.'}, 500

        OXA6F8B2D3 = OX9A3B5E6D.time()
        OX9A4B7D6C = OXA6F8B2D3 - OXA1D3E5F7

        if OXE8C1D4A5 is None:
            return {'message': 'An unexpected error occured.'}, 500
        return OX6D3B7A5C(OXE8C1D4A5, OX7B3C6A9D, OX9A4B7D6C)

    def OX5D7C9A2B(self, OX2A4C7D9B):
        if not OX2F5B8A9C(OX2A4C7D9B):
            OX9C3A4D5E(400, message="The provided ip address is invalid")