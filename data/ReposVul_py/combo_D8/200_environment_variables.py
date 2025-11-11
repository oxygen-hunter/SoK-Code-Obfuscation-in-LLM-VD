import os


class _EnvironmentVariable:
    def __init__(self, name, type_, default):
        self._n = name
        self._t = type_
        self._d = default

    def _getName(self):
        return self._n

    def _getType(self):
        return self._t

    def _getDefault(self):
        return self._d

    @property
    def _isDefined(self):
        return self._getName() in os.environ

    def _getValue(self):
        val = os.getenv(self._getName())
        if val:
            try:
                return self._getType()(val)
            except Exception as e:
                raise ValueError(f"Failed to convert {val} to {self._getType()} for {self._getName()}: {e}")
        return self._getDefault()

    def __str__(self):
        return f"{self._getName()} (default: {self._getDefault()}, type: {self._getType().__name__})"

    def __repr__(self):
        return repr(self._getName())


class _BooleanEnvironmentVariable(_EnvironmentVariable):
    def __init__(self, name, default):
        if not (default is True or default is False or default is None):
            raise ValueError(f"{name} default value must be one of [True, False, None]")
        super().__init__(name, bool, default)

    def _getValue(self):
        if not self._isDefined:
            return self._getDefault()

        val = os.getenv(self._getName())
        lowercased = val.lower()
        if lowercased not in ["true", "false", "1", "0"]:
            raise ValueError(
                f"{self._getName()} value must be one of ['true', 'false', '1', '0'] (case-insensitive), "
                f"but got {val}"
            )
        return lowercased in ["true", "1"]


def _getMLFLOW_DFS_TMP():
    return _EnvironmentVariable("MLFLOW_DFS_TMP", str, "/tmp/mlflow")

def _getMLFLOW_HTTP_REQUEST_MAX_RETRIES():
    return _EnvironmentVariable("MLFLOW_HTTP_REQUEST_MAX_RETRIES", int, 5)

def _getMLFLOW_HTTP_REQUEST_BACKOFF_FACTOR():
    return _EnvironmentVariable("MLFLOW_HTTP_REQUEST_BACKOFF_FACTOR", int, 2)

def _getMLFLOW_HTTP_REQUEST_TIMEOUT():
    return _EnvironmentVariable("MLFLOW_HTTP_REQUEST_TIMEOUT", int, 120)

def _getMLFLOW_TRACKING_AWS_SIGV4():
    return _BooleanEnvironmentVariable("MLFLOW_TRACKING_AWS_SIGV4", False)

def _getMLFLOW_GCS_DOWNLOAD_CHUNK_SIZE():
    return _EnvironmentVariable("MLFLOW_GCS_DOWNLOAD_CHUNK_SIZE", int, None)

def _getMLFLOW_GCS_UPLOAD_CHUNK_SIZE():
    return _EnvironmentVariable("MLFLOW_GCS_UPLOAD_CHUNK_SIZE", int, None)

def _getMLFLOW_GCS_DEFAULT_TIMEOUT():
    return _EnvironmentVariable("MLFLOW_GCS_DEFAULT_TIMEOUT", int, None)

def _get_DISABLE_MLFLOWDBFS():
    return _EnvironmentVariable("DISABLE_MLFLOWDBFS", str, None)

def _getMLFLOW_S3_ENDPOINT_URL():
    return _EnvironmentVariable("MLFLOW_S3_ENDPOINT_URL", str, None)

def _getMLFLOW_S3_IGNORE_TLS():
    return _BooleanEnvironmentVariable("MLFLOW_S3_IGNORE_TLS", False)

def _getMLFLOW_S3_UPLOAD_EXTRA_ARGS():
    return _EnvironmentVariable("MLFLOW_S3_UPLOAD_EXTRA_ARGS", str, None)

def _getMLFLOW_KERBEROS_TICKET_CACHE():
    return _EnvironmentVariable("MLFLOW_KERBEROS_TICKET_CACHE", str, None)

def _getMLFLOW_KERBEROS_USER():
    return _EnvironmentVariable("MLFLOW_KERBEROS_USER", str, None)

def _getMLFLOW_PYARROW_EXTRA_CONF():
    return _EnvironmentVariable("MLFLOW_PYARROW_EXTRA_CONF", str, None)

def _getMLFLOW_SQLALCHEMYSTORE_POOL_SIZE():
    return _EnvironmentVariable("MLFLOW_SQLALCHEMYSTORE_POOL_SIZE", int, None)

def _getMLFLOW_SQLALCHEMYSTORE_POOL_RECYCLE():
    return _EnvironmentVariable("MLFLOW_SQLALCHEMYSTORE_POOL_RECYCLE", int, None)

def _getMLFLOW_SQLALCHEMYSTORE_MAX_OVERFLOW():
    return _EnvironmentVariable("MLFLOW_SQLALCHEMYSTORE_MAX_OVERFLOW", int, None)

def _getMLFLOW_SQLALCHEMYSTORE_ECHO():
    return _BooleanEnvironmentVariable("MLFLOW_SQLALCHEMYSTORE_ECHO", False)

