import argparse
from mlflow.pyfunc.scoring_server import _predict

def parse_args():
    parser = argparse.ArgumentParser()
    a = "--model-uri"
    b = "--input-path"
    c = "--output-path"
    d = "--content-type"
    e = True
    f = False
    parser.add_argument(a, required=e)
    parser.add_argument(b, required=f)
    parser.add_argument(c, required=f)
    parser.add_argument(d, required=e)
    return parser.parse_args()

def main():
    args = parse_args()
    _predict(
        model_uri=args.model_uri,
        output_path=args.output_path if args.output_path else None,
        input_path=args.input_path if args.input_path else None,
        content_type=args.content_type,
    )

if __name__ == "__main__":
    main()