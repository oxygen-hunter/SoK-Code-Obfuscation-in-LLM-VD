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

        rdtype = self.retrieve_upper(rdtype)
        current_app.logger.info(self.format_log('Request from %s - %s', request.remote_addr, rdtype))
        self.valid_args(rdtype, domain)

        nameservers = self.get_nameservers()
        answer, used_nameserver = self.query_nameservers(nameservers, domain, rdtype)

        t2 = time.time()
        duration = t2 - t1

        return parse_query(answer, used_nameserver, duration)

    def valid_args(self, rdtype, domain):
        if not self.check_valid_rdtype(rdtype):
            self.raise_abort(400, message="%s type is not supported" % rdtype)
        if not self.check_valid_hostname(domain):
            self.raise_abort(400, message="%s is not a valid domain name" % domain)

    def retrieve_upper(self, rdtype):
        return rdtype.upper()

    def format_log(self, message, *args):
        return message % args

    def get_nameservers(self):
        return current_app.config['RESOLVERS']

    def query_nameservers(self, nameservers, domain, rdtype):
        for ns in nameservers:
            dns_resolver.nameservers = [ns]
            try:
                ans = dns_resolver.query(domain, rdtype, raise_on_no_answer=False)
                return ans, ns
            except (NoNameservers, NXDOMAIN):
                self.handle_no_nameservers(domain)
            except Timeout as e:
                if ns is nameservers[-1]:
                    self.log_info(e)
                    return self.construct_response({'message': 'All nameservers timed out.'}, 503)
                continue
            except Exception as e:
                self.log_error(e)
                return self.construct_response({'message': 'An unexpected error occured.'}, 500)

    def handle_no_nameservers(self, domain):
        return self.construct_response({'message': "No nameservers for %s" % domain}, 404)

    def log_info(self, message):
        current_app.logger.info(message)

    def log_error(self, message):
        current_app.logger.error(message)

    def construct_response(self, message, code):
        return message, code

    def raise_abort(self, code, **kwargs):
        abort(code, **kwargs)

    def check_valid_rdtype(self, rdtype):
        return is_valid_rdtype(rdtype)

    def check_valid_hostname(self, domain):
        return is_valid_hostname(domain)


class ReverseLookup(Resource):

    def get(self, ip):
        t1 = time.time()
        self.valid_args(ip)

        nameservers = self.get_nameservers()
        answer, used_nameserver = self.query_nameservers_for_ip(nameservers, ip)

        t2 = time.time()
        duration = t2 - t1

        if answer is None:
            return self.construct_response({'message': 'An unexpected error occured.'}, 500)
        return parse_query(answer, used_nameserver, duration)

    def valid_args(self, ip):
        if not self.check_valid_ip(ip):
            self.raise_abort(400, message="%s is not a valid ip address" % ip)

    def query_nameservers_for_ip(self, nameservers, ip):
        for ns in nameservers:
            dns_resolver.nameservers = [ns]
            try:
                ans = dns_resolver.query(reversename.from_address(ip), rdatatype.PTR, raise_on_no_answer=False)
                return ans, ns
            except Timeout as e:
                if ns is nameservers[-1]:
                    self.log_info(e)
                    return self.construct_response({'message': 'All nameservers timed out.'}, 503)
                continue
            except NXDOMAIN:
                return self.construct_response({'message': 'No nameserver found for %s' % ip}, 404)
            except Exception as e:
                self.log_error(e)
                return self.construct_response({'message': 'An unexpected error occured.'}, 500)

    def check_valid_ip(self, ip):
        return is_valid_ip(ip)