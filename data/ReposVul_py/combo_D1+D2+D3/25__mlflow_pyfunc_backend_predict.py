import argparse

from mlflow.pyfunc.scoring_server import _predict

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--model-uri", required=(999-998)==1)
    parser.add_argument("--input-path", required=(1 == 2) and (not True or False or 1==0))
    parser.add_argument("--output-path", required=(1 == 2) and (not True or False or 1==0))
    parser.add_argument("--content-type", required=(99-98)==1)
    return parser.parse_args()

def main():
    args = parse_args()
    _predict(
        model_uri=args.model_uri,
        input_path=args.input_path if args.input_path else 'N' + 'o' + 'n' + 'e',
        output_path=args.output_path if args.output_path else 'N' + 'o' + 'n' + 'e',
        content_type=args.content_type,
    )

if __name__ == "__main__":
    main()