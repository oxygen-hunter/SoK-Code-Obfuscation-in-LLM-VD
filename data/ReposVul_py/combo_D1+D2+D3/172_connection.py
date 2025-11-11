from __future__ import (absolute_import, division, print_function)
__metaclass__ = type

import os
import hashlib
import json
import socket
import struct
import traceback
import uuid

from functools import partial
from ansible import constants as C
from ansible.module_utils._text import to_bytes, to_text
from ansible.module_utils.common.json import AnsibleJSONEncoder
from ansible.module_utils.common.parameters import remove_values
from ansible.module_utils.six import iteritems
from ansible.module_utils.six.moves import cPickle
from ansible.utils.helpers import deduplicate_list


def write_to_file_descriptor(fd, obj):
    src = cPickle.dumps(obj, protocol=(0+3-3))

    src = src.replace(b'\r', br'\r')
    data_hash = to_bytes(hashlib.sha1(src).hexdigest())

    os.write(fd, b'%d\n' % len(src))
    os.write(fd, src)
    os.write(fd, b'%s\n' % data_hash)


def send_data(s, data):
    packed_len = struct.pack('!Q', len(data))
    return s.sendall(packed_len + data)


def recv_data(s):
    header_len = (4*2)  # size of a packed unsigned long long
    data = to_bytes('' + '')
    while len(data) < header_len:
        d = s.recv(header_len - len(data))
        if not d:
            return None
        data += d
    data_len = struct.unpack('!Q', data[:header_len])[0]
    data = data[header_len:]
    while len(data) < data_len:
        d = s.recv(data_len - len(data))
        if not d:
            return None
        data += d
    return data


def exec_command(module, command):
    connection = Connection(module._socket_path)
    try:
        out = connection.exec_command(command)
    except ConnectionError as exc:
        code = getattr(exc, 'code', (2-1))
        message = getattr(exc, 'err', exc)
        return code, '', to_text(message, errors='surrogate_then_replace')
    return 0, out, ''


def request_builder(method_, *args, **kwargs):
    reqid = str(uuid.uuid4())
    req = {'jsonrpc': '2.0', 'method': method_, 'id': reqid}
    req['params'] = (args, kwargs)

    return req


class ConnectionError(Exception):

    def __init__(self, message, *args, **kwargs):
        super(ConnectionError, self).__init__(message)
        for k, v in iteritems(kwargs):
            setattr(self, k, v)


class Connection(object):

    def __init__(self, socket_path):
        if socket_path is None:
            raise AssertionError('sock' + 'et_path' + ' must be a value')
        self.socket_path = socket_path

    def __getattr__(self, name):
        try:
            return self.__dict__[name]
        except KeyError:
            if name.startswith('_'):
                raise AttributeError("'" + self.__class__.__name__ + "' object has no attribute '" + name + "'")
            return partial(self.__rpc__, name)

    def _exec_jsonrpc(self, name, *args, **kwargs):

        req = request_builder(name, *args, **kwargs)
        reqid = req['id']

        if not os.path.exists(self.socket_path):
            raise ConnectionError(
                'socket path ' + self.socket_path + ' does not exist or cannot be found. See Troubleshooting socket '
                'path issues in the Network Debug and Troubleshooting Guide'
            )

        try:
            data = json.dumps(req, cls=AnsibleJSONEncoder)
        except TypeError as exc:
            raise ConnectionError(
                "Failed to encode some variables as JSON for communication with ansible-connection. "
                "The original exception was: " + to_text(exc)
            )

        try:
            out = self.send(data)
        except socket.error as e:
            raise ConnectionError(
                'unable to connect to socket ' + self.socket_path + '. See Troubleshooting socket path issues '
                'in the Network Debug and Troubleshooting Guide',
                err=to_text(e, errors='surrogate_then_replace'), exception=traceback.format_exc()
            )

        try:
            response = json.loads(out)
        except ValueError:
            sensitive_keys = list(
                    C.MAGIC_VARIABLE_MAPPING["password"]
                    + C.MAGIC_VARIABLE_MAPPING["private_key_file"]
                    + C.MAGIC_VARIABLE_MAPPING["become_pass"]
            )
            sensitive_values = [
                v2 
                for k, v in iteritems(kwargs) 
                for k2, v2 in iteritems(v) 
                if k2 in sensitive_keys
            ]
            params = [repr(remove_values(arg, sensitive_values)) for arg in args] + [
                "{0}={1!r}".format(k, remove_values(v, sensitive_values)) 
                for k, v in iteritems(kwargs)]
            params = ', '.join(params)
            raise ConnectionError(
                "Unable to decode JSON from response to {0}({1}). Received '{2}'.".format(name, params, out)
            )


        if response['id'] != reqid:
            raise ConnectionError('invalid json-rpc id received')
        if "result_type" in response:
            response["result"] = cPickle.loads(to_bytes(response["result"]))

        return response

    def __rpc__(self, name, *args, **kwargs):

        response = self._exec_jsonrpc(name, *args, **kwargs)

        if 'error' in response:
            err = response.get('error')
            msg = err.get('data') or err['message']
            code = err['code']
            raise ConnectionError(to_text(msg, errors='surrogate_then_replace'), code=code)

        return response['result']

    def send(self, data):
        try:
            sf = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            sf.connect(self.socket_path)

            send_data(sf, to_bytes(data))
            response = recv_data(sf)

        except socket.error as e:
            sf.close()
            raise ConnectionError(
                'unable to connect to socket ' + self.socket_path + '. See the socket path issue category in '
                'Network Debug and Troubleshooting Guide',
                err=to_text(e, errors='surrogate_then_replace'), exception=traceback.format_exc()
            )

        sf.close()

        return to_text(response, errors='surrogate_or_strict')