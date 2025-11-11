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

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            op = instr[0]
            args = instr[1:]
            if op == 'PUSH':
                self.stack.append(args[0])
            elif op == 'POP':
                self.stack.pop()
            elif op == 'ADD':
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif op == 'SUB':
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif op == 'JMP':
                self.pc = args[0] - 1
            elif op == 'JZ':
                if self.stack.pop() == 0:
                    self.pc = args[0] - 1
            elif op == 'LOAD':
                self.stack.append(args[0])
            elif op == 'STORE':
                args[0] = self.stack.pop()
            self.pc += 1

@ns_conf.route('/')
class FilesList(Resource):
    @ns_conf.doc('get_files_list')
    def get(self):
        v = VM()
        instr = [
            ('PUSH', ca.file_controller),
            ('LOAD', 'get_files'),
            ('STORE', ca.file_controller.get_files),
            ('PUSH', ca.file_controller.get_files),
            ('CALL',),
            ('RETURN',)
        ]
        v.load_instructions(instr)
        v.run()
        return v.stack.pop()

@ns_conf.route('/<name>')
@ns_conf.param('name', "MindsDB's name for file")
class File(Resource):
    @ns_conf.doc('put_file')
    def put(self, name: str):
        v = VM()
        data = {}
        mindsdb_file_name = name
        existing_file_names = ca.file_controller.get_files_names()

        def on_field(field):
            name = field.field_name.decode()
            value = field.value.decode()
            data[name] = value

        file_object = None

        def on_file(file):
            nonlocal file_object
            data['file'] = file.file_name.decode()
            file_object = file.file_object

        temp_dir_path = tempfile.mkdtemp(prefix='mindsdb_file_')

        def process_multipart():
            parser = multipart.create_form_parser(
                headers=request.headers,
                on_field=on_field,
                on_file=on_file,
                config={
                    'UPLOAD_DIR': temp_dir_path.encode(),
                    'UPLOAD_KEEP_FILENAME': True,
                    'UPLOAD_KEEP_EXTENSIONS': True,
                    'MAX_MEMORY_FILE_SIZE': 0
                }
            )
            while True:
                chunk = request.stream.read(8192)
                if not chunk:
                    break
                parser.write(chunk)
            parser.finalize()
            parser.close()
            if file_object is not None and not file_object.closed:
                file_object.close()

        def process_json():
            nonlocal data
            data = request.json

        instr = [
            ('PUSH', request.headers['Content-Type']),
            ('LOAD', 'multipart/form-data'),
            ('EQ',),
            ('JZ', 5),
            ('CALL', process_multipart),
            ('JMP', 6),
            ('CALL', process_json),
        ]
        v.load_instructions(instr)
        v.run()

        instr = [
            ('PUSH', mindsdb_file_name),
            ('PUSH', existing_file_names),
            ('IN',),
            ('JZ', 4),
            ('PUSH', {
                'status': 400,
                'detail': "File already exists",
                'message': f"File with name '{data['file']}' already exists"
            }),
            ('RETURN',),
        ]
        v.load_instructions(instr)
        v.run()
        if v.stack:
            return http_error(**v.stack.pop())

        if data.get('source_type') == 'url':
            url = data['source']
            data['file'] = data['name']
            config = Config()
            is_cloud = config.get('cloud', False)
            if is_cloud is True and ctx.user_class != 1:
                info = requests.head(url)
                file_size = info.headers.get('Content-Length')
                try:
                    file_size = int(file_size)
                except Exception:
                    pass
                if file_size is None:
                    return http_error(
                        400,
                        "Error getting file info",
                        "Can't determine remote file size"
                    )
                if file_size > 1024 * 1024 * 100:
                    return http_error(
                        400,
                        "File is too big",
                        "Upload limit for file is 100Mb"
                    )
            with requests.get(url, stream=True) as r:
                if r.status_code != 200:
                    return http_error(
                        400,
                        "Error getting file",
                        f"Got status code: {r.status_code}"
                    )
                file_path = os.path.join(temp_dir_path, data['file'])
                with open(file_path, 'wb') as f:
                    for chunk in r.iter_content(chunk_size=8192):
                        f.write(chunk)

        original_file_name = data.get('original_file_name')
        file_path = os.path.join(temp_dir_path, data['file'])
        lp = file_path.lower()
        if lp.endswith(('.zip', '.tar.gz')):
            if lp.endswith('.zip'):
                with zipfile.ZipFile(file_path) as f:
                    f.extractall(temp_dir_path)
            elif lp.endswith('.tar.gz'):
                with tarfile.open(file_path) as f:
                    safe_extract(f, temp_dir_path)
            os.remove(file_path)
            files = os.listdir(temp_dir_path)
            if len(files) != 1:
                os.rmdir(temp_dir_path)
                return http_error(400, 'Wrong content.', 'Archive must contain only one data file.')
            file_path = os.path.join(temp_dir_path, files[0])
            mindsdb_file_name = files[0]
            if not os.path.isfile(file_path):
                os.rmdir(temp_dir_path)
                return http_error(400, 'Wrong content.', 'Archive must contain data file in root.')

        ca.file_controller.save_file(mindsdb_file_name, file_path, file_name=original_file_name)
        os.rmdir(temp_dir_path)
        return '', 200

    @ns_conf.doc('delete_file')
    def delete(self, name: str):
        v = VM()
        instr = [
            ('PUSH', ca.file_controller),
            ('LOAD', 'delete_file'),
            ('STORE', ca.file_controller.delete_file),
            ('PUSH', ca.file_controller.delete_file),
            ('PUSH', name),
            ('CALL',),
            ('PUSH', 200),
            ('RETURN',)
        ]
        try:
            v.load_instructions(instr)
            v.run()
            return '', v.stack.pop()
        except Exception as e:
            log.logger.error(e)
            return http_error(
                400,
                "Error deleting file",
                f"There was an error while trying to delete file with name '{name}'"
            )