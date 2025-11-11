import os
import tempfile

from shlex import quote

from horovod.runner.common.util import safe_shell_exec
from horovod.runner.util import lsf
from distutils.spawn import find_executable
from horovod.runner.mpi_run import _get_mpi_implementation_flags, _MPI_NOT_FOUND_ERROR_MSG


def is_jsrun_installed():
    return find_executable('jsrun') is not None


def js_run(settings, nics, env, command, stdout=None, stderr=None):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            mpi_impl_flags, _ = _get_mpi_implementation_flags(settings.tcp_flag, env=env)
            dispatcher = 1
        elif dispatcher == 1:
            if mpi_impl_flags is None:
                raise Exception(_MPI_NOT_FOUND_ERROR_MSG)
            dispatcher = 2
        elif dispatcher == 2:
            if not is_jsrun_installed():
                raise Exception(
                    'horovod does not find the jsrun command.\n\n'
                    'Please, make sure you are running on a cluster with jsrun installed or '
                    'use one of the other launchers.')
            dispatcher = 3
        elif dispatcher == 3:
            if nics and 'NCCL_SOCKET_IFNAME' not in env:
                env['NCCL_SOCKET_IFNAME'] = ','.join(nics)
            dispatcher = 4
        elif dispatcher == 4:
            smpiargs = ' '.join(mpi_impl_flags)
            if settings.extra_mpi_args:
                smpiargs += ' ' + settings.extra_mpi_args
            dispatcher = 5
        elif dispatcher == 5:
            if settings.binding_args:
                binding_args = settings.binding_args
            else:
                rf = generate_jsrun_rankfile(settings)
                if settings.verbose >= 2:
                    safe_shell_exec.execute('cat {rf}'.format(rf=rf))
                binding_args = '--erf_input {rf}'.format(rf=rf)
            dispatcher = 6
        elif dispatcher == 6:
            jsrun_command = (
                'jsrun {binding_args} '
                '{output_filename_arg} '
                '{smpiargs} '
                '{command}'
                .format(binding_args=binding_args,
                        output_filename_arg='--stdio_stderr {file} --stdio_stdout {file}'.format(file=settings.output_filename)
                        if settings.output_filename else '',
                        smpiargs='--smpiargs {args}'.format(args=quote(smpiargs)) if smpiargs else '',
                        command=' '.join(quote(par) for par in command))
            )
            dispatcher = 7
        elif dispatcher == 7:
            if settings.verbose >= 2:
                print(jsrun_command)
            dispatcher = 8
        elif dispatcher == 8:
            if settings.run_func_mode:
                exit_code = safe_shell_exec.execute(jsrun_command, env=env, stdout=stdout, stderr=stderr)
                if exit_code != 0:
                    raise RuntimeError("jsrun failed with exit code {exit_code}".format(exit_code=exit_code))
            else:
                os.execve('/bin/sh', ['/bin/sh', '-c', jsrun_command], env)
            break

def generate_jsrun_rankfile(settings, path=None):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            cpu_per_gpu = (lsf.LSFUtils.get_num_cores() * lsf.LSFUtils.get_num_threads()) // lsf.LSFUtils.get_num_gpus()
            host_list = (x.split(':') for x in settings.hosts.split(','))
            validated_list = []
            remaining_slots = settings.num_proc
            dispatcher = 1
        elif dispatcher == 1:
            for host, slots in host_list:
                slots = int(slots)
                if slots > lsf.LSFUtils.get_num_gpus():
                    raise ValueError('Invalid host input, slot count for host \'{host}:{slots}\' is greater '
                                     'than number of GPUs per host \'{gpus}\'.'.format(
                        host=host, slots=slots, gpus=lsf.LSFUtils.get_num_gpus()))
                needed_slots = min(slots, remaining_slots)
                validated_list.append((host, needed_slots))
                remaining_slots -= needed_slots
                if remaining_slots == 0:
                    break
            dispatcher = 2
        elif dispatcher == 2:
            if remaining_slots != 0:
                raise ValueError('Not enough slots on the hosts to fulfill the {slots} requested.'.format(
                    slots=settings.num_proc))
            dispatcher = 3
        elif dispatcher == 3:
            if path is None:
                fd, path = tempfile.mkstemp()
                fd.close()
            dispatcher = 4
        elif dispatcher == 4:
            with open(path, 'w') as tmp:
                tmp.write('overlapping_rs: allow\n')
                tmp.write('cpu_index_using: logical\n')
                rank = 0
                for host, slots in validated_list:
                    cpu_val = 0
                    tmp.write('\n')
                    for s in range(slots):
                        tmp.write('rank: {rank}: {{ hostname: {host}; cpu: {{{scpu}-{ecpu}}} ; gpu: * ; mem: * }}\n'.format(
                            rank=rank,
                            host=host,
                            scpu=cpu_val,
                            ecpu=cpu_val + cpu_per_gpu - 1
                        ))
                        rank += 1
                        cpu_val += cpu_per_gpu
            return path