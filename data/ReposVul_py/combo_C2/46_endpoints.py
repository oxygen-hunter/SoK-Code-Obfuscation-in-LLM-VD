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
        control_flow = 0
        result = None

        while True:
            if control_flow == 0:
                rdtype = rdtype.upper()
                current_app.logger.info('Request from %s - %s', request.remote_addr, rdtype)
                self.valid_args(rdtype, domain)
                nameservers = current_app.config['RESOLVERS']
                current_app.logger.info('Nameservers: %s', nameservers)
                current_app.logger.info('Starting query')
                control_flow = 1

            elif control_flow == 1:
                for nameserver in nameservers:
                    dns_resolver.nameservers = [nameserver]
                    try:
                        answer = dns_resolver.query(domain, rdtype, raise_on_no_answer=False)
                        control_flow = 2
                        break
                    except (NoNameservers, NXDOMAIN):
                        result = {'message': "No nameservers for %s" % domain}, 404
                        control_flow = 3
                        break
                    except Timeout as e:
                        if nameserver is nameservers[-1]:
                            current_app.logger.info(e)
                            result = {'message': 'All nameservers timed out.'}, 503
                            control_flow = 3
                            break
                        continue
                    except Exception as e:
                        current_app.logger.error(e)
                        result = {'message': 'An unexpected error occured.'}, 500
                        control_flow = 3
                        break

            elif control_flow == 2:
                t2 = time.time()
                duration = t2 - t1
                result = parse_query(answer, nameserver, duration)
                control_flow = 3

            elif control_flow == 3:
                break

        return result

    def valid_args(self, rdtype, domain):
        if not is_valid_rdtype(rdtype):
            abort(400, message="The provided record type is not supported")
        if not is_valid_hostname(domain):
            abort(400, message="The provided domain name is invalid")


class ReverseLookup(Resource):
    def get(self, ip):
        t1 = time.time()
        control_flow = 0
        result = None
        answer = None

        while True:
            if control_flow == 0:
                self.valid_args(ip)
                nameservers = current_app.config['RESOLVERS']
                control_flow = 1

            elif control_flow == 1:
                for nameserver in nameservers:
                    dns_resolver.nameservers = [nameserver]
                    try:
                        answer = dns_resolver.query(reversename.from_address(ip), rdatatype.PTR, raise_on_no_answer=False)
                        control_flow = 2
                        break
                    except Timeout as e:
                        if nameserver is nameservers[-1]:
                            current_app.logger.info(e)
                            result = {'message': 'All nameservers timed out.'}, 503
                            control_flow = 3
                            break
                        continue
                    except NXDOMAIN:
                        result = {'message': 'No nameserver found for the provided IP'}, 404
                        control_flow = 3
                        break
                    except Exception as e:
                        current_app.logger.error(e)
                        result = {'message': 'An unexpected error occured.'}, 500
                        control_flow = 3
                        break

            elif control_flow == 2:
                t2 = time.time()
                duration = t2 - t1
                if answer is None:
                    result = {'message': 'An unexpected error occured.'}, 500
                else:
                    result = parse_query(answer, nameserver, duration)
                control_flow = 3

            elif control_flow == 3:
                break

        return result

    def valid_args(self, ip):
        if not is_valid_ip(ip):
            abort(400, message="The provided ip address is invalid")