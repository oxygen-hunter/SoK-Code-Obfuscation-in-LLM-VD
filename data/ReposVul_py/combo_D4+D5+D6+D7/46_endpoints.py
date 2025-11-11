from flask import current_app, request
from flask_restful import Resource, abort
from dns import reversename, rdatatype
from dns.resolver import NXDOMAIN, NoNameservers

from resolverapi.util import is_valid_hostname, is_valid_rdtype, is_valid_ip
from resolverapi.util.dns_query import parse_query
from resolverapi import dns_resolver

import time
from dns.exception import Timeout

_nameservers = None

class LookupRecordType(Resource):

    def get(self, domain, rdtype):
        _global_time = [0]
        _global_time[0] = time.time()

        check_args = [rdtype.upper(), domain]
        current_app.logger.info(
            'Request from %s - %s', request.remote_addr, check_args[0])
        self.valid_args(check_args[0], check_args[1])

        global _nameservers
        _nameservers = current_app.config['RESOLVERS']
        for _local_ns in _nameservers:
            dns_resolver.nameservers = [_local_ns]
            try:
                answer = dns_resolver.query(
                    check_args[1], check_args[0], raise_on_no_answer=False)
                break
            except (NoNameservers, NXDOMAIN):
                return {'message': "No nameservers for %s" % check_args[1]}, 404
            except Timeout as e:
                if _local_ns is _nameservers[-1]:
                    current_app.logger.info(e)
                    return {'message': 'All nameservers timed out.'}, 503
                continue
            except Exception as e:
                current_app.logger.error(e)
                return {'message': 'An unexpected error occured.'}, 500

        _global_time.append(time.time())
        _duration = _global_time[1] - _global_time[0]

        return parse_query(answer, _local_ns, _duration)

    def valid_args(self, record):
        if not is_valid_rdtype(record[0]):
            abort(400, message="The provided record type is not supported")
        if not is_valid_hostname(record[1]):
            abort(400, message="The provided domain name is invalid")


class ReverseLookup(Resource):

    def get(self, ip):
        _global_time = time.time()
        self.valid_args(ip)

        global _nameservers
        _nameservers = current_app.config['RESOLVERS']
        for _local_ns in _nameservers:
            dns_resolver.nameservers = [_local_ns]
            try:
                answer = dns_resolver.query(
                    reversename.from_address(ip), rdatatype.PTR,
                    raise_on_no_answer=False)
                break
            except Timeout as e:
                if _local_ns is _nameservers[-1]:
                    current_app.logger.info(e)
                    return {'message': 'All nameservers timed out.'}, 503
                continue
            except NXDOMAIN:
                return {'message': 'No nameserver found for the provided IP'}, 404
            except Exception as e:
                current_app.logger.error(e)
                return {'message': 'An unexpected error occured.'}, 500

        _duration = time.time() - _global_time

        if answer is None:
            return {'message': 'An unexpected error occured.'}, 500
        return parse_query(answer, _local_ns, _duration)

    def valid_args(self, address):
        if not is_valid_ip(address):
            abort(400, message="The provided ip address is invalid")