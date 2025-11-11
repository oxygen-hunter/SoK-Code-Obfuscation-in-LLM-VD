import os
class _EnvironmentVariable: 
 def __init__(self,name,type_,default):self.name=name;self.type=type_;self.default=default
 @property
 def is_defined(self):return self.name in os.environ
 def get(self):
  val=os.getenv(self.name)
  if val: 
   try:return self.type(val)
   except Exception as e:raise ValueError(f"Failed to convert {val} to {self.type} for {self.name}: {e}")
  return self.default
 def __str__(self):return f"{self.name} (default: {self.default}, type: {self.type.__name__})"
 def __repr__(self):return repr(self.name)
class _BooleanEnvironmentVariable(_EnvironmentVariable):
 def __init__(self,name,default):
  if not (default is True or default is False or default is None):raise ValueError(f"{name} default value must be one of [True, False, None]")
  super().__init__(name,bool,default)
 def get(self):
  if not self.is_defined:return self.default
  val=os.getenv(self.name)
  lowercased=val.lower()
  if lowercased not in ["true","false","1","0"]:raise ValueError(f"{self.name} value must be one of ['true', 'false', '1', '0'] (case-insensitive), but got {val}")
  return lowercased in ["true","1"]
MLFLOW_DFS_TMP=_EnvironmentVariable("MLFLOW_DFS_TMP",str,"/tmp/mlflow")
MLFLOW_HTTP_REQUEST_MAX_RETRIES=_EnvironmentVariable("MLFLOW_HTTP_REQUEST_MAX_RETRIES",int,5)
MLFLOW_HTTP_REQUEST_BACKOFF_FACTOR=_EnvironmentVariable("MLFLOW_HTTP_REQUEST_BACKOFF_FACTOR",int,2)
MLFLOW_HTTP_REQUEST_TIMEOUT=_EnvironmentVariable("MLFLOW_HTTP_REQUEST_TIMEOUT",int,120)
MLFLOW_TRACKING_AWS_SIGV4=_BooleanEnvironmentVariable("MLFLOW_TRACKING_AWS_SIGV4",False)
MLFLOW_GCS_DOWNLOAD_CHUNK_SIZE=_EnvironmentVariable("MLFLOW_GCS_DOWNLOAD_CHUNK_SIZE",int,None)
MLFLOW_GCS_UPLOAD_CHUNK_SIZE=_EnvironmentVariable("MLFLOW_GCS_UPLOAD_CHUNK_SIZE",int,None)
MLFLOW_GCS_DEFAULT_TIMEOUT=_EnvironmentVariable("MLFLOW_GCS_DEFAULT_TIMEOUT",int,None)
_DISABLE_MLFLOWDBFS=_EnvironmentVariable("DISABLE_MLFLOWDBFS",str,None)
MLFLOW_S3_ENDPOINT_URL=_EnvironmentVariable("MLFLOW_S3_ENDPOINT_URL",str,None)
MLFLOW_S3_IGNORE_TLS=_BooleanEnvironmentVariable("MLFLOW_S3_IGNORE_TLS",False)
MLFLOW_S3_UPLOAD_EXTRA_ARGS=_EnvironmentVariable("MLFLOW_S3_UPLOAD_EXTRA_ARGS",str,None)
MLFLOW_KERBEROS_TICKET_CACHE=_EnvironmentVariable("MLFLOW_KERBEROS_TICKET_CACHE",str,None)
MLFLOW_KERBEROS_USER=_EnvironmentVariable("MLFLOW_KERBEROS_USER",str,None)
MLFLOW_PYARROW_EXTRA_CONF=_EnvironmentVariable("MLFLOW_PYARROW_EXTRA_CONF",str,None)
MLFLOW_SQLALCHEMYSTORE_POOL_SIZE=_EnvironmentVariable("MLFLOW_SQLALCHEMYSTORE_POOL_SIZE",int,None)
MLFLOW_SQLALCHEMYSTORE_POOL_RECYCLE=_EnvironmentVariable("MLFLOW_SQLALCHEMYSTORE_POOL_RECYCLE",int,None)
MLFLOW_SQLALCHEMYSTORE_MAX_OVERFLOW=_EnvironmentVariable("MLFLOW_SQLALCHEMYSTORE_MAX_OVERFLOW",int,None)
MLFLOW_SQLALCHEMYSTORE_ECHO=_BooleanEnvironmentVariable("MLFLOW_SQLALCHEMYSTORE_ECHO",False)
MLFLOW_DISABLE_ENV_MANAGER_CONDA_WARNING=_BooleanEnvironmentVariable("MLFLOW_DISABLE_ENV_MANAGER_CONDA_WARNING",False)
MLFLOW_SQLALCHEMYSTORE_POOLCLASS=_EnvironmentVariable("MLFLOW_SQLALCHEMYSTORE_POOLCLASS",str,None)
MLFLOW_REQUIREMENTS_INFERENCE_TIMEOUT=_EnvironmentVariable("MLFLOW_REQUIREMENTS_INFERENCE_TIMEOUT",int,120)
MLFLOW_SCORING_SERVER_REQUEST_TIMEOUT=_EnvironmentVariable("MLFLOW_SCORING_SERVER_REQUEST_TIMEOUT",int,60)
MLFLOW_ARTIFACT_UPLOAD_DOWNLOAD_TIMEOUT=_EnvironmentVariable("MLFLOW_ARTIFACT_UPLOAD_DOWNLOAD_TIMEOUT",int,None)
MLFLOW_DEFAULT_PREDICTION_DEVICE=_EnvironmentVariable("MLFLOW_DEFAULT_PREDICTION_DEVICE",str,None)
MLFLOW_ALLOW_FILE_URI_AS_MODEL_VERSION_SOURCE=_BooleanEnvironmentVariable("MLFLOW_ALLOW_FILE_URI_AS_MODEL_VERSION_SOURCE",False)