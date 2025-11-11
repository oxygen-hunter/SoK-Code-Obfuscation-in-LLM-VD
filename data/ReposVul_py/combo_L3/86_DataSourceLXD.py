"""Datasource for LXD, reads /dev/lxd/sock representation of instance data.
Notes:
 * This datasource replaces previous NoCloud datasource for LXD.
 * Older LXD images may not have updates for cloud-init so NoCloud may
   still be detected on those images.
 * Detect LXD datasource when /dev/lxd/sock is an active socket file.
 * Info on dev-lxd API: https://linuxcontainers.org/lxd/docs/master/dev-lxd
"""
import os, socket, stat, time
from enum import Flag, auto
from json.decoder import JSONDecodeError
from typing import Any, Dict, List, Optional, Tuple, Union, cast
import requests
from requests.adapters import HTTPAdapter
from urllib3.connection import HTTPConnection
from urllib3.connectionpool import HTTPConnectionPool
from cloudinit import log as logging
from cloudinit import sources, subp, url_helper, util
from cloudinit.net import find_fallback_nic
LOG=logging.getLogger(__name__)
LXD_SOCKET_PATH="/dev/lxd/sock"
LXD_SOCKET_API_VERSION="1.0"
LXD_URL="http://lxd"
CONFIG_KEY_ALIASES={"cloud-init.user-data":"user-data","cloud-init.network-config":"network-config","cloud-init.vendor-data":"vendor-data","user.user-data":"user-data","user.network-config":"network-config","user.vendor-data":"vendor-data"}
def _get_fallback_interface_name()->str:
 default_name="eth0"
 if subp.which("systemd-detect-virt"):
  try:virt_type,_=subp.subp(["systemd-detect-virt"])
  except subp.ProcessExecutionError as err:
   LOG.warning("Unable to run systemd-detect-virt: %s."" Rendering default network config.",err,)
   return default_name
  if virt_type.strip()in("kvm","qemu",):
   arch=util.system_info()["uname"][4]
   if arch=="ppc64le":return"enp0s5"
   elif arch=="s390x":return"enc9"
   else:return"enp5s0"
 return default_name
def generate_network_config(nics:Optional[List[str]]=None,)->Dict[str,Any]:
 primary_nic=find_fallback_nic()
 if primary_nic:LOG.debug("LXD datasource generating network from discovered active"" device: %s",primary_nic,)
 else:
  primary_nic=_get_fallback_interface_name()
  LOG.debug("LXD datasource generating network from systemd-detect-virt"" platform default device: %s",primary_nic,)
 return{"version":1,"config":[{"type":"physical","name":primary_nic,"subnets":[{"type":"dhcp","control":"auto"}],}],}
class SocketHTTPConnection(HTTPConnection):
 def __init__(self,socket_path):super().__init__("localhost");self.socket_path=socket_path
 def connect(self):self.sock=socket.socket(socket.AF_UNIX,socket.SOCK_STREAM);self.sock.connect(self.socket_path)
class SocketConnectionPool(HTTPConnectionPool):
 def __init__(self,socket_path):self.socket_path=socket_path;super().__init__("localhost")
 def _new_conn(self):return SocketHTTPConnection(self.socket_path)
class LXDSocketAdapter(HTTPAdapter):
 def get_connection(self,url,proxies=None):return SocketConnectionPool(LXD_SOCKET_PATH)
def _raw_instance_data_to_dict(metadata_type:str,metadata_value)->dict:
 if isinstance(metadata_value,dict):return metadata_value
 if metadata_value is None:return{}
 try:parsed_metadata=util.load_yaml(metadata_value)
 except AttributeError as exc:
  raise sources.InvalidMetaDataException("Invalid {md_type}. Expected str, bytes or dict but found:"" {value}".format(md_type=metadata_type,value=metadata_value)) from exc
 if parsed_metadata is None:
  raise sources.InvalidMetaDataException("Invalid {md_type} format. Expected YAML but found:"" {value}".format(md_type=metadata_type,value=metadata_value))
 return parsed_metadata
class DataSourceLXD(sources.DataSource):
 dsname="LXD"
 _network_config:Union[Dict,str]=sources.UNSET
 _crawled_metadata:Union[Dict,str]=sources.UNSET
 sensitive_metadata_keys:Tuple[str,...]=sources.DataSource.sensitive_metadata_keys+("user.meta-data","user.vendor-data","user.user-data","cloud-init.user-data","cloud-init.vendor-data",)
 skip_hotplug_detect=True
 def _unpickle(self,ci_pkl_version:int)->None:
  super()._unpickle(ci_pkl_version)
  self.skip_hotplug_detect=True
 @staticmethod
 def ds_detect()->bool:return is_platform_viable()
 def _get_data(self)->bool:
  self._crawled_metadata=util.log_time(logfunc=LOG.debug,msg="Crawl of metadata service",func=read_metadata,)
  self.metadata=_raw_instance_data_to_dict("meta-data",self._crawled_metadata.get("meta-data"))
  config=self._crawled_metadata.get("config",{})
  user_metadata=config.get("user.meta-data",{})
  if user_metadata:user_metadata=_raw_instance_data_to_dict("user.meta-data",user_metadata)
  if not isinstance(self.metadata,dict):self.metadata=util.mergemanydict([util.load_yaml(self.metadata),user_metadata])
  if"user-data"in self._crawled_metadata:self.userdata_raw=self._crawled_metadata["user-data"]
  if"network-config"in self._crawled_metadata:self._network_config=_raw_instance_data_to_dict("network-config",self._crawled_metadata["network-config"])
  if"vendor-data"in self._crawled_metadata:self.vendordata_raw=self._crawled_metadata["vendor-data"]
  return True
 def _get_subplatform(self)->str:
  return"LXD socket API v. {ver} ({socket})".format(ver=LXD_SOCKET_API_VERSION,socket=LXD_SOCKET_PATH)
 def check_instance_id(self,sys_cfg)->str:
  response=read_metadata(metadata_keys=MetaDataKeys.META_DATA)
  md=response.get("meta-data",{})
  if not isinstance(md,dict):md=util.load_yaml(md)
  return md.get("instance-id")==self.metadata.get("instance-id")
 @property
 def network_config(self)->dict:
  if self._network_config==sources.UNSET:
   if self._crawled_metadata==sources.UNSET:self._get_data()
   if isinstance(self._crawled_metadata,dict):
    if self._crawled_metadata.get("network-config"):
     LOG.debug("LXD datasource using provided network config")
     self._network_config=self._crawled_metadata["network-config"]
    elif self._crawled_metadata.get("devices"):
     devices:List[str]=[k for k,v in self._crawled_metadata["devices"].items()if v["type"]=="nic"]
     self._network_config=generate_network_config(devices)
  if self._network_config==sources.UNSET:
   LOG.debug("LXD datasource generating network config using fallback.")
   self._network_config=generate_network_config()
  return cast(dict,self._network_config)
