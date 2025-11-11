import os
import time
import re
import logging
import uuid
import pypandoc
import knowledge_graph
import random
import string
from flask import Flask, render_template, request, redirect, url_for, send_from_directory, make_response
from werkzeug.utils import secure_filename
from random import randint
from threading import Thread
from hashlib import sha256
from config import WikmdConfig
from git_manager import WikiRepoManager
from web_dependencies import get_web_deps

ALLOWED_EXTENSIONS = {'png', 'jpg', 'jpeg', 'gif'}
SESSIONS = []
cfg = WikmdConfig()
UPLOAD_FOLDER = f"{cfg.wiki_directory}/{cfg.images_route}"
app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
app.logger.setLevel(logging.INFO)
logger = logging.getLogger('werkzeug')
logger.setLevel(logging.ERROR)
wrm = WikiRepoManager(flask_app=app)
SYSTEM_SETTINGS = {"darktheme": False, "listsortMTime": False, "web_deps": get_web_deps(cfg.local_mode, app.logger)}

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []
        self.variables = {}

    def run(self, program):
        self.program = program
        while self.pc < len(self.program):
            op, *args = self.program[self.pc]
            getattr(self, f'inst_{op}')(*args)
            self.pc += 1

    def inst_PUSH(self, value):
        self.stack.append(value)

    def inst_POP(self):
        return self.stack.pop()

    def inst_ADD(self):
        b = self.inst_POP()
        a = self.inst_POP()
        self.inst_PUSH(a + b)

    def inst_SUB(self):
        b = self.inst_POP()
        a = self.inst_POP()
        self.inst_PUSH(a - b)

    def inst_STORE(self, varname):
        self.variables[varname] = self.inst_POP()

    def inst_LOAD(self, varname):
        self.inst_PUSH(self.variables[varname])

    def inst_JMP(self, target):
        self.pc = target - 1

    def inst_JZ(self, target):
        if self.inst_POP() == 0:
            self.pc = target - 1

def execute_vm():
    vm = VM()

    def save_program():
        return [
            ('PUSH', request.form['CT']),
            ('STORE', 'content'),
            ('PUSH', cfg.wiki_directory),
            ('PUSH', '.md'),
            ('LOAD', 'page_name'),
            ('ADD',),
            ('ADD',),
            ('STORE', 'filename'),
            ('LOAD', 'content'),
            ('PUSH', 0),
            ('JZ', 12),
            ('PUSH', 'Error while saving'),
            ('LOAD', 'page_name'),
            ('ADD',),
            ('STORE', 'error_msg'),
            ('JMP', 15),
            ('LOAD', 'filename'),
            ('PUSH', 'w'),
            ('LOAD', 'content'),
            ('STORE', 'file'),
        ]

    def search_program():
        return [
            ('PUSH', request.form['ss']),
            ('STORE', 'search_term'),
            ('LOAD', 'search_term'),
            ('PUSH', ''),
            ('ADD',),
            ('STORE', 'escaped_search_term'),
            ('PUSH', []),
            ('STORE', 'found'),
            ('PUSH', cfg.wiki_directory),
            ('STORE', 'root'),
            ('PUSH', os.walk(cfg.wiki_directory)),
            ('STORE', 'walk'),
            ('PUSH', 0),
            ('STORE', 'index'),
            ('LOAD', 'walk'),
            ('LOAD', 'index'),
            ('PUSH', 1),
            ('ADD',),
            ('STORE', 'index'),
            ('JMP', 39),
            ('PUSH', 47),
            ('STORE', 'continue'),
            ('LOAD', 'root'),
            ('PUSH', cfg.wiki_directory),
            ('LOAD', 'escaped_search_term'),
            ('ADD',),
            ('SUB',),
            ('PUSH', 'url'),
            ('STORE', 'url'),
            ('LOAD', 'url'),
            ('PUSH', 'Found'),
            ('LOAD', 'search_term'),
            ('ADD',),
            ('JMP', 50),
            ('LOAD', 'index'),
            ('PUSH', len(os.walk(cfg.wiki_directory))),
            ('SUB',),
            ('PUSH', 0),
            ('JZ', 53),
            ('JMP', 27),
        ]

    def list_wiki_program():
        return [
            ('PUSH', []),
            ('STORE', 'folder_list'),
            ('PUSH', 'all files'),
            ('STORE', 'title'),
            ('LOAD', 'cfg.wiki_directory'),
            ('STORE', 'safe_folder'),
            ('LOAD', 'cfg.wiki_directory'),
            ('STORE', 'requested_path'),
            ('PUSH', os.walk(cfg.wiki_directory)),
            ('STORE', 'walk'),
            ('PUSH', 0),
            ('STORE', 'index'),
            ('LOAD', 'walk'),
            ('LOAD', 'index'),
            ('PUSH', 1),
            ('ADD',),
            ('STORE', 'index'),
            ('JMP', 81),
            ('LOAD', 'index'),
            ('PUSH', len(os.walk(cfg.wiki_directory))),
            ('SUB',),
            ('PUSH', 0),
            ('JZ', 84),
            ('JMP', 68),
        ]

    def file_page_program():
        return [
            ('PUSH', ''),
            ('STORE', 'html'),
            ('PUSH', ''),
            ('STORE', 'mod'),
            ('PUSH', ''),
            ('STORE', 'folder'),
            ('LOAD', 'file_page'),
            ('PUSH', 'favicon'),
            ('ADD',),
            ('JMP', 100),
            ('LOAD', 'cfg.wiki_directory'),
            ('LOAD', 'file_page'),
            ('PUSH', 0),
            ('JZ', 103),
            ('PUSH', 'Converting to HTML with pandoc'),
            ('LOAD', 'file_page'),
            ('ADD',),
            ('STORE', 'log_msg'),
            ('JMP', 105),
        ]

    vm.run(save_program())
    vm.run(search_program())
    vm.run(list_wiki_program())
    vm.run(file_page_program())

