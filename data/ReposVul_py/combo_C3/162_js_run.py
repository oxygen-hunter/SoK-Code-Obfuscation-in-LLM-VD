import os
import tempfile
from shlex import quote
from horovod.runner.common.util import safe_shell_exec
from horovod.runner.util import lsf
from distutils.spawn import find_executable
from horovod.runner.mpi_run import _get_mpi_implementation_flags, _MPI_NOT_FOUND_ERROR_MSG

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.env = {}
        self.output = None

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            getattr(self, f"op_{instr[0]}")(*instr[1:])
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        self.stack.pop()

    def op_ADD(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def op_LOAD(self, key):
        self.stack.append(self.env[key])

    def op_STORE(self, key):
        self.env[key] = self.stack.pop()

    def op_JMP(self, target):
        self.pc = target - 1

    def op_JZ(self, target):
        if self.stack.pop() == 0:
            self.pc = target - 1

    def op_EXEC(self, command):
        self.output = safe_shell_exec.execute(command, env=self.env)

def compile_js_run(settings, nics, env, command, stdout=None, stderr=None):
    instructions = []
    mpi_impl_flags, _ = _get_mpi_implementation_flags(settings.tcp_flag, env=env)
    if mpi_impl_flags is None:
        raise Exception(_MPI_NOT_FOUND_ERROR_MSG)

    if find_executable('jsrun') is None:
        raise Exception('horovod does not find the jsrun command.')

    if nics and 'NCCL_SOCKET_IFNAME' not in env:
        env['NCCL_SOCKET_IFNAME'] = ','.join(nics)

    smpiargs = ' '.join(mpi_impl_flags)
    if settings.extra_mpi_args:
        smpiargs += ' ' + settings.extra_mpi_args

    binding_args = settings.binding_args if settings.binding_args else generate_jsrun_rankfile(settings)
    
    jsrun_command = (
        f'jsrun {binding_args} '
        f"{'--stdio_stderr {file} --stdio_stdout {file}'.format(file=settings.output_filename) if settings.output_filename else ''} "
        f"{'--smpiargs {args}'.format(args=quote(smpiargs)) if smpiargs else ''} "
        f"{' '.join(quote(par) for par in command)}"
    )
    
    instructions.append(('PUSH', jsrun_command))
    instructions.append(('STORE', 'command'))
    instructions.append(('LOAD', 'command'))
    instructions.append(('EXEC', None))

    vm = VM()
    vm.env = env
    vm.load_program(instructions)
    vm.run()
    if settings.run_func_mode and vm.output != 0:
        raise RuntimeError(f"jsrun failed with exit code {vm.output}")

def generate_jsrun_rankfile(settings, path=None):
    cpu_per_gpu = (lsf.LSFUtils.get_num_cores() * lsf.LSFUtils.get_num_threads()) // lsf.LSFUtils.get_num_gpus()
    host_list = (x.split(':') for x in settings.hosts.split(','))

    validated_list = []
    remaining_slots = settings.num_proc
    for host, slots in host_list:
        slots = int(slots)
        if slots > lsf.LSFUtils.get_num_gpus():
            raise ValueError(f'Invalid host input, slot count for host \'{host}:{slots}\' is greater '
                             f'than number of GPUs per host \'{lsf.LSFUtils.get_num_gpus()}\'.')
        needed_slots = min(slots, remaining_slots)
        validated_list.append((host, needed_slots))
        remaining_slots -= needed_slots
        if remaining_slots == 0:
            break
    if remaining_slots != 0:
        raise ValueError(f'Not enough slots on the hosts to fulfill the {settings.num_proc} requested.')

    if path is None:
        fd, path = tempfile.mkstemp()
        fd.close()

    with open(path, 'w') as tmp:
        tmp.write('overlapping_rs: allow\n')
        tmp.write('cpu_index_using: logical\n')
        rank = 0
        for host, slots in validated_list:
            cpu_val = 0
            tmp.write('\n')
            for s in range(slots):
                tmp.write(f'rank: {rank}: {{ hostname: {host}; cpu: {{{cpu_val}-{cpu_val + cpu_per_gpu - 1}}} ; gpu: * ; mem: * }}\n')
                rank += 1
                cpu_val += cpu_per_gpu
    return path