def is_platform_viable()->bool:
 if os.path.exists(LXD_SOCKET_PATH):return stat.S_ISSOCK(os.lstat(LXD_SOCKET_PATH).st_mode)
 return False
def _get_json_response(session:requests.Session,url:str,do_raise:bool=True):
 url_response=_do_request(session,url,do_raise)
 if not url_response.ok:
  LOG.debug("Skipping %s on [HTTP:%d]:%s",url,url_response.status_code,url_response.text,)
  return{}
 try:return url_response.json()
 except JSONDecodeError as exc:
  raise sources.InvalidMetaDataException("Unable to process LXD config at {url}."" Expected JSON but found: {resp}".format(url=url,resp=url_response.text)) from exc
def _do_request(session:requests.Session,url:str,do_raise:bool=True)->requests.Response:
 for retries in range(30,0,-1):
  response=session.get(url)
  if 500==response.status_code:
   time.sleep(0.1)
   LOG.warning("[GET] [HTTP:%d] %s, retrying %d more time(s)",response.status_code,url,retries,)
  else:break
 LOG.debug("[GET] [HTTP:%d] %s",response.status_code,url)
 if do_raise and not response.ok:
  raise sources.InvalidMetaDataException("Invalid HTTP response [{code}] from {route}: {resp}".format(code=response.status_code,route=url,resp=response.text,))
 return response
class MetaDataKeys(Flag):
 NONE=auto()
 CONFIG=auto()
 DEVICES=auto()
 META_DATA=auto()
 ALL=CONFIG|DEVICES|META_DATA
class _MetaDataReader:
 def __init__(self,api_version:str=LXD_SOCKET_API_VERSION):
  self.api_version=api_version
  self._version_url=url_helper.combine_url(LXD_URL,self.api_version)
 def _process_config(self,session:requests.Session)->dict:
  config:dict={"config":{}}
  config_url=url_helper.combine_url(self._version_url,"config")
  config_routes=_get_json_response(session,config_url)
  for config_route in sorted(config_routes):
   config_route_url=url_helper.combine_url(LXD_URL,config_route)
   config_route_response=_do_request(session,config_route_url,do_raise=False)
   if not config_route_response.ok:
    LOG.debug("Skipping %s on [HTTP:%d]:%s",config_route_url,config_route_response.status_code,config_route_response.text,)
    continue
   cfg_key=config_route.rpartition("/")[-1]
   config["config"][cfg_key]=config_route_response.text
   if cfg_key in CONFIG_KEY_ALIASES:
    if CONFIG_KEY_ALIASES[cfg_key]not in config:
     config[CONFIG_KEY_ALIASES[cfg_key]]=config_route_response.text
    else:
     LOG.warning("Ignoring LXD config %s in favor of %s value.",cfg_key,cfg_key.replace("user","cloud-init",1),)
  return config
 def __call__(self,*,metadata_keys:MetaDataKeys)->dict:
  with requests.Session()as session:
   session.mount(self._version_url,LXDSocketAdapter())
   md:dict={"_metadata_api_version":self.api_version}
   if MetaDataKeys.META_DATA in metadata_keys:
    md_route=url_helper.combine_url(self._version_url,"meta-data")
    md["meta-data"]=_do_request(session,md_route).text
   if MetaDataKeys.CONFIG in metadata_keys:md.update(self._process_config(session))
   if MetaDataKeys.DEVICES in metadata_keys:
    url=url_helper.combine_url(self._version_url,"devices")
    devices=_get_json_response(session,url,do_raise=False)
    if devices:md["devices"]=devices
   return md
def read_metadata(api_version:str=LXD_SOCKET_API_VERSION,metadata_keys:MetaDataKeys=MetaDataKeys.ALL,)->dict:return _MetaDataReader(api_version=api_version)(metadata_keys=metadata_keys)
datasources=[(DataSourceLXD,(sources.DEP_FILESYSTEM,)),]
def get_datasource_list(depends):return sources.list_from_depends(depends,datasources)
if __name__=="__main__":
 import argparse
 description="""Query LXD metadata and emit a JSON object."""
 parser=argparse.ArgumentParser(description=description)
 parser.parse_args()
 print(util.json_dumps(read_metadata(metadata_keys=MetaDataKeys.ALL)))