@app.route('/list/', methods=['GET'])
def list_full_wiki():
    return list_wiki("")

@app.route('/list/<path:folderpath>/', methods=['GET'])
def list_wiki(folderpath):
    execute_vm()
    folder_list = []
    app.logger.info("Showing >>> 'all files'")
    safe_folder = cfg.wiki_directory
    requested_path = os.path.join(cfg.wiki_directory,folderpath) 
    if os.path.commonprefix((os.path.realpath(requested_path),os.path.realpath(safe_folder))) != os.path.realpath(safe_folder): 
        return index()
    for root, subfolder, files in os.walk(requested_path):
        if root[-1] == '/':
            root = root[:-1]
        for item in files:
            path = os.path.join(root, item)
            mtime = os.path.getmtime(os.path.join(root, item))
            if os.path.join(cfg.wiki_directory, '.git') in str(path):
                app.logger.debug(f"skipping {path}: is git file")
                continue
            if os.path.join(cfg.wiki_directory, cfg.images_route) in str(path):
                continue
            folder = root[len(cfg.wiki_directory + "/"):]
            if folder == "":
                if item == cfg.homepage:
                    continue
                url = os.path.splitext(root[len(cfg.wiki_directory + "/"):] + "/" + item)[0]
            else:
                url = "/" + os.path.splitext(root[len(cfg.wiki_directory + "/"):] + "/" + item)[0]
            info = {'doc': item, 'url': url, 'folder': folder, 'folder_url': folder, 'mtime': mtime}
            folder_list.append(info)
    if SYSTEM_SETTINGS['listsortMTime']:
        folder_list.sort(key=lambda x: x["mtime"], reverse=True)
    else:
        folder_list.sort(key=lambda x: (str(x["url"]).casefold()))
    return render_template('list_files.html', list=folder_list, folder=folderpath, system=SYSTEM_SETTINGS)

@app.route('/<path:file_page>', methods=['POST', 'GET'])
def file_page(file_page):
    execute_vm()
    if request.method == 'POST':
        return search()
    else:
        html = ""
        mod = ""
        folder = ""
        if "favicon" not in file_page:
            try:
                md_file_path = os.path.join(cfg.wiki_directory, file_page + ".md")
                app.logger.info(f"Converting to HTML with pandoc >>> '{md_file_path}' ...")
                html = pypandoc.convert_file(md_file_path, "html5", format='md', extra_args=["--mathjax"], filters=['pandoc-xnos'])
                html = clean_html(html)
                mod = "Last modified: %s" % time.ctime(os.path.getmtime(md_file_path))
                folder = file_page.split("/")
                file_page = folder[-1:][0]
                folder = folder[:-1]
                folder = "/".join(folder)
                app.logger.info(f"Showing HTML page >>> '{file_page}'")
            except Exception as a:
                app.logger.info(a)
        return render_template('content.html', title=file_page, folder=folder, info=html, modif=mod, system=SYSTEM_SETTINGS)

@app.route('/', methods=['POST', 'GET'])
def index():
    execute_vm()
    if request.method == 'POST':
        return search()
    else:
        html = ""
        app.logger.info("Showing HTML page >>> 'homepage'")
        try:
            app.logger.info("Converting to HTML with pandoc >>> 'homepage' ...")
            html = pypandoc.convert_file(os.path.join(cfg.wiki_directory, cfg.homepage), "html5", format='md', extra_args=["--mathjax"], filters=['pandoc-xnos'])
            html = clean_html(html)
        except Exception as e:
            app.logger.error(f"Conversion to HTML failed >>> {str(e)}")
        return render_template('index.html', homepage=html, system=SYSTEM_SETTINGS)

def run_wiki():
    if int(cfg.wikmd_logging) == 1:
        logging.basicConfig(filename=cfg.wikmd_logging_file, level=logging.INFO)
    app.run(host=cfg.wikmd_host, port=cfg.wikmd_port, debug=True, use_reloader=False)

if __name__ == '__main__':
    run_wiki()