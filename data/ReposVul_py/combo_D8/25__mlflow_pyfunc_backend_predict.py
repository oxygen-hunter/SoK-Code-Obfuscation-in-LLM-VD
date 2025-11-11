import argparse
from mlflow.pyfunc.scoring_server import _predict

def get_parser():
    return argparse.ArgumentParser()

def get_argument(arg_name, parser, req=False):
    return parser.add_argument(arg_name, required=req)

def get_args(parser):
    return parser.parse_args()

def parse_args():
    p = get_parser()
    get_argument("--model-uri", p, True)
    get_argument("--input-path", p)
    get_argument("--output-path", p)
    get_argument("--content-type", p, True)
    return get_args(p)

def main():
    a = parse_args()
    _predict(
        model_uri=getattr(a, "model_uri"),
        input_path=getattr(a, "input_path") if getattr(a, "input_path") else None,
        output_path=getattr(a, "output_path") if getattr(a, "output_path") else None,
        content_type=getattr(a, "content_type"),
    )

if __name__ == "__main__":
    main()