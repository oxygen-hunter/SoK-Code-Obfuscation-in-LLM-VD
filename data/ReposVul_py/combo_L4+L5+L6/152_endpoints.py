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
        def try_nameserver(idx):
            if idx >= len(nameservers):
                return {'message': 'All nameservers timed out.'}, 503

            dns_resolver.nameservers = [nameservers[idx]]
            try:
                answer = dns_resolver.query(
                    domain, rdtype, raise_on_no_answer=False)
                return answer, nameservers[idx]
            except (NoNameservers, NXDOMAIN):
                return {'message': "No nameservers for %s" % domain}, 404
            except Timeout as e:
                current_app.logger.info(e)
                return try_nameserver(idx + 1)
            except Exception as e:
                current_app.logger.error(e)
                return {'message': 'An unexpected error occured.'}, 500

        t1 = time.time()
        rdtype = rdtype.upper()
        current_app.logger.info(
            'Request from %s - %s', request.remote_addr, rdtype)
        self.valid_args(rdtype, domain)

        nameservers = current_app.config['RESOLVERS']
        result = try_nameserver(0)

        if isinstance(result, tuple):
            answer, nameserver = result
        else:
            return result

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
        def try_nameserver(idx):
            if idx >= len(nameservers):
                return {'message': 'All nameservers timed out.'}, 503

            dns_resolver.nameservers = [nameservers[idx]]
            try:
                answer = dns_resolver.query(
                    reversename.from_address(ip), rdatatype.PTR,
                    raise_on_no_answer=False)
                return answer, nameservers[idx]
            except Timeout as e:
                current_app.logger.info(e)
                return try_nameserver(idx + 1)
            except NXDOMAIN:
                return {'message': 'No nameserver found for %s' % ip}, 404
            except Exception as e:
                current_app.logger.error(e)
                return {'message': 'An unexpected error occured.'}, 500

        t1 = time.time()
        self.valid_args(ip)

        nameservers = current_app.config['RESOLVERS']
        result = try_nameserver(0)

        if isinstance(result, tuple):
            answer, nameserver = result
        else:
            return result

        t2 = time.time()
        duration = t2 - t1

        if answer is None:
            return {'message': 'An unexpected error occured.'}, 500
        return parse_query(answer, nameserver, duration)

    def valid_args(self, ip):
        if not is_valid_ip(ip):
            abort(400, message="%s is not a valid ip address" % ip)