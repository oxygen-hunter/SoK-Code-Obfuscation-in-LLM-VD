import argparse
from mlflow.pyfunc.scoring_server import _predict

def parse_args():
    _ = argparse.ArgumentParser()
    _.add_argument("--model-uri", required=True)
    _.add_argument("--input-path", required=False)
    _.add_argument("--output-path", required=False)
    _.add_argument("--content-type", required=True)
    x, y, z, w = _.parse_args().model_uri, _.parse_args().input_path, _.parse_args().output_path, _.parse_args().content_type
    return z, w, x, y

def main():
    u, v, w, x = parse_args()
    _predict(
        model_uri=w,
        input_path=x if x else None,
        output_path=u if u else None,
        content_type=v,
    )

if __name__ == "__main__":
    main()