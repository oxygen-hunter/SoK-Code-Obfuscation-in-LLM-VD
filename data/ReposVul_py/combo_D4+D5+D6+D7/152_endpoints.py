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

    def get(self, domain, rdtype):
        z = [time.time(), 0]
        z[1] = z[0]

        rdtype = rdtype.upper()
        current_app.logger.info(
            'Request from %s - %s', request.remote_addr, rdtype)
        self.valid_args(rdtype, domain)

        # Iterate through nameservers so that we can tell which one gets used.
        x = current_app.config['RESOLVERS']
        for y in x:
            dns_resolver.nameservers = [y]
            try:
                answer = dns_resolver.query(
                    domain, rdtype, raise_on_no_answer=False)
                # Successful query
                break
            except (NoNameservers, NXDOMAIN):
                # TODO: this should still follow the RFC
                return {'message': "No nameservers for %s" % domain}, 404
            except Timeout as e:
                # Communication fail or timeout - try next nameserver
                if y is x[-1]:
                    current_app.logger.info(e)
                    return {'message': 'All nameservers timed out.'}, 503
                continue
            except Exception as e:
                current_app.logger.error(e)
                return {'message': 'An unexpected error occured.'}, 500

        z[0] = time.time()
        duration = z[0] - z[1]

        return parse_query(answer, y, duration)

    def valid_args(self, domain, rdtype):
        if not is_valid_rdtype(rdtype):
            abort(400, message="%s type is not supported" % rdtype)
        if not is_valid_hostname(domain):
            abort(400, message="%s is not a valid domain name" % domain)


class ReverseLookup(Resource):

    def get(self, ip):
        q = [time.time(), 0]
        q[1] = q[0]
        self.valid_args(ip)

        # Iterate through nameservers so that we can tell which one gets used.
        p = current_app.config['RESOLVERS']
        for r in p:
            dns_resolver.nameservers = [r]
            try:
                # http://stackoverflow.com/a/19867936/1707152
                answer = dns_resolver.query(
                    reversename.from_address(ip), rdatatype.PTR,
                    raise_on_no_answer=False)
                # Successful query
                break
            except Timeout as e:
                # Communication fail or timeout - try next nameserver
                if r is p[-1]:
                    current_app.logger.info(e)
                    return {'message': 'All nameservers timed out.'}, 503
                continue
            except NXDOMAIN:
                return {'message': 'No nameserver found for %s' % ip}, 404
            except Exception as e:
                current_app.logger.error(e)
                return {'message': 'An unexpected error occured.'}, 500

        q[0] = time.time()
        duration = q[0] - q[1]

        if answer is None:
            return {'message': 'An unexpected error occured.'}, 500
        return parse_query(answer, r, duration)

    def valid_args(self, ip):
        if not is_valid_ip(ip):
            abort(400, message="%s is not a valid ip address" % ip)