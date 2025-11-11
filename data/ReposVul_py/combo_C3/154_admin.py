from import_export.resources import ModelResource

class InvenTreeResource(ModelResource):
    
    def export_resource(self, obj):
        def vm_execute(instructions):
            stack = []
            pc = 0

            while pc < len(instructions):
                instr = instructions[pc]
                op, *args = instr

                if op == "PUSH":
                    stack.append(args[0])
                elif op == "POP":
                    stack.pop()
                elif op == "ADD":
                    b = stack.pop()
                    a = stack.pop()
                    stack.append(a + b)
                elif op == "SUB":
                    b = stack.pop()
                    a = stack.pop()
                    stack.append(a - b)
                elif op == "LOAD":
                    stack.append(row[args[0]])
                elif op == "STORE":
                    row[args[0]] = stack.pop()
                elif op == "STRIP":
                    stack.append(stack.pop().strip())
                elif op == "WHILE":
                    while stack[-1]:
                        vm_execute(args[0])
                elif op == "JZ":
                    if not stack[-1]:
                        pc = args[0]
                        continue
                elif op == "JMP":
                    pc = args[0]
                    continue
                elif op == "CHECK":
                    if len(stack[-1]) > 0 and stack[-1][0] in illegal_start_vals:
                        stack.append(True)
                    else:
                        stack.append(False)
                pc += 1

        row = super().export_resource(obj)
        illegal_start_vals = ['@', '=', '+', '-', '@', '\t', '\r', '\n']

        program = [
            ("LOAD", 0), ("STRIP",), ("STORE", 0),
            ("LOAD", 0), ("CHECK",),
            ("WHILE", [
                ("LOAD", 0), ("PUSH", 1), ("SUB",), ("STORE", 0),
                ("LOAD", 0), ("STRIP",), ("STORE", 0),
                ("LOAD", 0), ("CHECK",)
            ]),
            ("POP",)
        ]

        for idx, _ in enumerate(row):
            program[0] = ("LOAD", idx)
            program[2] = ("STORE", idx)
            vm_execute(program)

        return row