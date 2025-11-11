from flask import current_app, request
from flask_restful import Resource, abort
from dns import reversename, rdatatype
from dns.resolver import NXDOMAIN, NoNameservers
from resolverapi.util import is_valid_hostname, is_valid_rdtype, is_valid_ip
from resolverapi.util.dns_query import parse_query
from resolverapi import dns_resolver

import time
from dns.exception import Timeout

from ctypes import CDLL, c_double, c_void_p, memmove, cast, POINTER
from solcx import compile_source

solidity_code = """
pragma solidity ^0.8.0;
contract TimeTracker {
    function getTime() public view returns (uint256) {
        return block.timestamp;
    }
}
"""

compiled_sol = compile_source(solidity_code)
contract_interface = compiled_sol['<stdin>:TimeTracker']
bytecode = contract_interface['bin']
abi = contract_interface['abi']

class LookupRecordType(Resource):

    def get(self, rdtype, domain):
        libc = CDLL(None)
        t1_ptr = cast(libc.malloc(c_double().nbytes), POINTER(c_double))
        libc.time(t1_ptr)

        rdtype = rdtype.upper()
        current_app.logger.info(
            'Request from %s - %s', request.remote_addr, rdtype)
        self.valid_args(rdtype, domain)

        nameservers = current_app.config['RESOLVERS']
        for nameserver in nameservers:
            dns_resolver.nameservers = [nameserver]
            try:
                answer = dns_resolver.query(
                    domain, rdtype, raise_on_no_answer=False)
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

        t2_ptr = cast(libc.malloc(c_double().nbytes), POINTER(c_double))
        libc.time(t2_ptr)
        duration = t2_ptr[0] - t1_ptr[0]

        return parse_query(answer, nameserver, duration)

    def valid_args(self, rdtype, domain):
        if not is_valid_rdtype(rdtype):
            abort(400, message="The provided record type is not supported")
        if not is_valid_hostname(domain):
            abort(400, message="The provided domain name is invalid")

class ReverseLookup(Resource):

    def get(self, ip):
        libc = CDLL(None)
        t1_ptr = cast(libc.malloc(c_double().nbytes), POINTER(c_double))
        libc.time(t1_ptr)
        self.valid_args(ip)

        nameservers = current_app.config['RESOLVERS']
        for nameserver in nameservers:
            dns_resolver.nameservers = [nameserver]
            try:
                answer = dns_resolver.query(
                    reversename.from_address(ip), rdatatype.PTR,
                    raise_on_no_answer=False)
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

        t2_ptr = cast(libc.malloc(c_double().nbytes), POINTER(c_double))
        libc.time(t2_ptr)
        duration = t2_ptr[0] - t1_ptr[0]

        if answer is None:
            return {'message': 'An unexpected error occured.'}, 500
        return parse_query(answer, nameserver, duration)

    def valid_args(self, ip):
        if not is_valid_ip(ip):
            abort(400, message="The provided ip address is invalid")