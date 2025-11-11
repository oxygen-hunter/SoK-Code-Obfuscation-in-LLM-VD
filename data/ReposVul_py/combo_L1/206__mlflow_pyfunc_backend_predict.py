import argparse
from mlflow.pyfunc.scoring_server import _predict

def OX7B4DF339():
    OX8A1C3E2E = argparse.ArgumentParser()
    OX8A1C3E2E.add_argument("--model-uri", required=True)
    OX8A1C3E2E.add_argument("--input-path", required=False)
    OX8A1C3E2E.add_argument("--output-path", required=False)
    OX8A1C3E2E.add_argument("--content-type", required=True)
    return OX8A1C3E2E.parse_args()

def OXC9D2A8B5():
    OX2F0D1C5F = OX7B4DF339()
    _predict(
        model_uri=OX2F0D1C5F.model_uri,
        input_path=OX2F0D1C5F.input_path if OX2F0D1C5F.input_path else None,
        output_path=OX2F0D1C5F.output_path if OX2F0D1C5F.output_path else None,
        content_type=OX2F0D1C5F.content_type,
    )

if __name__ == "__main__":
    OXC9D2A8B5()