import os
import zipfile
import tarfile

from flask import request, current_app as ca
from flask_restx import Resource
import tempfile
import multipart
import requests

from mindsdb.utilities import log
from mindsdb.api.http.utils import http_error, safe_extract
from mindsdb.api.http.namespaces.configs.files import ns_conf
from mindsdb.utilities.config import Config
from mindsdb.utilities.context import context as ctx

@ns_conf.route('/')
class FilesList(Resource):
    @ns_conf.doc('get_files_list')
    def get(self):
        '''List all files'''
        return ca.file_controller.get_files()

@ns_conf.route('/<name>')
@ns_conf.param('name', "MindsDB's name for file")
class File(Resource):
    @ns_conf.doc('put_file')
    def put(self, p: str):
        ''' add new file
            params in FormData:
                - file
                - original_file_name [optional]
        '''
        data = {}
        h = p
        s = ca.file_controller.get_files_names()

        def on_field(g):
            n, v = g.field_name.decode(), g.value.decode()
            data[n] = v

        g = None

        def on_file(f):
            nonlocal g
            data['file'] = f.file_name.decode()
            g = f.file_object

        q = tempfile.mkdtemp(prefix='mindsdb_file_')

        if request.headers['Content-Type'].startswith('multipart/form-data'):
            parser = multipart.create_form_parser(
                headers=request.headers,
                on_field=on_field,
                on_file=on_file,
                config={
                    'UPLOAD_DIR': q.encode(),
                    'UPLOAD_KEEP_FILENAME': True,
                    'UPLOAD_KEEP_EXTENSIONS': True,
                    'MAX_MEMORY_FILE_SIZE': 0
                }
            )

            while True:
                u = request.stream.read(8192)
                if not u:
                    break
                parser.write(u)
            parser.finalize()
            parser.close()

            if g is not None and not g.closed:
                g.close()
        else:
            data = request.json

        if h in s:
            return http_error(
                400,
                "File already exists",
                f"File with name '{data['file']}' already exists"
            )

        if data.get('source_type') == 'url':
            j = data['source']
            data['file'] = data['name']

            config = Config()
            is_cloud = config.get('cloud', False)
            if is_cloud is True and ctx.user_class != 1:
                info = requests.head(j)
                k = info.headers.get('Content-Length')
                try:
                    k = int(k)
                except Exception:
                    pass

                if k is None:
                    return http_error(
                        400,
                        "Error getting file info",
                        "Сan't determine remote file size"
                    )
                if k > 1024 * 1024 * 100:
                    return http_error(
                        400,
                        "File is too big",
                        "Upload limit for file is 100Mb"
                    )
            with requests.get(j, stream=True) as r:
                if r.status_code != 200:
                    return http_error(
                        400,
                        "Error getting file",
                        f"Got status code: {r.status_code}"
                    )
                file_path = os.path.join(q, data['file'])
                with open(file_path, 'wb') as f:
                    for u in r.iter_content(chunk_size=8192):
                        f.write(u)

        t = data.get('original_file_name')

        file_path = os.path.join(q, data['file'])
        v = file_path.lower()
        if v.endswith(('.zip', '.tar.gz')):
            if v.endswith('.zip'):
                with zipfile.ZipFile(file_path) as f:
                    f.extractall(q)
            elif v.endswith('.tar.gz'):
                with tarfile.open(file_path) as f:
                    safe_extract(f, q)
            os.remove(file_path)
            files = os.listdir(q)
            if len(files) != 1:
                os.rmdir(q)
                return http_error(400, 'Wrong content.', 'Archive must contain only one data file.')
            file_path = os.path.join(q, files[0])
            h = files[0]
            if not os.path.isfile(file_path):
                os.rmdir(q)
                return http_error(400, 'Wrong content.', 'Archive must contain data file in root.')

        ca.file_controller.save_file(h, file_path, file_name=t)

        os.rmdir(q)

        return '', 200

    @ns_conf.doc('delete_file')
    def delete(self, p: str):
        '''delete file'''

        try:
            ca.file_controller.delete_file(p)
        except Exception as e:
            log.logger.error(e)
            return http_error(
                400,
                "Error deleting file",
                f"There was an error while tring to delete file with name '{p}'"
            )
        return '', 200