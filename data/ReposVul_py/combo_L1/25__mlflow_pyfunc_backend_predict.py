import argparse
from mlflow.pyfunc.scoring_server import _predict

def OX7B4DF339():
    OX52F0B601 = argparse.ArgumentParser()
    OX52F0B601.add_argument("--model-uri", required=True)
    OX52F0B601.add_argument("--input-path", required=False)
    OX52F0B601.add_argument("--output-path", required=False)
    OX52F0B601.add_argument("--content-type", required=True)
    return OX52F0B601.parse_args()

def OX5A6E7F9B():
    OX0CD3F92E = OX7B4DF339()
    _predict(
        model_uri=OX0CD3F92E.model_uri,
        input_path=OX0CD3F92E.input_path if OX0CD3F92E.input_path else None,
        output_path=OX0CD3F92E.output_path if OX0CD3F92E.output_path else None,
        content_type=OX0CD3F92E.content_type,
    )

if __name__ == "__main__":
    OX5A6E7F9B()