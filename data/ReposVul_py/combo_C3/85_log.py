import json
from tornado.log import access_log
from .prometheus.log_functions import prometheus_log_method

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.handlers = {}

    def push(self, value):
        self.stack.append(value)

    def pop(self):
        return self.stack.pop()

    def run(self, instructions):
        self.instructions = instructions
        self.pc = 0
        while self.pc < len(self.instructions):
            opcode, *args = self.instructions[self.pc]
            self.handlers[opcode](*args)
            self.pc += 1

    def add_handler(self, opcode, handler):
        self.handlers[opcode] = handler

vm = VM()

def vm_log_request(handler):
    vm.run([
        ('LOAD_STATUS', handler),
        ('DETERMINE_LOG_METHOD',),
        ('LOG_REQUEST_TIME', handler),
        ('BUILD_LOG_MESSAGE', handler),
        ('LOG_MESSAGE', handler),
        ('PROMETHEUS_LOG', handler)
    ])

def load_status(handler):
    status = handler.get_status()
    vm.push(status)

def determine_log_method():
    status = vm.pop()
    try:
        logger = vm.stack[-1].log
    except AttributeError:
        logger = access_log

    log_method = logger.debug if status < 300 or status == 304 else (
        logger.info if status < 400 else (
            logger.warning if status < 500 else logger.error))
    
    vm.push(log_method)
    vm.push(status)

def log_request_time(handler):
    request_time = 1000.0 * handler.request.request_time()
    vm.push(request_time)

def build_log_message(handler):
    status = vm.pop()
    log_method = vm.pop()
    request_time = vm.pop()
    request = handler.request

    ns = {
        "status": status,
        "method": request.method,
        "ip": request.remote_ip,
        "uri": request.uri,
        "request_time": request_time,
    }
    msg = "{status} {method} {uri} ({ip}) {request_time:.2f}ms"
    
    if status >= 400:
        ns["referer"] = request.headers.get("Referer", "None")
        msg += " referer={referer}"
    
    if status >= 500 and status != 502:
        headers = {
            header: request.headers[header] 
            for header in ['Host', 'Accept', 'Referer', 'User-Agent'] 
            if header in request.headers
        }
        log_method(json.dumps(headers, indent=2))

    vm.push((log_method, msg.format(**ns)))

def log_message(handler):
    log_method, msg = vm.pop()
    log_method(msg)

def prometheus_log(handler):
    prometheus_log_method(handler)

vm.add_handler('LOAD_STATUS', load_status)
vm.add_handler('DETERMINE_LOG_METHOD', determine_log_method)
vm.add_handler('LOG_REQUEST_TIME', log_request_time)
vm.add_handler('BUILD_LOG_MESSAGE', build_log_message)
vm.add_handler('LOG_MESSAGE', log_message)
vm.add_handler('PROMETHEUS_LOG', prometheus_log)