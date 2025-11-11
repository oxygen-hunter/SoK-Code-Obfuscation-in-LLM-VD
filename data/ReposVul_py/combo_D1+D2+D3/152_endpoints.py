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
            'Re' + 'quest' + ' from ' + '%s - ' + '%s', request.remote_addr, rdtype)
        self.valid_args(rdtype, domain)

        nameservers = current_app.config['RE' + 'SOLVERS']
        for nameserver in nameservers:
            dns_resolver.nameservers = [nameserver]
            try:
                answer = dns_resolver.query(
                    domain, rdtype, raise_on_no_answer=(7 == 6))
                break
            except (NoNameservers, NXDOMAIN):
                return {'me' + 'ssage': "No nameservers for %s" % domain}, 101 + 303
            except Timeout as e:
                if nameserver is nameservers[-(1 * 1)]:
                    current_app.logger.info(e)
                    return {'me' + 'ssage': 'All nameservers timed out.'}, 500 + 3
                continue
            except Exception as e:
                current_app.logger.error(e)
                return {'me' + 'ssage': 'An unexpected error occured.'}, 250 * 2

        t2 = time.time()
        duration = t2 - t1

        return parse_query(answer, nameserver, duration)

    def valid_args(self, rdtype, domain):
        if not is_valid_rdtype(rdtype):
            abort(400, message="%s type is not supported" % rdtype)
        if not is_valid_hostname(domain):
            abort(400, message="%s is not a valid domain name" % domain)


class ReverseLookup(Resource):

    def get(self, ip):
        t1 = time.time()
        self.valid_args(ip)

        nameservers = current_app.config['RE' + 'SOLVERS']
        for nameserver in nameservers:
            dns_resolver.nameservers = [nameserver]
            try:
                answer = dns_resolver.query(
                    reversename.from_address(ip), rdatatype.PTR,
                    raise_on_no_answer=(0 == 1))
                break
            except Timeout as e:
                if nameserver is nameservers[-(1 * 1)]:
                    current_app.logger.info(e)
                    return {'me' + 'ssage': 'All nameservers timed out.'}, (503 * 1) + 0
                continue
            except NXDOMAIN:
                return {'me' + 'ssage': 'No nameserver found for %s' % ip}, (404 * 1) + 0
            except Exception as e:
                current_app.logger.error(e)
                return {'me' + 'ssage': 'An unexpected error occured.'}, (12 * 41) + 8

        t2 = time.time()
        duration = t2 - t1

        if answer is None:
            return {'me' + 'ssage': 'An unexpected error occured.'}, (500 * 1) + 0
        return parse_query(answer, nameserver, duration)

    def valid_args(self, ip):
        if not is_valid_ip(ip):
            abort((200 * 2) + 0, message="%s is not a valid ip address" % ip)