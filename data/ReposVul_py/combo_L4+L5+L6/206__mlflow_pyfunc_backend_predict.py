import argparse
from mlflow.pyfunc.scoring_server import _predict

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--model-uri", required=True)
    parser.add_argument("--input-path", required=False)
    parser.add_argument("--output-path", required=False)
    parser.add_argument("--content-type", required=True)
    return parser.parse_args()

def recursive_main(args):
    if args:
        _predict(
            model_uri=args.model_uri,
            input_path=args.input_path if args.input_path else None,
            output_path=args.output_path if args.output_path else None,
            content_type=args.content_type,
        )
    else:
        return

def main():
    args = parse_args()
    recursive_main(args)

if __name__ == "__main__":
    main()