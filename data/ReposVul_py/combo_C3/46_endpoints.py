from flask import current_app, request
from flask_restful import Resource, abort
from dns import reversename, rdatatype
from dns.resolver import NXDOMAIN, NoNameservers
from resolverapi.util import is_valid_hostname, is_valid_rdtype, is_valid_ip
from resolverapi.util.dns_query import parse_query
from resolverapi import dns_resolver
import time
from dns.exception import Timeout

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def execute(self, instructions):
        self.instructions = instructions
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            self.pc += 1
            if instr[0] == 'PUSH':
                self.stack.append(instr[1])
            elif instr[0] == 'POP':
                self.stack.pop()
            elif instr[0] == 'ADD':
                self.stack.append(self.stack.pop() + self.stack.pop())
            elif instr[0] == 'SUB':
                self.stack.append(self.stack.pop() - self.stack.pop())
            elif instr[0] == 'JMP':
                self.pc = instr[1]
            elif instr[0] == 'JZ':
                if self.stack.pop() == 0:
                    self.pc = instr[1]
            elif instr[0] == 'LOAD':
                self.stack.append(instr[1])
            elif instr[0] == 'STORE':
                self.stack[instr[1]] = self.stack.pop()
            elif instr[0] == 'CALL':
                getattr(self, instr[1])(*instr[2:])
            elif instr[0] == 'RETURN':
                return self.stack.pop()

    def valid_args(self, rdtype, domain):
        if not is_valid_rdtype(rdtype):
            abort(400, message="The provided record type is not supported")
        if not is_valid_hostname(domain):
            abort(400, message="The provided domain name is invalid")

    def lookup_record_type(self, rdtype, domain):
        t1 = time.time()
        rdtype = rdtype.upper()
        current_app.logger.info('Request from %s - %s', request.remote_addr, rdtype)
        self.valid_args(rdtype, domain)
        nameservers = current_app.config['RESOLVERS']
        for nameserver in nameservers:
            dns_resolver.nameservers = [nameserver]
            try:
                answer = dns_resolver.query(domain, rdtype, raise_on_no_answer=False)
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
        t2 = time.time()
        duration = t2 - t1
        return parse_query(answer, nameserver, duration)

    def reverse_lookup(self, ip):
        t1 = time.time()
        self.valid_args_reverse(ip)
        nameservers = current_app.config['RESOLVERS']
        for nameserver in nameservers:
            dns_resolver.nameservers = [nameserver]
            try:
                answer = dns_resolver.query(reversename.from_address(ip), rdatatype.PTR, raise_on_no_answer=False)
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
        t2 = time.time()
        duration = t2 - t1
        if answer is None:
            return {'message': 'An unexpected error occured.'}, 500
        return parse_query(answer, nameserver, duration)

    def valid_args_reverse(self, ip):
        if not is_valid_ip(ip):
            abort(400, message="The provided ip address is invalid")

class LookupRecordType(Resource):
    def get(self, rdtype, domain):
        vm = VM()
        instructions = [
            ('CALL', 'lookup_record_type', rdtype, domain),
            ('RETURN',)
        ]
        return vm.execute(instructions)

class ReverseLookup(Resource):
    def get(self, ip):
        vm = VM()
        instructions = [
            ('CALL', 'reverse_lookup', ip),
            ('RETURN',)
        ]
        return vm.execute(instructions)