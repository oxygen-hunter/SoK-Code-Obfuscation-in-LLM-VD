import os


class _EnvironmentVariable:
    def __init__(self, n, t, d):
        self.n = n
        self.t = t
        self.d = d

    @property
    def is_defined(self):
        return self.n in os.environ

    def get(self):
        v = os.getenv(self.n)
        return self._convert(v) if v else self.d

    def _convert(self, v):
        try:
            return self.t(v)
        except Exception as e:
            raise ValueError(f"Failed to convert {v} to {self.t} for {self.n}: {e}")

    def __str__(self):
        return f"{self.n} (default: {self.d}, type: {self.t.__name__})"

    def __repr__(self):
        return repr(self.n)


class _BooleanEnvironmentVariable(_EnvironmentVariable):
    def __init__(self, n, d):
        if not (d is True or d is False or d is None):
            raise ValueError(f"{n} default value must be one of [True, False, None]")
        super().__init__(n, bool, d)

    def get(self):
        return self._get_bool(os.getenv(self.n)) if self.is_defined else self.d

    def _get_bool(self, v):
        l = v.lower()
        if l not in ["true", "false", "1", "0"]:
            raise ValueError(
                f"{self.n} value must be one of ['true', 'false', '1', '0'] (case-insensitive), but got {v}"
            )
        return l in ["true", "1"]


def create_env_var(name, var_type, default):
    return _EnvironmentVariable(name, var_type, default)


def create_bool_env_var(name, default):
    return _BooleanEnvironmentVariable(name, default)


def initialize_env_vars(vars_list):
    return [create_env_var(*var) for var in vars_list]


env_var_data = [
    ("MLFLOW_DFS_TMP", str, "/tmp/mlflow"),
    ("MLFLOW_HTTP_REQUEST_MAX_RETRIES", int, 5),
    ("MLFLOW_HTTP_REQUEST_BACKOFF_FACTOR", int, 2),
    ("MLFLOW_HTTP_REQUEST_TIMEOUT", int, 120),
    ("MLFLOW_GCS_DOWNLOAD_CHUNK_SIZE", int, None),
    ("MLFLOW_GCS_UPLOAD_CHUNK_SIZE", int, None),
    ("MLFLOW_GCS_DEFAULT_TIMEOUT", int, None),
    ("DISABLE_MLFLOWDBFS", str, None),
    ("MLFLOW_S3_ENDPOINT_URL", str, None),
    ("MLFLOW_S3_UPLOAD_EXTRA_ARGS", str, None),
    ("MLFLOW_KERBEROS_TICKET_CACHE", str, None),
    ("MLFLOW_KERBEROS_USER", str, None),
    ("MLFLOW_PYARROW_EXTRA_CONF", str, None),
    ("MLFLOW_SQLALCHEMYSTORE_POOL_SIZE", int, None),
    ("MLFLOW_SQLALCHEMYSTORE_POOL_RECYCLE", int, None),
    ("MLFLOW_SQLALCHEMYSTORE_MAX_OVERFLOW", int, None),
    ("MLFLOW_SQLALCHEMYSTORE_POOLCLASS", str, None),
    ("MLFLOW_REQUIREMENTS_INFERENCE_TIMEOUT", int, 120),
    ("MLFLOW_SCORING_SERVER_REQUEST_TIMEOUT", int, 60),
    ("MLFLOW_ARTIFACT_UPLOAD_DOWNLOAD_TIMEOUT", int, None),
    ("MLFLOW_DEFAULT_PREDICTION_DEVICE", str, None),
]

env_vars = initialize_env_vars(env_var_data)

bool_env_var_data = [
    ("MLFLOW_TRACKING_AWS_SIGV4", False),
    ("MLFLOW_S3_IGNORE_TLS", False),
    ("MLFLOW_SQLALCHEMYSTORE_ECHO", False),
    ("MLFLOW_DISABLE_ENV_MANAGER_CONDA_WARNING", False),
    ("MLFLOW_ALLOW_FILE_URI_AS_MODEL_VERSION_SOURCE", False),
]

bool_env_vars = [create_bool_env_var(*var) for var in bool_env_var_data]