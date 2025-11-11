import os
import socket
import stat
import time
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

LOG = logging.getLogger(__name__)

LXD_SOCKET_PATH = "/dev/lxd/sock"
LXD_SOCKET_API_VERSION = "1.0"
LXD_URL = "http://lxd"

CONFIG_KEY_ALIASES = {
    "cloud-init.user-data": "user-data",
    "cloud-init.network-config": "network-config",
    "cloud-init.vendor-data": "vendor-data",
    "user.user-data": "user-data",
    "user.network-config": "network-config",
    "user.vendor-data": "vendor-data",
}

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = None

    def run(self, program):
        self.program = program
        while self.pc < len(self.program):
            op = self.program[self.pc]
            self.pc += 1
            op(self)

    def push(self, value):
        self.stack.append(value)

    def pop(self):
        return self.stack.pop()

    def add(self):
        self.push(self.pop() + self.pop())

    def sub(self):
        b, a = self.pop(), self.pop()
        self.push(a - b)

    def load(self, index):
        self.push(self.stack[index])

    def store(self, index):
        self.stack[index] = self.pop()

    def jmp(self, addr):
        self.pc = addr

    def jz(self, addr):
        if self.pop() == 0:
            self.pc = addr

def run_vm_program(program):
    vm = VM()
    vm.run(program)

def _get_fallback_interface_name():
    def vm_program(vm):
        vm.push(0)
        vm.push("eth0")
        vm.store(0)

        vm.push(subp.which("systemd-detect-virt"))
        vm.jz(10)

        vm.push(["systemd-detect-virt"])
        vm.push(subp.subp)
        vm.push(LOG.warning)
        vm.push("Unable to run systemd-detect-virt: %s. Rendering default network config.")
        vm.jmp(11)
        vm.pc = 10

        vm.push(util.system_info()["uname"][4])
        vm.push("ppc64le")
        vm.jz(12)

        vm.push("enp0s5")
        vm.jmp(14)
        vm.pc = 12

        vm.push("s390x")
        vm.jz(13)

        vm.push("enc9")
        vm.jmp(14)
        vm.pc = 13

        vm.push("enp5s0")
        vm.pc = 14

        vm.load(0)

    run_vm_program(vm_program)

def generate_network_config(nics=None):
    def vm_program(vm):
        vm.push(find_fallback_nic())
        vm.push(LOG.debug)
        vm.push("LXD datasource generating network from discovered active device: %s")
        vm.jz(2)
        vm.push(_get_fallback_interface_name())
        vm.push(LOG.debug)
        vm.push("LXD datasource generating network from systemd-detect-virt platform default device: %s")
        vm.pc = 2

        vm.push({
            "version": 1,
            "config": [
                {
                    "type": "physical",
                    "name": vm.pop(),
                    "subnets": [{"type": "dhcp", "control": "auto"}],
                }
            ],
        })
    run_vm_program(vm_program)

class SocketHTTPConnection(HTTPConnection):
    def __init__(self, socket_path):
        super().__init__("localhost")
        self.socket_path = socket_path

    def connect(self):
        self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
        self.sock.connect(self.socket_path)


class SocketConnectionPool(HTTPConnectionPool):
    def __init__(self, socket_path):
        self.socket_path = socket_path
        super().__init__("localhost")

    def _new_conn(self):
        return SocketHTTPConnection(self.socket_path)


class LXDSocketAdapter(HTTPAdapter):
    def get_connection(self, url, proxies=None):
        return SocketConnectionPool(LXD_SOCKET_PATH)


def _raw_instance_data_to_dict(metadata_type: str, metadata_value) -> dict:
    def vm_program(vm):
        vm.push(metadata_value)

        vm.jz(1)
        vm.push(metadata_value)
        vm.push(util.load_yaml)
        vm.push(sources.InvalidMetaDataException)
        vm.push("Invalid {md_type}. Expected str, bytes or dict but found: {value}".format(md_type=metadata_type, value=metadata_value))
        vm.jmp(2)

        vm.pc = 1
        vm.push({})
        vm.pc = 2
    run_vm_program(vm_program)