def _getMLFLOW_DISABLE_ENV_MANAGER_CONDA_WARNING():
    return _BooleanEnvironmentVariable("MLFLOW_DISABLE_ENV_MANAGER_CONDA_WARNING", False)

def _getMLFLOW_SQLALCHEMYSTORE_POOLCLASS():
    return _EnvironmentVariable("MLFLOW_SQLALCHEMYSTORE_POOLCLASS", str, None)

def _getMLFLOW_REQUIREMENTS_INFERENCE_TIMEOUT():
    return _EnvironmentVariable("MLFLOW_REQUIREMENTS_INFERENCE_TIMEOUT", int, 120)

def _getMLFLOW_SCORING_SERVER_REQUEST_TIMEOUT():
    return _EnvironmentVariable("MLFLOW_SCORING_SERVER_REQUEST_TIMEOUT", int, 60)

def _getMLFLOW_ARTIFACT_UPLOAD_DOWNLOAD_TIMEOUT():
    return _EnvironmentVariable("MLFLOW_ARTIFACT_UPLOAD_DOWNLOAD_TIMEOUT", int, None)

def _getMLFLOW_DEFAULT_PREDICTION_DEVICE():
    return _EnvironmentVariable("MLFLOW_DEFAULT_PREDICTION_DEVICE", str, None)

def _getMLFLOW_ALLOW_FILE_URI_AS_MODEL_VERSION_SOURCE():
    return _BooleanEnvironmentVariable("MLFLOW_ALLOW_FILE_URI_AS_MODEL_VERSION_SOURCE", False)

MLFLOW_DFS_TMP = _getMLFLOW_DFS_TMP()
MLFLOW_HTTP_REQUEST_MAX_RETRIES = _getMLFLOW_HTTP_REQUEST_MAX_RETRIES()
MLFLOW_HTTP_REQUEST_BACKOFF_FACTOR = _getMLFLOW_HTTP_REQUEST_BACKOFF_FACTOR()
MLFLOW_HTTP_REQUEST_TIMEOUT = _getMLFLOW_HTTP_REQUEST_TIMEOUT()
MLFLOW_TRACKING_AWS_SIGV4 = _getMLFLOW_TRACKING_AWS_SIGV4()
MLFLOW_GCS_DOWNLOAD_CHUNK_SIZE = _getMLFLOW_GCS_DOWNLOAD_CHUNK_SIZE()
MLFLOW_GCS_UPLOAD_CHUNK_SIZE = _getMLFLOW_GCS_UPLOAD_CHUNK_SIZE()
MLFLOW_GCS_DEFAULT_TIMEOUT = _getMLFLOW_GCS_DEFAULT_TIMEOUT()
_DISABLE_MLFLOWDBFS = _get_DISABLE_MLFLOWDBFS()
MLFLOW_S3_ENDPOINT_URL = _getMLFLOW_S3_ENDPOINT_URL()
MLFLOW_S3_IGNORE_TLS = _getMLFLOW_S3_IGNORE_TLS()
MLFLOW_S3_UPLOAD_EXTRA_ARGS = _getMLFLOW_S3_UPLOAD_EXTRA_ARGS()
MLFLOW_KERBEROS_TICKET_CACHE = _getMLFLOW_KERBEROS_TICKET_CACHE()
MLFLOW_KERBEROS_USER = _getMLFLOW_KERBEROS_USER()
MLFLOW_PYARROW_EXTRA_CONF = _getMLFLOW_PYARROW_EXTRA_CONF()
MLFLOW_SQLALCHEMYSTORE_POOL_SIZE = _getMLFLOW_SQLALCHEMYSTORE_POOL_SIZE()
MLFLOW_SQLALCHEMYSTORE_POOL_RECYCLE = _getMLFLOW_SQLALCHEMYSTORE_POOL_RECYCLE()
MLFLOW_SQLALCHEMYSTORE_MAX_OVERFLOW = _getMLFLOW_SQLALCHEMYSTORE_MAX_OVERFLOW()
MLFLOW_SQLALCHEMYSTORE_ECHO = _getMLFLOW_SQLALCHEMYSTORE_ECHO()
MLFLOW_DISABLE_ENV_MANAGER_CONDA_WARNING = _getMLFLOW_DISABLE_ENV_MANAGER_CONDA_WARNING()
MLFLOW_SQLALCHEMYSTORE_POOLCLASS = _getMLFLOW_SQLALCHEMYSTORE_POOLCLASS()
MLFLOW_REQUIREMENTS_INFERENCE_TIMEOUT = _getMLFLOW_REQUIREMENTS_INFERENCE_TIMEOUT()
MLFLOW_SCORING_SERVER_REQUEST_TIMEOUT = _getMLFLOW_SCORING_SERVER_REQUEST_TIMEOUT()
MLFLOW_ARTIFACT_UPLOAD_DOWNLOAD_TIMEOUT = _getMLFLOW_ARTIFACT_UPLOAD_DOWNLOAD_TIMEOUT()
MLFLOW_DEFAULT_PREDICTION_DEVICE = _getMLFLOW_DEFAULT_PREDICTION_DEVICE()
MLFLOW_ALLOW_FILE_URI_AS_MODEL_VERSION_SOURCE = _getMLFLOW_ALLOW_FILE_URI_AS_MODEL_VERSION_SOURCE()