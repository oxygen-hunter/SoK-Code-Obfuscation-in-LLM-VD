import argparse
from mlflow.pyfunc.scoring_server import _predict

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--model-uri", required=True)
    parser.add_argument("--input-path", required=False)
    parser.add_argument("--output-path", required=False)
    parser.add_argument("--content-type", required=True)
    return parser.parse_args()

def recursive_predict(args):
    if args.input_path:
        input_path = args.input_path
    else:
        input_path = None
    
    if args.output_path:
        output_path = args.output_path
    else:
        output_path = None
    
    _predict(
        model_uri=args.model_uri,
        input_path=input_path,
        output_path=output_path,
        content_type=args.content_type,
    )

def main():
    recursive_predict(parse_args())

if __name__ == "__main__":
    main()