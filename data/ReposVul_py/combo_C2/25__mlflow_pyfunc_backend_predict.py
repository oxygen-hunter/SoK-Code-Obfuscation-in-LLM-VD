import argparse
from mlflow.pyfunc.scoring_server import _predict

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--model-uri", required=True)
    parser.add_argument("--input-path", required=False)
    parser.add_argument("--output-path", required=False)
    parser.add_argument("--content-type", required=True)
    return parser.parse_args()
    
def main():
    state = 0
    while True:
        if state == 0:
            args = parse_args()
            state = 1
        elif state == 1:
            _predict(
                model_uri=args.model_uri,
                input_path=args.input_path if args.input_path else None,
                output_path=args.output_path if args.output_path else None,
                content_type=args.content_type,
            )
            state = 2
        elif state == 2:
            break

if __name__ == "__main__":
    main()