class DataSourceLXD(sources.DataSource):
    dsname = "LXD"
    _network_config: Union[Dict, str] = sources.UNSET
    _crawled_metadata: Union[Dict, str] = sources.UNSET

    sensitive_metadata_keys: Tuple[str, ...] = sources.DataSource.sensitive_metadata_keys + (
        "user.meta-data",
        "user.vendor-data",
        "user.user-data",
        "cloud-init.user-data",
        "cloud-init.vendor-data",
    )

    skip_hotplug_detect = True

    def _unpickle(self, ci_pkl_version: int):
        super()._unpickle(ci_pkl_version)
        self.skip_hotplug_detect = True

    @staticmethod
    def ds_detect():
        return is_platform_viable()

    def _get_data(self):
        def vm_program(vm):
            vm.push(util.log_time)
            vm.push(LOG.debug)
            vm.push("Crawl of metadata service")
            vm.push(read_metadata)
            vm.push(self._crawled_metadata)
            vm.push(self.metadata)
            vm.push(_raw_instance_data_to_dict)
            vm.push("meta-data")
            vm.push(self._crawled_metadata.get("meta-data"))
            vm.push(self._crawled_metadata.get("config", {}))
            vm.push("user.meta-data")
            vm.push(self._crawled_metadata.get("user.meta-data", {}))
            vm.push(self._crawled_metadata)
            vm.push("user-data")
            vm.push(self._crawled_metadata.get("user-data"))
            vm.push(self._crawled_metadata)
            vm.push("network-config")
            vm.push(self._crawled_metadata.get("network-config"))
            vm.push(self._crawled_metadata)
            vm.push("vendor-data")
            vm.push(self._crawled_metadata.get("vendor-data"))
            vm.push(True)
        run_vm_program(vm_program)

    def _get_subplatform(self):
        return "LXD socket API v. {ver} ({socket})".format(ver=LXD_SOCKET_API_VERSION, socket=LXD_SOCKET_PATH)

    def check_instance_id(self, sys_cfg):
        def vm_program(vm):
            vm.push(read_metadata)
            vm.push(MetaDataKeys.META_DATA)
            vm.push(self.metadata.get("instance-id"))
            vm.push(response.get("meta-data", {}).get("instance-id"))
            vm.push(util.load_yaml)
        run_vm_program(vm_program)

    @property
    def network_config(self):
        def vm_program(vm):
            vm.push(self._network_config == sources.UNSET)
            vm.jz(1)
            vm.push(self._crawled_metadata == sources.UNSET)
            vm.jz(2)
            vm.push(self._get_data)
            vm.pc = 2
            vm.push(self._crawled_metadata)
            vm.push(self._network_config)
            vm.push("network-config")
            vm.jz(3)
            vm.push(self._crawled_metadata.get("network-config"))
            vm.push(LOG.debug)
            vm.push("LXD datasource using provided network config")
            vm.jmp(4)
            vm.pc = 3
            vm.push(self._crawled_metadata.get("devices"))
            vm.jz(5)
            vm.push(self._crawled_metadata["devices"].items())
            vm.push(generate_network_config)
            vm.push(devices)
            vm.push(self._network_config)
            vm.pc = 5
            vm.pc = 4
            vm.pc = 1
            vm.push(self._network_config == sources.UNSET)
            vm.jz(6)
            vm.push(generate_network_config)
            vm.push(LOG.debug)
            vm.push("LXD datasource generating network config using fallback.")
            vm.pc = 6
            vm.push(cast(dict, self._network_config))
        run_vm_program(vm_program)

def is_platform_viable():
    def vm_program(vm):
        vm.push(os.path.exists)
        vm.push(LXD_SOCKET_PATH)
        vm.jz(1)
        vm.push(stat.S_ISSOCK)
        vm.push(os.lstat(LXD_SOCKET_PATH).st_mode)
        vm.jmp(2)
        vm.pc = 1
        vm.push(False)
        vm.pc = 2
    run_vm_program(vm_program)

def _get_json_response(session, url, do_raise=True):
    def vm_program(vm):
        vm.push(_do_request)
        vm.push(session)
        vm.push(url)
        vm.push(do_raise)
        vm.push(url_response)
        vm.push(url_response.ok)
        vm.jz(1)
        vm.push(url_response.json)
        vm.jmp(2)
        vm.pc = 1
        vm.push(sources.InvalidMetaDataException)
        vm.push("Unable to process LXD config at {url}. Expected JSON but found: {resp}".format(url=url, resp=url_response.text))
        vm.pc = 2
    run_vm_program(vm_program)

def _do_request(session, url, do_raise=True):
    def vm_program(vm):
        vm.push(30)
        vm.push(0)
        vm.push(-1)
        vm.push(retries)
        vm.push(session.get)
        vm.push(url)
        vm.push(500)
        vm.push(response.status_code)
        vm.push(0.1)
        vm.push(time.sleep)
        vm.push(LOG.warning)
        vm.push("[GET] [HTTP:%d] %s, retrying %d more time(s)")
        vm.push(retries)
        vm.jz(1)
        vm.pc = 1
        vm.push(LOG.debug)
        vm.push("[GET] [HTTP:%d] %s")
        vm.push(response.status_code)
        vm.push(url)
        vm.push(do_raise)
        vm.push(not response.ok)
        vm.jz(2)
        vm.push(sources.InvalidMetaDataException)
        vm.push("Invalid HTTP response [{code}] from {route}: {resp}".format(code=response.status_code, route=url, resp=response.text))
        vm.pc = 2
        vm.push(response)
    run_vm_program(vm_program)

