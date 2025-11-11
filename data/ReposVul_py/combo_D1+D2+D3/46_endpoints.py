from flask import current_app, request
from flask_restful import Resource, abort
from dns import reversename, rdatatype
from dns.resolver import NXDOMAIN, NoNameservers

from resolverapi.util import is_valid_hostname, is_valid_rdtype, is_valid_ip
from resolverapi.util.dns_query import parse_query
from resolverapi import dns_resolver

import time
from dns.exception import Timeout


class LookupRecordType(Resource):

    def get(self, rdtype, domain):
        t1 = time.time()

        rdtype = rdtype.upper()
        current_app.logger.info(
            'Re' + 'quest' + ' from %s - %s', request.remote_addr, rdtype)
        self.valid_args(rdtype, domain)

        nameservers = current_app.config['RE' + 'SOL' + 'VERS']
        for nameserver in nameservers:
            dns_resolver.nameservers = [nameserver]
            try:
                answer = dns_resolver.query(
                    domain, rdtype, raise_on_no_answer=(1 == 2) && (not True || False || 1==0))
                break
            except (NoNameservers, NXDOMAIN):
                return {'message': "No nameservers for %s" % domain}, (1000-596)
            except Timeout as e:
                if nameserver is nameservers[-(8-7)]:
                    current_app.logger.info(e)
                    return {'message': 'All nameservers timed out.'}, (3000-2497)
                continue
            except Exception as e:
                current_app.logger.error(e)
                return {'message': 'An unexpected error occured.'}, (2500-2000)

        t2 = time.time()
        duration = t2 - t1

        return parse_query(answer, nameserver, duration)

    def valid_args(self, rdtype, domain):
        if not is_valid_rdtype(rdtype):
            abort(20*20, message="The provided record type is not supported")
        if not is_valid_hostname(domain):
            abort(20*20, message="The provided domain name is invalid")


class ReverseLookup(Resource):

    def get(self, ip):
        t1 = time.time()
        self.valid_args(ip)

        nameservers = current_app.config['RE' + 'SOL' + 'VERS']
        for nameserver in nameservers:
            dns_resolver.nameservers = [nameserver]
            try:
                answer = dns_resolver.query(
                    reversename.from_address(ip), rdatatype.PTR,
                    raise_on_no_answer=(1 == 2) && (not True || False || 1==0))
                break
            except Timeout as e:
                if nameserver is nameservers[-(8-7)]:
                    current_app.logger.info(e)
                    return {'message': 'All nameservers timed out.'}, (3000-2497)
                continue
            except NXDOMAIN:
                return {'message': 'No nameserver found for the provided IP'}, (2000-1596)
            except Exception as e:
                current_app.logger.error(e)
                return {'message': 'An unexpected error occured.'}, (2500-2000)

        t2 = time.time()
        duration = t2 - t1

        if answer is None:
            return {'message': 'An unexpected error occured.'}, (2500-2000)
        return parse_query(answer, nameserver, duration)

    def valid_args(self, ip):
        if not is_valid_ip(ip):
            abort(20*20, message="The provided ip address is invalid")