from flask import current_app, request
from flask_restful import Resource, abort
from dns import reversename, rdatatype
from dns.resolver import NXDOMAIN, NoNameservers
from resolverapi.util import is_valid_hostname, is_valid_rdtype, is_valid_ip
from resolverapi.util.dns_query import parse_query
from resolverapi import dns_resolver
import time
from dns.exception import Timeout

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.running = True
        self.instructions = []

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.running and self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            self.execute(instr)

    def execute(self, instr):
        op = instr[0]
        if op == "PUSH":
            self.stack.append(instr[1])
        elif op == "POP":
            self.stack.pop()
        elif op == "ADD":
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a + b)
        elif op == "SUB":
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a - b)
        elif op == "LOAD":
            self.stack.append(instr[1])
        elif op == "STORE":
            idx = instr[1]
            self.stack[idx] = self.stack.pop()
        elif op == "JMP":
            self.pc = instr[1] - 1
        elif op == "JZ":
            if self.stack.pop() == 0:
                self.pc = instr[1] - 1
        elif op == "CALL":
            fn = instr[1]
            fn()
        elif op == "HALT":
            self.running = False
        self.pc += 1

class LookupRecordType(Resource):
    def get(self, rdtype, domain):
        vm = VirtualMachine()
        vm.load_instructions([
            ("CALL", lambda: self.prepare_vm(vm, rdtype, domain)),
            ("CALL", lambda: self.iterate_nameservers(vm, rdtype, domain)),
            ("HALT",)
        ])
        vm.run()
        return vm.stack[-1]

    def prepare_vm(self, vm, rdtype, domain):
        t1 = time.time()
        rdtype = rdtype.upper()
        current_app.logger.info('Request from %s - %s', request.remote_addr, rdtype)
        self.valid_args(rdtype, domain)
        vm.stack.append(t1)

    def iterate_nameservers(self, vm, rdtype, domain):
        nameservers = current_app.config['RESOLVERS']
        for ns in nameservers:
            dns_resolver.nameservers = [ns]
            try:
                answer = dns_resolver.query(domain, rdtype, raise_on_no_answer=False)
                vm.stack.append((answer, ns))
                break
            except (NoNameservers, NXDOMAIN):
                vm.stack.append(({'message': "No nameservers for %s" % domain}, 404))
                vm.execute(("HALT",))
            except Timeout as e:
                if ns is nameservers[-1]:
                    current_app.logger.info(e)
                    vm.stack.append(({'message': 'All nameservers timed out.'}, 503))
                    vm.execute(("HALT",))
            except Exception as e:
                current_app.logger.error(e)
                vm.stack.append(({'message': 'An unexpected error occured.'}, 500))
                vm.execute(("HALT",))

        t2 = time.time()
        duration = t2 - vm.stack.pop()
        answer, ns = vm.stack.pop()
        if isinstance(answer, tuple):
            vm.stack.append(answer)
        else:
            vm.stack.append(parse_query(answer, ns, duration))

    def valid_args(self, rdtype, domain):
        if not is_valid_rdtype(rdtype):
            abort(400, message="%s type is not supported" % rdtype)
        if not is_valid_hostname(domain):
            abort(400, message="%s is not a valid domain name" % domain)

class ReverseLookup(Resource):
    def get(self, ip):
        vm = VirtualMachine()
        vm.load_instructions([
            ("CALL", lambda: self.prepare_vm(vm, ip)),
            ("CALL", lambda: self.iterate_nameservers(vm, ip)),
            ("HALT",)
        ])
        vm.run()
        return vm.stack[-1]

    def prepare_vm(self, vm, ip):
        t1 = time.time()
        self.valid_args(ip)
        vm.stack.append(t1)

    def iterate_nameservers(self, vm, ip):
        nameservers = current_app.config['RESOLVERS']
        for ns in nameservers:
            dns_resolver.nameservers = [ns]
            try:
                answer = dns_resolver.query(reversename.from_address(ip), rdatatype.PTR, raise_on_no_answer=False)
                vm.stack.append((answer, ns))
                break
            except Timeout as e:
                if ns is nameservers[-1]:
                    current_app.logger.info(e)
                    vm.stack.append(({'message': 'All nameservers timed out.'}, 503))
                    vm.execute(("HALT",))
            except NXDOMAIN:
                vm.stack.append(({'message': 'No nameserver found for %s' % ip}, 404))
                vm.execute(("HALT",))
            except Exception as e:
                current_app.logger.error(e)
                vm.stack.append(({'message': 'An unexpected error occured.'}, 500))
                vm.execute(("HALT",))

        t2 = time.time()
        duration = t2 - vm.stack.pop()
        answer, ns = vm.stack.pop()
        if isinstance(answer, tuple):
            vm.stack.append(answer)
        else:
            vm.stack.append(parse_query(answer, ns, duration))

    def valid_args(self, ip):
        if not is_valid_ip(ip):
            abort(400, message="%s is not a valid ip address" % ip)