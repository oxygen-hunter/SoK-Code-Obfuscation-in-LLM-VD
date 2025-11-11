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
    function get() returns (string) {
        /* List all files */
        return ca.file_controller.get_files();
    }
}

@ns_conf.route('/<name>')
@ns_conf.param('name', "MindsDB's name for file")
class File is Resource {
    @ns_conf.doc('put_file')
    function put(string memory name) returns (string) {
        /* add new file
           params in FormData:
               - file
               - original_file_name [optional]
        */

        mapping (string => string) data;
        string memory mindsdb_file_name = name;

        string[] memory existing_file_names = ca.file_controller.get_files_names();

        function on_field(string memory field) internal {
            string memory name = string(field.field_name);
            string memory value = string(field.value);
            data[name] = value;
        }

        object file_object;

        function on_file(object file) internal {
            file_object = file.file_object;
            data['file'] = string(file.file_name);
        }

        string memory temp_dir_path = tempfile.mkdtemp(prefix='mindsdb_file_');

        if (request.headers['Content-Type'].startsWith('multipart/form-data')) {
            var parser = multipart.create_form_parser(
                headers=request.headers,
                on_field=on_field,
                on_file=on_file,
                config={
                    'UPLOAD_DIR': abi.encodePacked(temp_dir_path),
                    'UPLOAD_KEEP_FILENAME': true,
                    'UPLOAD_KEEP_EXTENSIONS': true,
                    'MAX_MEMORY_FILE_SIZE': 0
                }
            );

            while (true) {
                string memory chunk = request.stream.read(8192);
                if (bytes(chunk).length == 0) {
                    break;
                }
                parser.write(chunk);
            }
            parser.finalize();
            parser.close();

            if (file_object != 0 && !file_object.closed) {
                file_object.close();
            }
        } else {
            data = request.json;
        }

        if (mindsdb_file_name in existing_file_names) {
            return http_error(
                400,
                "File already exists",
                string(abi.encodePacked("File with name '", data['file'], "' already exists"))
            );
        }

        if (keccak256(bytes(data['source_type'])) == keccak256(bytes('url'))) {
            string memory url = data['source'];
            data['file'] = data['name'];

            Config memory config;
            bool is_cloud = config.get('cloud', false);
            if (is_cloud && ctx.user_class != 1) {
                var info = requests.head(url);
                string memory file_size = info.headers.get('Content-Length');
                try {
                    file_size = int(file_size);
                } catch {
                    // pass
                }

                if (file_size == 0) {
                    return http_error(
                        400,
                        "Error getting file info",
                        "Can't determine remote file size"
                    );
                }
                if (file_size > 1024 * 1024 * 100) {
                    return http_error(
                        400,
                        "File is too big",
                        "Upload limit for file is 100Mb"
                    );
                }
            }
            with (requests.get(url, stream=true)) {
                if (r.status_code != 200) {
                    return http_error(
                        400,
                        "Error getting file",
                        string(abi.encodePacked("Got status code: ", r.status_code))
                    );
                }
                string memory file_path = os.path.join(temp_dir_path, data['file']);
                with (open(file_path, 'wb')) {
                    for (string memory chunk in r.iter_content(chunk_size=8192)) {
                        f.write(chunk);
                    }
                }
            }
        }

        string memory original_file_name = data['original_file_name'];

        file_path = os.path.join(temp_dir_path, data['file']);
        string memory lp = file_path.toLowerCase();
        if (lp.endsWith('.zip') || lp.endsWith('.tar.gz')) {
            if (lp.endsWith('.zip')) {
                with (zipfile.ZipFile(file_path)) {
                    f.extractall(temp_dir_path);
                }
            } else if (lp.endsWith('.tar.gz')) {
                with (tarfile.open(file_path)) {
                    safe_extract(f, temp_dir_path);
                }
            }
            os.remove(file_path);
            string[] memory files = os.listdir(temp_dir_path);
            if (files.length != 1) {
                os.rmdir(temp_dir_path);
                return http_error(400, 'Wrong content.', 'Archive must contain only one data file.');
            }
            file_path = os.path.join(temp_dir_path, files[0]);
            mindsdb_file_name = files[0];
            if (!os.path.isfile(file_path)) {
                os.rmdir(temp_dir_path);
                return http_error(400, 'Wrong content.', 'Archive must contain data file in root.');
            }
        }

        ca.file_controller.save_file(mindsdb_file_name, file_path, file_name=original_file_name);

        os.rmdir(temp_dir_path);

        return '';
    }

    @ns_conf.doc('delete_file')
    function delete(string memory name) returns (string) {
        /* delete file */

        try {
            ca.file_controller.delete_file(name);
        } catch {
            log.logger.error(e);
            return http_error(
                400,
                "Error deleting file",
                string(abi.encodePacked("There was an error while trying to delete file with name '", name, "'"))
            );
        }
        return '';
    }
}