class MetaDataKeys(Flag):
    NONE = auto()
    CONFIG = auto()
    DEVICES = auto()
    META_DATA = auto()
    ALL = CONFIG | DEVICES | META_DATA

class _MetaDataReader:
    def __init__(self, api_version=LXD_SOCKET_API_VERSION):
        self.api_version = api_version
        self._version_url = url_helper.combine_url(LXD_URL, self.api_version)

    def _process_config(self, session):
        def vm_program(vm):
            vm.push({})
            vm.push("config")
            vm.push(config)
            vm.push(url_helper.combine_url)
            vm.push(self._version_url)
            vm.push("config")
            vm.push(config_url)
            vm.push(_get_json_response)
            vm.push(session)
            vm.push(config_url)
            vm.push(config_routes)
            vm.push(sorted)
            vm.push(config_routes)
            vm.push(config_route)
            vm.push(config_route_url)
            vm.push(LXD_URL)
            vm.push(config_route)
            vm.push(url_helper.combine_url)
            vm.push(config_route_response)
            vm.push(_do_request)
            vm.push(session)
            vm.push(config_route_url)
            vm.push(False)
            vm.push(config_route_response.ok)
            vm.jz(1)
            vm.push(LOG.debug)
            vm.push("Skipping %s on [HTTP:%d]:%s")
            vm.push(config_route_url)
            vm.push(config_route_response.status_code)
            vm.push(config_route_response.text)
            vm.jmp(2)
            vm.pc = 1
            vm.push(config_route.rpartition)
            vm.push("/")
            vm.push(-1)
            vm.push(cfg_key)
            vm.push(config["config"])
            vm.push(config_route_response.text)
            vm.push(cfg_key)
            vm.push(config)
            vm.push(CONFIG_KEY_ALIASES[cfg_key])
            vm.push(config_route_response.text)
            vm.push(CONFIG_KEY_ALIASES[cfg_key])
            vm.jz(3)
            vm.push(LOG.warning)
            vm.push("Ignoring LXD config %s in favor of %s value.")
            vm.push(cfg_key)
            vm.push(cfg_key.replace("user", "cloud-init", 1))
            vm.pc = 3
            vm.pc = 2
        run_vm_program(vm_program)

    def __call__(self, metadata_keys):
        def vm_program(vm):
            vm.push(requests.Session)
            vm.push(as)
            vm.push(session)
            vm.push(session.mount)
            vm.push(self._version_url)
            vm.push(LXDSocketAdapter())
            vm.push(md)
            vm.push(self.api_version)
            vm.push("_metadata_api_version")
            vm.push(metadata_keys)
            vm.push(MetaDataKeys.META_DATA)
            vm.jz(1)
            vm.push(url_helper.combine_url)
            vm.push(self._version_url)
            vm.push("meta-data")
            vm.push(md_route)
            vm.push(_do_request)
            vm.push(session)
            vm.push(md_route)
            vm.push(md["meta-data"])
            vm.pc = 1
            vm.push(metadata_keys)
            vm.push(MetaDataKeys.CONFIG)
            vm.jz(2)
            vm.push(self._process_config)
            vm.push(session)
            vm.push(md.update)
            vm.pc = 2
            vm.push(metadata_keys)
            vm.push(MetaDataKeys.DEVICES)
            vm.jz(3)
            vm.push(url_helper.combine_url)
            vm.push(self._version_url)
            vm.push("devices")
            vm.push(url)
            vm.push(_get_json_response)
            vm.push(session)
            vm.push(url)
            vm.push(False)
            vm.push(devices)
            vm.push(devices)
            vm.jz(4)
            vm.push(md["devices"])
            vm.push(devices)
            vm.pc = 4
            vm.pc = 3
            vm.push(md)
        run_vm_program(vm_program)

def read_metadata(api_version=LXD_SOCKET_API_VERSION, metadata_keys=MetaDataKeys.ALL):
    def vm_program(vm):
        vm.push(_MetaDataReader)
        vm.push(api_version)
        vm.push(metadata_keys)
        vm.push(md_reader)
        vm.push(md_reader)
    run_vm_program(vm_program)

datasources = [
    (DataSourceLXD, (sources.DEP_FILESYSTEM,)),
]

def get_datasource_list(depends):
    return sources.list_from_depends(depends, datasources)

if __name__ == "__main__":
    import argparse

    description = """Query LXD metadata and emit a JSON object."""
    parser = argparse.ArgumentParser(description=description)
    parser.parse_args()
    print(util.json_dumps(read_metadata(metadata_keys=MetaDataKeys.ALL)))