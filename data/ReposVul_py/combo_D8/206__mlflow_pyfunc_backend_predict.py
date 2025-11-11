import argparse
from mlflow.pyfunc.scoring_server import _predict

def dynamic_parser():
    return argparse.ArgumentParser()

def dynamic_add_argument(parser, *args, **kwargs):
    parser.add_argument(*args, **kwargs)

def dynamic_get_args(parser):
    return parser.parse_args()

def dynamic_getattr(args, attr):
    return getattr(args, attr)

def dynamic_none():
    return None

def parse_args():
    p = dynamic_parser()
    dynamic_add_argument(p, "--model-uri", required=True)
    dynamic_add_argument(p, "--input-path", required=False)
    dynamic_add_argument(p, "--output-path", required=False)
    dynamic_add_argument(p, "--content-type", required=True)
    return dynamic_get_args(p)

def main():
    args = parse_args()
    _predict(
        model_uri=dynamic_getattr(args, 'model_uri'),
        input_path=dynamic_getattr(args, 'input_path') if dynamic_getattr(args, 'input_path') else dynamic_none(),
        output_path=dynamic_getattr(args, 'output_path') if dynamic_getattr(args, 'output_path') else dynamic_none(),
        content_type=dynamic_getattr(args, 'content_type'),
    )

if __name__ == "__main__":
    main()