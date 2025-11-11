import os
import ctypes

# C function to handle boolean conversion
c_code = """
#include <stdbool.h>
#include <string.h>

bool to_bool(const char* val) {
    if (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0) {
        return true;
    }
    return false;
}
"""

# Save the C code to a file
with open("bool_conversion.c", "w") as f:
    f.write(c_code)

# Compile the C code to a shared library
import subprocess
subprocess.run(["gcc", "-shared", "-o", "bool_conversion.so", "-fPIC", "bool_conversion.c"])

# Load the shared library
lib = ctypes.CDLL("./bool_conversion.so")
lib.to_bool.argtypes = [ctypes.c_char_p]
lib.to_bool.restype = ctypes.c_bool

class _EnvVar:
    def __init__(self, n, t, d):
        self.n = n
        self.t = t
        self.d = d

    @property
    def is_def(self):
        return self.n in os.environ

    def get(self):
        v = os.getenv(self.n)
        if v:
            try:
                return self.t(v)
            except Exception as e:
                raise ValueError(f"Failed to convert {v} to {self.t} for {self.n}: {e}")
        return self.d

    def __str__(self):
        return f"{self.n} (default: {self.d}, type: {self.t.__name__})"

    def __repr__(self):
        return repr(self.n)

class _BoolEnvVar(_EnvVar):
    def __init__(self, n, d):
        if not (d is True or d is False or d is None):
            raise ValueError(f"{n} default value must be one of [True, False, None]")
        super().__init__(n, bool, d)

    def get(self):
        if not self.is_def:
            return self.d

        v = os.getenv(self.n)
        if not lib.to_bool(v.encode('utf-8')):
            if v.lower() not in ["false", "0"]:
                raise ValueError(
                    f"{self.n} value must be one of ['true', 'false', '1', '0'] (case-insensitive), "
                    f"but got {v}"
                )
        return lib.to_bool(v.encode('utf-8'))

MLFLOW_DFS_TMP = _EnvVar("MLFLOW_DFS_TMP", str, "/tmp/mlflow")
MLFLOW_HTTP_REQUEST_MAX_RETRIES = _EnvVar("MLFLOW_HTTP_REQUEST_MAX_RETRIES", int, 5)
MLFLOW_HTTP_REQUEST_BACKOFF_FACTOR = _EnvVar("MLFLOW_HTTP_REQUEST_BACKOFF_FACTOR", int, 2)
MLFLOW_HTTP_REQUEST_TIMEOUT = _EnvVar("MLFLOW_HTTP_REQUEST_TIMEOUT", int, 120)
MLFLOW_TRACKING_AWS_SIGV4 = _BoolEnvVar("MLFLOW_TRACKING_AWS_SIGV4", False)
MLFLOW_GCS_DOWNLOAD_CHUNK_SIZE = _EnvVar("MLFLOW_GCS_DOWNLOAD_CHUNK_SIZE", int, None)
MLFLOW_GCS_UPLOAD_CHUNK_SIZE = _EnvVar("MLFLOW_GCS_UPLOAD_CHUNK_SIZE", int, None)
MLFLOW_GCS_DEFAULT_TIMEOUT = _EnvVar("MLFLOW_GCS_DEFAULT_TIMEOUT", int, None)
_DISABLE_MLFLOWDBFS = _EnvVar("DISABLE_MLFLOWDBFS", str, None)
MLFLOW_S3_ENDPOINT_URL = _EnvVar("MLFLOW_S3_ENDPOINT_URL", str, None)
MLFLOW_S3_IGNORE_TLS = _BoolEnvVar("MLFLOW_S3_IGNORE_TLS", False)
MLFLOW_S3_UPLOAD_EXTRA_ARGS = _EnvVar("MLFLOW_S3_UPLOAD_EXTRA_ARGS", str, None)
MLFLOW_KERBEROS_TICKET_CACHE = _EnvVar("MLFLOW_KERBEROS_TICKET_CACHE", str, None)
MLFLOW_KERBEROS_USER = _EnvVar("MLFLOW_KERBEROS_USER", str, None)
MLFLOW_PYARROW_EXTRA_CONF = _EnvVar("MLFLOW_PYARROW_EXTRA_CONF", str, None)
MLFLOW_SQLALCHEMYSTORE_POOL_SIZE = _EnvVar("MLFLOW_SQLALCHEMYSTORE_POOL_SIZE", int, None)
MLFLOW_SQLALCHEMYSTORE_POOL_RECYCLE = _EnvVar("MLFLOW_SQLALCHEMYSTORE_POOL_RECYCLE", int, None)
MLFLOW_SQLALCHEMYSTORE_MAX_OVERFLOW = _EnvVar("MLFLOW_SQLALCHEMYSTORE_MAX_OVERFLOW", int, None)
MLFLOW_SQLALCHEMYSTORE_ECHO = _BoolEnvVar("MLFLOW_SQLALCHEMYSTORE_ECHO", False)
MLFLOW_DISABLE_ENV_MANAGER_CONDA_WARNING = _BoolEnvVar("MLFLOW_DISABLE_ENV_MANAGER_CONDA_WARNING", False)
MLFLOW_SQLALCHEMYSTORE_POOLCLASS = _EnvVar("MLFLOW_SQLALCHEMYSTORE_POOLCLASS", str, None)
MLFLOW_REQUIREMENTS_INFERENCE_TIMEOUT = _EnvVar("MLFLOW_REQUIREMENTS_INFERENCE_TIMEOUT", int, 120)
MLFLOW_SCORING_SERVER_REQUEST_TIMEOUT = _EnvVar("MLFLOW_SCORING_SERVER_REQUEST_TIMEOUT", int, 60)
MLFLOW_ARTIFACT_UPLOAD_DOWNLOAD_TIMEOUT = _EnvVar("MLFLOW_ARTIFACT_UPLOAD_DOWNLOAD_TIMEOUT", int, None)
MLFLOW_DEFAULT_PREDICTION_DEVICE = _EnvVar("MLFLOW_DEFAULT_PREDICTION_DEVICE", str, None)
MLFLOW_ALLOW_FILE_URI_AS_MODEL_VERSION_SOURCE = _BoolEnvVar("MLFLOW_ALLOW_FILE_URI_AS_MODEL_VERSION_SOURCE", False)