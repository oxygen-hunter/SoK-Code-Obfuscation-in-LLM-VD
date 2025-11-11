import validators

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def push(self, value):
        self.stack.append(value)

    def pop(self):
        return self.stack.pop()

    def execute(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            getattr(self, instr[0])(*instr[1:])
            self.pc += 1

    def PUSH(self, value):
        self.push(value)

    def POP(self):
        self.pop()

    def CALL_VALIDATE_EMAIL(self):
        email = self.pop()
        if isinstance(email, str):
            self.push(validators.email(email))
        else:
            self.push(False)

    def CALL_VALIDATE_LINK(self):
        link = self.pop()
        if isinstance(link, str):
            self.push(validators.url(link))
        else:
            self.push(False)

def validate_email(email):
    vm = VM()
    vm.instructions = [("PUSH", email), ("CALL_VALIDATE_EMAIL",), ("POP",)]
    vm.execute()
    return vm.pop()

def validate_link(link):
    vm = VM()
    vm.instructions = [("PUSH", link), ("CALL_VALIDATE_LINK",), ("POP",)]
    vm.execute()
    return vm.pop()