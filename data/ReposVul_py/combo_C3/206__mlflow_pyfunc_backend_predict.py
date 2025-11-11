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
            inst = self.instructions[self.pc]
            self.execute(inst)
            self.pc += 1

    def execute(self, inst):
        op, *args = inst
        if op == "PUSH":
            self.stack.append(args[0])
        elif op == "POP":
            self.stack.pop()
        elif op == "CALL_PARSE_ARGS":
            self.stack.append(parse_args())
        elif op == "CALL_PREDICT":
            args = self.stack.pop()
            _predict(
                model_uri=args.model_uri,
                input_path=args.input_path if args.input_path else None,
                output_path=args.output_path if args.output_path else None,
                content_type=args.content_type,
            )
        elif op == "HALT":
            self.pc = len(self.instructions)

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--model-uri", required=True)
    parser.add_argument("--input-path", required=False)
    parser.add_argument("--output-path", required=False)
    parser.add_argument("--content-type", required=True)
    return parser.parse_args()

def main():
    vm = VM()
    program = [
        ("CALL_PARSE_ARGS",),
        ("CALL_PREDICT",),
        ("HALT",)
    ]
    vm.run(program)

if __name__ == "__main__":
    main()