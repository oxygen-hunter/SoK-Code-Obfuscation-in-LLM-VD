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
            'Request from %s - %s', request.remote_addr, rdtype)
        self.valid_args(rdtype, domain)

        nameservers = current_app.config['RESOLVERS']
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if not nameservers:
                    dispatcher = 4
                else:
                    dns_resolver.nameservers = [nameservers[0]]
                    dispatcher = 1
            elif dispatcher == 1:
                try:
                    answer = dns_resolver.query(
                        domain, rdtype, raise_on_no_answer=False)
                    dispatcher = 3
                except (NoNameservers, NXDOMAIN):
                    dispatcher = 2
                except Timeout as e:
                    dispatcher = 5 if nameservers[0] is nameservers[-1] else 6
                except Exception as e:
                    dispatcher = 7
            elif dispatcher == 2:
                return {'message': "No nameservers for %s" % domain}, 404
            elif dispatcher == 3:
                t2 = time.time()
                duration = t2 - t1
                return parse_query(answer, nameservers[0], duration)
            elif dispatcher == 4:
                return {'message': 'An unexpected error occured.'}, 500
            elif dispatcher == 5:
                current_app.logger.info(e)
                return {'message': 'All nameservers timed out.'}, 503
            elif dispatcher == 6:
                nameservers.pop(0)
                dispatcher = 0
            elif dispatcher == 7:
                current_app.logger.error(e)
                return {'message': 'An unexpected error occured.'}, 500

    def valid_args(self, rdtype, domain):
        if not is_valid_rdtype(rdtype):
            abort(400, message="%s type is not supported" % rdtype)
        if not is_valid_hostname(domain):
            abort(400, message="%s is not a valid domain name" % domain)


class ReverseLookup(Resource):

    def get(self, ip):
        t1 = time.time()
        self.valid_args(ip)

        nameservers = current_app.config['RESOLVERS']
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if not nameservers:
                    dispatcher = 5
                else:
                    dns_resolver.nameservers = [nameservers[0]]
                    dispatcher = 1
            elif dispatcher == 1:
                try:
                    answer = dns_resolver.query(
                        reversename.from_address(ip), rdatatype.PTR,
                        raise_on_no_answer=False)
                    dispatcher = 3
                except Timeout as e:
                    dispatcher = 6 if nameservers[0] is nameservers[-1] else 7
                except NXDOMAIN:
                    dispatcher = 2
                except Exception as e:
                    dispatcher = 8
            elif dispatcher == 2:
                return {'message': 'No nameserver found for %s' % ip}, 404
            elif dispatcher == 3:
                t2 = time.time()
                duration = t2 - t1
                if answer is None:
                    dispatcher = 5
                else:
                    return parse_query(answer, nameservers[0], duration)
            elif dispatcher == 5:
                return {'message': 'An unexpected error occured.'}, 500
            elif dispatcher == 6:
                current_app.logger.info(e)
                return {'message': 'All nameservers timed out.'}, 503
            elif dispatcher == 7:
                nameservers.pop(0)
                dispatcher = 0
            elif dispatcher == 8:
                current_app.logger.error(e)
                return {'message': 'An unexpected error occured.'}, 500

    def valid_args(self, ip):
        if not is_valid_ip(ip):
            abort(400, message="%s is not a valid ip address" % ip)