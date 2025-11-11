import argparse
from mlflow.pyfunc.scoring_server import _predict

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def run(self, instructions):
        self.instructions = instructions
        while self.pc < len(self.instructions):
            op = self.instructions[self.pc]
            if op == 'PUSH':
                self.pc += 1
                value = self.instructions[self.pc]
                self.stack.append(value)
            elif op == 'POP':
                self.stack.pop()
            elif op == 'LOAD':
                self.pc += 1
                value = self.instructions[self.pc]
                self.stack.append(value)
            elif op == 'CALL':
                self.pc += 1
                func = self.instructions[self.pc]
                args = []
                while self.stack:
                    args.append(self.stack.pop())
                func(*args[::-1])
            self.pc += 1

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--model-uri", required=True)
    parser.add_argument("--input-path", required=False)
    parser.add_argument("--output-path", required=False)
    parser.add_argument("--content-type", required=True)
    return parser.parse_args()

vm = VM()

def main():
    args = parse_args()
    vm.run([
        'PUSH', args.content_type,
        'PUSH', args.output_path if args.output_path else None,
        'PUSH', args.input_path if args.input_path else None,
        'PUSH', args.model_uri,
        'LOAD', _predict,
        'CALL'
    ])

if __name__ == "__main__":
    main()