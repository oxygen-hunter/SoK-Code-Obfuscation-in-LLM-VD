import argparse
from mlflow.pyfunc.scoring_server import _predict

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--model-uri", required=True)
    parser.add_argument("--input-path", required=False)
    parser.add_argument("--output-path", required=False)
    parser.add_argument("--content-type", required=True)
    return parser.parse_args()

def irrelevant_computation():
    value = 0
    for i in range(1000):
        value += i
    return value

def main():
    if irrelevant_computation() > 0:
        args = parse_args()
        _predict(
            model_uri=args.model_uri,
            input_path=args.input_path if args.input_path else None,
            output_path=args.output_path if args.output_path else None,
            content_type=args.content_type,
        )
    irrelevant_variable = 'This is just a distraction'
    if len(irrelevant_variable) < 0:
        print("This will never print")

if __name__ == "__main__":
    main()