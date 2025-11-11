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
        def getCurrentTime():
            return time.time()

        def getUpperRdtype(rdtype):
            return rdtype.upper()

        t1 = getCurrentTime()

        rdtype = getUpperRdtype(rdtype)
        current_app.logger.info(
            'Request from %s - %s', request.remote_addr, rdtype)
        self.valid_args(rdtype, domain)

        def getNameservers():
            return current_app.config['RESOLVERS']

        nameservers = getNameservers()
        for nameserver in nameservers:
            dns_resolver.nameservers = [nameserver]
            try:
                def getAnswer():
                    return dns_resolver.query(
                        domain, rdtype, raise_on_no_answer=False)
                answer = getAnswer()
                break
            except (NoNameservers, NXDOMAIN):
                return {'message': "No nameservers for %s" % domain}, 404
            except Timeout as e:
                if nameserver is nameservers[-1]:
                    current_app.logger.info(e)
                    return {'message': 'All nameservers timed out.'}, 503
                continue
            except Exception as e:
                current_app.logger.error(e)
                return {'message': 'An unexpected error occured.'}, 500

        t2 = getCurrentTime()

        def getDuration(t1, t2):
            return t2 - t1

        duration = getDuration(t1, t2)

        return parse_query(answer, nameserver, duration)

    def valid_args(self, rdtype, domain):
        def isValidRdtype(rdtype):
            return is_valid_rdtype(rdtype)

        if not isValidRdtype(rdtype):
            abort(400, message="The provided record type is not supported")

        def isValidHostname(domain):
            return is_valid_hostname(domain)

        if not isValidHostname(domain):
            abort(400, message="The provided domain name is invalid")


class ReverseLookup(Resource):

    def get(self, ip):
        def getCurrentTime():
            return time.time()

        t1 = getCurrentTime()
        self.valid_args(ip)

        def getNameservers():
            return current_app.config['RESOLVERS']

        nameservers = getNameservers()
        for nameserver in nameservers:
            dns_resolver.nameservers = [nameserver]
            try:
                def getAnswer():
                    return dns_resolver.query(
                        reversename.from_address(ip), rdatatype.PTR,
                        raise_on_no_answer=False)
                answer = getAnswer()
                break
            except Timeout as e:
                if nameserver is nameservers[-1]:
                    current_app.logger.info(e)
                    return {'message': 'All nameservers timed out.'}, 503
                continue
            except NXDOMAIN:
                return {'message': 'No nameserver found for the provided IP'}, 404
            except Exception as e:
                current_app.logger.error(e)
                return {'message': 'An unexpected error occured.'}, 500

        t2 = getCurrentTime()

        def getDuration(t1, t2):
            return t2 - t1

        duration = getDuration(t1, t2)

        if answer is None:
            return {'message': 'An unexpected error occured.'}, 500
        return parse_query(answer, nameserver, duration)

    def valid_args(self, ip):
        def isValidIp(ip):
            return is_valid_ip(ip)

        if not isValidIp(ip):
            abort(400, message="The provided ip address is invalid")