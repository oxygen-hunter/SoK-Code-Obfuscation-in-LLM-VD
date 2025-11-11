import os

class VirtualMachine:
    def __init__(self, instructions):
        self.instructions = instructions
        self.stack = []
        self.pc = 0

    def run(self):
        while self.pc < len(self.instructions):
            op, *args = self.instructions[self.pc]
            getattr(self, f'op_{op}')(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        b = self.op_POP()
        a = self.op_POP()
        self.stack.append(a + b)

    def op_SUB(self):
        b = self.op_POP()
        a = self.op_POP()
        self.stack.append(a - b)

    def op_LOAD(self, name):
        self.stack.append(os.getenv(name))

    def op_STORE(self, name, value):
        os.environ[name] = value

    def op_JMP(self, address):
        self.pc = address - 1

    def op_JZ(self, address):
        value = self.op_POP()
        if value == 0:
            self.pc = address - 1

    def op_CONVERT(self, type_):
        value = self.op_POP()
        try:
            self.stack.append(type_(value))
        except Exception as e:
            raise ValueError(f"Failed to convert {value}: {e}")

    def op_RETURN(self):
        self.pc = len(self.instructions)

class EnvironmentVariableManager:
    def __init__(self):
        self.vm = VirtualMachine([])

    def define_variable(self, name, type_, default):
        self.vm.instructions.append(('PUSH', name))
        self.vm.instructions.append(('LOAD', name))
        self.vm.instructions.append(('PUSH', default))
        self.vm.instructions.append(('JZ', 8))
        self.vm.instructions.append(('PUSH', type_))
        self.vm.instructions.append(('CONVERT', type_))
        self.vm.instructions.append(('RETURN',))
        self.vm.instructions.append(('RETURN',))
        
    def get_value(self):
        self.vm.run()
        return self.vm.op_POP()

env_var_manager = EnvironmentVariableManager()
env_var_manager.define_variable("MLFLOW_DFS_TMP", str, "/tmp/mlflow")
print(env_var_manager.get_value())

env_var_manager.define_variable("MLFLOW_HTTP_REQUEST_MAX_RETRIES", int, 5)
print(env_var_manager.get_value())

env_var_manager.define_variable("MLFLOW_HTTP_REQUEST_BACKOFF_FACTOR", int, 2)
print(env_var_manager.get_value())

env_var_manager.define_variable("MLFLOW_HTTP_REQUEST_TIMEOUT", int, 120)
print(env_var_manager.get_value())

env_var_manager.define_variable("MLFLOW_TRACKING_AWS_SIGV4", lambda x: x.lower() in ["true", "1"], False)
print(env_var_manager.get_value())