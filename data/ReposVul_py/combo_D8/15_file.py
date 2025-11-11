import os
import zipfile
import tarfile

from flask import request, current_app as _0
from flask_restx import Resource
import tempfile
import multipart
import requests

from mindsdb.utilities import log
from mindsdb.api.http.utils import http_error as _1, safe_extract as _2
from mindsdb.api.http.namespaces.configs.files import ns_conf
from mindsdb.utilities.config import Config as _3
from mindsdb.utilities.context import context as _4


@ns_conf.route('/')
class _5(Resource):
    @ns_conf.doc('get_files_list')
    def get(self):
        return _0.file_controller.get_files()


@ns_conf.route('/<name>')
@ns_conf.param('name', "MindsDB's name for file")
class _6(Resource):
    @ns_conf.doc('put_file')
    def put(self, _7: str):
        _8 = {}
        _9 = _7

        _10 = _0.file_controller.get_files_names()

        def _11(_12):
            _13 = _12.field_name.decode()
            _14 = _12.value.decode()
            _8[_13] = _14

        _15 = None

        def _16(_17):
            nonlocal _15
            _8['file'] = _17.file_name.decode()
            _15 = _17.file_object

        _18 = tempfile.mkdtemp(prefix='mindsdb_file_')

        if request.headers['Content-Type'].startswith('multipart/form-data'):
            _19 = multipart.create_form_parser(
                headers=request.headers,
                on_field=_11,
                on_file=_16,
                config={
                    'UPLOAD_DIR': _18.encode(),
                    'UPLOAD_KEEP_FILENAME': True,
                    'UPLOAD_KEEP_EXTENSIONS': True,
                    'MAX_MEMORY_FILE_SIZE': 0
                }
            )

            while True:
                _20 = request.stream.read(8192)
                if not _20:
                    break
                _19.write(_20)
            _19.finalize()
            _19.close()

            if _15 is not None and not _15.closed:
                _15.close()
        else:
            _8 = request.json

        if _9 in _10:
            return _1(
                400,
                "File already exists",
                f"File with name '{_8['file']}' already exists"
            )

        if _8.get('source_type') == 'url':
            _21 = _8['source']
            _8['file'] = _8['name']

            _22 = _3()
            _23 = _22.get('cloud', False)
            if _23 is True and _4.user_class != 1:
                _24 = requests.head(_21)
                _25 = _24.headers.get('Content-Length')
                try:
                    _25 = int(_25)
                except Exception:
                    pass

                if _25 is None:
                    return _1(
                        400,
                        "Error getting file info",
                        "Сan't determine remote file size"
                    )
                if _25 > 1024 * 1024 * 100:
                    return _1(
                        400,
                        "File is too big",
                        "Upload limit for file is 100Mb"
                    )
            with requests.get(_21, stream=True) as _26:
                if _26.status_code != 200:
                    return _1(
                        400,
                        "Error getting file",
                        f"Got status code: {_26.status_code}"
                    )
                _27 = os.path.join(_18, _8['file'])
                with open(_27, 'wb') as _28:
                    for _29 in _26.iter_content(chunk_size=8192):
                        _28.write(_29)

        _30 = _8.get('original_file_name')

        _27 = os.path.join(_18, _8['file'])
        _31 = _27.lower()
        if _31.endswith(('.zip', '.tar.gz')):
            if _31.endswith('.zip'):
                with zipfile.ZipFile(_27) as _32:
                    _32.extractall(_18)
            elif _31.endswith('.tar.gz'):
                with tarfile.open(_27) as _33:
                    _2(_33, _18)
            os.remove(_27)
            _34 = os.listdir(_18)
            if len(_34) != 1:
                os.rmdir(_18)
                return _1(400, 'Wrong content.', 'Archive must contain only one data file.')
            _27 = os.path.join(_18, _34[0])
            _9 = _34[0]
            if not os.path.isfile(_27):
                os.rmdir(_18)
                return _1(400, 'Wrong content.', 'Archive must contain data file in root.')

        _0.file_controller.save_file(_9, _27, file_name=_30)

        os.rmdir(_18)

        return '', 200

    @ns_conf.doc('delete_file')
    def delete(self, _7: str):
        try:
            _0.file_controller.delete_file(_7)
        except Exception as _35:
            log.logger.error(_35)
            return _1(
                400,
                "Error deleting file",
                f"There was an error while tring to delete file with name '{_7}'"
            )
        return '', 200