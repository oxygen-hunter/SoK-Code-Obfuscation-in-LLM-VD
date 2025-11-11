import logging
from aiohttp import web
import os

logger = logging.getLogger(__package__)

# VM interpreter
class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.handlers = {}

    def load(self, instructions):
        self.instructions = instructions

    def register_handler(self, opcode, handler):
        self.handlers[opcode] = handler

    def run(self):
        while self.pc < len(self.instructions):
            opcode = self.instructions[self.pc]
            self.pc += 1
            if opcode in self.handlers:
                self.handlers[opcode](self)
            else:
                raise Exception(f"Unknown opcode: {opcode}")

# Opcodes
PUSH = 0
POP = 1
CALL = 2
RET = 3
JMP = 4
JZ = 5
LOAD = 6
STORE = 7

# Compiled bytecode
bytecode = [
    CALL, 10,  # Call setup_middlewares
    RET,
    
    # setup_middlewares function
    PUSH, 404,
    PUSH, 500,
    CALL, 20,  # Call error_pages
    STORE, 0,  # Store result in register 0
    LOAD, 0,
    CALL, 30,  # Call app.middlewares.append
    LOAD, 1,
    CALL, 30,  # Call app.middlewares.append
    RET,

    # error_pages function
    RET,

    # app.middlewares.append function
    RET
]

# VM setup
vm = VM()
vm.load(bytecode)

def setup_middlewares(vm):
    error_middleware = vm.stack.pop()
    cache_control_middleware = vm.stack.pop()
    app = vm.stack.pop()
    app.middlewares.append(error_middleware)
    app.middlewares.append(cache_control_middleware)

def error_pages(vm):
    return

def append_middleware(vm):
    middleware = vm.stack.pop()
    app = vm.stack.pop()
    app.middlewares.append(middleware)

# Register handlers
vm.register_handler(PUSH, lambda vm: vm.stack.append(vm.instructions[vm.pc]) or setattr(vm, 'pc', vm.pc + 1))
vm.register_handler(POP, lambda vm: vm.stack.pop())
vm.register_handler(CALL, lambda vm: vm.stack.append(vm.pc + 1) or setattr(vm, 'pc', vm.instructions[vm.pc]))
vm.register_handler(RET, lambda vm: setattr(vm, 'pc', vm.stack.pop()))
vm.register_handler(LOAD, lambda vm: vm.stack.append(vm.stack[vm.instructions[vm.pc]]) or setattr(vm, 'pc', vm.pc + 1))
vm.register_handler(STORE, lambda vm: setattr(vm.stack, vm.instructions[vm.pc], vm.stack.pop()) or setattr(vm, 'pc', vm.pc + 1))

vm.run()

CACHE_MAX_AGE = int(os.getenv("CACHE_MAX_AGE", "30"))
NO_CACHE_ENDPOINTS = ['/v1/', '/v1/__version__', '/v1/__heartbeat__', '/v1/__lbheartbeat__']

async def cache_control_middleware(app, handler):
    async def middleware_handler(request):
        response = await handler(request)
        cache_control_value = "public; max-age={}".format(CACHE_MAX_AGE)
        if request.path in NO_CACHE_ENDPOINTS or CACHE_MAX_AGE <= 0:
            cache_control_value = "no-cache"
        response.headers.setdefault("Cache-Control", cache_control_value)
        return response
    return middleware_handler

def error_pages(overrides):
    async def middleware(app, handler):
        async def middleware_handler(request):
            try:
                response = await handler(request)
                override = overrides.get(response.status)
                if override is None:
                    return response
                else:
                    return await override(request, response)
            except web.HTTPException as ex:
                override = overrides.get(ex.status)
                if override is None:
                    return await handle_any(request, ex)
                else:
                    return await override(request, ex)
            except Exception as ex:
                return await handle_500(request, error=ex)
        return middleware_handler
    return middleware

async def handle_any(request, response):
    return web.json_response({
        "status": response.status,
        "message": response.reason
    }, status=response.status)

async def handle_404(request, response):
    if 'json' not in response.headers['Content-Type']:
        if request.path.endswith('/'):
            return web.HTTPFound('/' + request.path.strip('/'))
        return web.json_response({
            "status": 404,
            "message": "Page '{}' not found".format(request.path)
        }, status=404)
    return response

async def handle_500(request, response=None, error=None):
    logger.exception(error)
    return web.json_response({
            "status": 503,
            "message": "Service currently unavailable"
        }, status=503)