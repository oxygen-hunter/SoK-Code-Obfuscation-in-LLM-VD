import os as OX7E88CBA3
import zipfile as OX9E1D11A1
import tarfile as OX1B9F5F6D

from flask import request as OX6D98A056, current_app as OX7F85C800
from flask_restx import Resource as OX6F6B6BF9
import tempfile as OX6F3F3C1F
import multipart as OX2CCF6C8C
import requests as OX7B9E0A97

from mindsdb.utilities import log as OX3C1B3E2C
from mindsdb.api.http.utils import http_error as OXB7D5BC90, safe_extract as OX9D8B9A03
from mindsdb.api.http.namespaces.configs.files import ns_conf as OX3F2C9BFA
from mindsdb.utilities.config import Config as OX5D8E3B4D
from mindsdb.utilities.context import context as OX4A3A8F2D


@OX3F2C9BFA.route('/')
class OX6FDF6C7B(OX6F6B6BF9):
    @OX3F2C9BFA.doc('get_files_list')
    def OX9C3C3FFD(self):
        return OX7F85C800.file_controller.get_files()


@OX3F2C9BFA.route('/<name>')
@OX3F2C9BFA.param('name', "MindsDB's name for file")
class OX3B9F1A3A(OX6F6B6BF9):
    @OX3F2C9BFA.doc('put_file')
    def OXA1B6D8C2(self, OXDFD9E5A6: str):
        OX1A2B3C4D = {}
        OX8B9A0C1D = OXDFD9E5A6

        OX6C1D2E3F = OX7F85C800.file_controller.get_files_names()

        def OX9F8A7B6C(OX6A7B8C9D):
            OX1F2E3D4C = OX6A7B8C9D.field_name.decode()
            OX2D3C4B5A = OX6A7B8C9D.value.decode()
            OX1A2B3C4D[OX1F2E3D4C] = OX2D3C4B5A

        OX4C5B6A7D = None

        def OX8C9D0E1F(OX5A6B7C8D):
            nonlocal OX4C5B6A7D
            OX1A2B3C4D['file'] = OX5A6B7C8D.file_name.decode()
            OX4C5B6A7D = OX5A6B7C8D.file_object

        OX3D4C5B6A = OX6F3F3C1F.mkdtemp(prefix='mindsdb_file_')

        if OX6D98A056.headers['Content-Type'].startswith('multipart/form-data'):
            OX6B7A8C9D = OX2CCF6C8C.create_form_parser(
                headers=OX6D98A056.headers,
                on_field=OX9F8A7B6C,
                on_file=OX8C9D0E1F,
                config={
                    'UPLOAD_DIR': OX3D4C5B6A.encode(),
                    'UPLOAD_KEEP_FILENAME': True,
                    'UPLOAD_KEEP_EXTENSIONS': True,
                    'MAX_MEMORY_FILE_SIZE': 0
                }
            )

            while True:
                OX1B2A3C4D = OX6D98A056.stream.read(8192)
                if not OX1B2A3C4D:
                    break
                OX6B7A8C9D.write(OX1B2A3C4D)
            OX6B7A8C9D.finalize()
            OX6B7A8C9D.close()

            if OX4C5B6A7D is not None and not OX4C5B6A7D.closed:
                OX4C5B6A7D.close()
        else:
            OX1A2B3C4D = OX6D98A056.json

        if OX8B9A0C1D in OX6C1D2E3F:
            return OXB7D5BC90(
                400,
                "File already exists",
                f"File with name '{OX1A2B3C4D['file']}' already exists"
            )

        if OX1A2B3C4D.get('source_type') == 'url':
            OX7E8D9A0B = OX1A2B3C4D['source']
            OX1A2B3C4D['file'] = OX1A2B3C4D['name']

            OX2D3C4B5A = OX5D8E3B4D()
            OX4A5B6C7D = OX2D3C4B5A.get('cloud', False)
            if OX4A5B6C7D is True and OX4A3A8F2D.user_class != 1:
                OX6C7B8A9D = OX7B9E0A97.head(OX7E8D9A0B)
                OX5B6A7C8D = OX6C7B8A9D.headers.get('Content-Length')
                try:
                    OX5B6A7C8D = int(OX5B6A7C8D)
                except Exception:
                    pass

                if OX5B6A7C8D is None:
                    return OXB7D5BC90(
                        400,
                        "Error getting file info",
                        "Сan't determine remote file size"
                    )
                if OX5B6A7C8D > 1024 * 1024 * 100:
                    return OXB7D5BC90(
                        400,
                        "File is too big",
                        "Upload limit for file is 100Mb"
                    )
            with OX7B9E0A97.get(OX7E8D9A0B, stream=True) as r:
                if r.status_code != 200:
                    return OXB7D5BC90(
                        400,
                        "Error getting file",
                        f"Got status code: {r.status_code}"
                    )
                OX5A6B7C8D = OX7E88CBA3.path.join(OX3D4C5B6A, OX1A2B3C4D['file'])
                with open(OX5A6B7C8D, 'wb') as f:
                    for OX6B7A8C9D in r.iter_content(chunk_size=8192):
                        f.write(OX6B7A8C9D)

        OX9A8B7C6D = OX1A2B3C4D.get('original_file_name')

        OX5A6B7C8D = OX7E88CBA3.path.join(OX3D4C5B6A, OX1A2B3C4D['file'])
        OX5E6F7A8D = OX5A6B7C8D.lower()
        if OX5E6F7A8D.endswith(('.zip', '.tar.gz')):
            if OX5E6F7A8D.endswith('.zip'):
                with OX9E1D11A1.ZipFile(OX5A6B7C8D) as f:
                    f.extractall(OX3D4C5B6A)
            elif OX5E6F7A8D.endswith('.tar.gz'):
                with OX1B9F5F6D.open(OX5A6B7C8D) as f:
                    OX9D8B9A03(f, OX3D4C5B6A)
            OX7E88CBA3.remove(OX5A6B7C8D)
            OX7C8B9A0D = OX7E88CBA3.listdir(OX3D4C5B6A)
            if len(OX7C8B9A0D) != 1:
                OX7E88CBA3.rmdir(OX3D4C5B6A)
                return OXB7D5BC90(400, 'Wrong content.', 'Archive must contain only one data file.')
            OX5A6B7C8D = OX7E88CBA3.path.join(OX3D4C5B6A, OX7C8B9A0D[0])
            OX8B9A0C1D = OX7C8B9A0D[0]
            if not OX7E88CBA3.path.isfile(OX5A6B7C8D):
                OX7E88CBA3.rmdir(OX3D4C5B6A)
                return OXB7D5BC90(400, 'Wrong content.', 'Archive must contain data file in root.')

        OX7F85C800.file_controller.save_file(OX8B9A0C1D, OX5A6B7C8D, file_name=OX9A8B7C6D)

        OX7E88CBA3.rmdir(OX3D4C5B6A)

        return '', 200

    @OX3F2C9BFA.doc('delete_file')
    def OX5B6C7A8D(self, OXDFD9E5A6: str):
        try:
            OX7F85C800.file_controller.delete_file(OXDFD9E5A6)
        except Exception as OXE8F7A9C6:
            OX3C1B3E2C.logger.error(OXE8F7A9C6)
            return OXB7D5BC90(
                400,
                "Error deleting file",
                f"There was an error while tring to delete file with name '{OXDFD9E5A6}'"
            )
        return '', 200