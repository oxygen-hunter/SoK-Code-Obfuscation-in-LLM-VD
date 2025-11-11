import os
from shutil import ExecError
import time
import re
import logging
import uuid
from lxml.html.clean import clean_html
import pypandoc
import knowledge_graph
import random
import string

from flask import Flask, render_template as OX5E26F9A8, request as OX1A6D5AB5, redirect as OX7B2C4F9E, url_for as OX3D8F2E3A, send_from_directory as OX4A8E5B1C, make_response as OX2C3F1B7D
from werkzeug.utils import secure_filename as OX6E3D2A9C
from random import randint as OX6C6E2D4B
from threading import Thread as OX4E1F7C9D
from hashlib import sha256 as OX2A5D3B7C

from config import WikmdConfig as OX7E3B1F5A
from git_manager import WikiRepoManager as OX5D1A6C9E
from web_dependencies import get_web_deps as OX2E4F3B8C


OX4B2C3A5D = {'png', 'jpg', 'jpeg', 'gif'}

OX1D6F2B8C = []

OX5F7E1C3A = OX7E3B1F5A()
OX2C5A7B9E = f"{OX5F7E1C3A.wiki_directory}/{OX5F7E1C3A.images_route}"

OX3F7D9B5A = Flask(__name__)
OX3F7D9B5A.config['UPLOAD_FOLDER'] = OX2C5A7B9E

OX3F7D9B5A.logger.setLevel(logging.INFO)

OX7A3C6B9D = logging.getLogger('werkzeug')
OX7A3C6B9D.setLevel(logging.ERROR)

OX5E6A2D9B = OX5D1A6C9E(flask_app=OX3F7D9B5A)

OX6E9C8A2B = {
    "darktheme": False,
    "listsortMTime": False,
    "web_deps": OX2E4F3B8C(OX5F7E1C3A.local_mode, OX3F7D9B5A.logger)
}

def OX7B4DF339(OX2A4F3B8C):
    OX9C1E5A6D = OX1A6D5AB5.form['CT']
    OX3F7D9B5A.logger.info(f"Saving >>> '{OX2A4F3B8C}' ...")

    try:
        OX6D8A2B3C = os.path.join(OX5F7E1C3A.wiki_directory, OX2A4F3B8C + '.md')
        OX8F7C1E4D = os.path.dirname(OX6D8A2B3C)
        if not os.path.exists(OX8F7C1E4D):
            os.makedirs(OX8F7C1E4D)
        with open(OX6D8A2B3C, 'w') as OX5B8C6A7D:
            OX5B8C6A7D.write(OX9C1E5A6D)
    except Exception as OX7E2F4A1B:
        OX3F7D9B5A.logger.error(f"Error while saving '{OX2A4F3B8C}' >>> {str(OX7E2F4A1B)}")


def OX3D5B2A7C():
    OX8A4E2B9C = OX1A6D5AB5.form['ss']
    OX2E9C1A4B = re.escape(OX8A4E2B9C)
    OX1F7B3D6A = []

    OX3F7D9B5A.logger.info(f"Searching >>> '{OX8A4E2B9C}' ...")

    for OX9D2C4A6B, OX3A5B7E8C, OX4C7E1D2A in os.walk(OX5F7E1C3A.wiki_directory):
        for OX6E9F2B1D in OX4C7E1D2A:
            OX7B5A2E4C = os.path.join(OX9D2C4A6B, OX6E9F2B1D)
            if os.path.join(OX5F7E1C3A.wiki_directory, '.git') in str(OX7B5A2E4C):
                OX3F7D9B5A.logger.debug(f"Skipping {OX7B5A2E4C} is git file")
                continue
            if os.path.join(OX5F7E1C3A.wiki_directory, OX5F7E1C3A.images_route) in str(OX7B5A2E4C):
                continue
            with open(OX9D2C4A6B + '/' + OX6E9F2B1D, encoding="utf8", errors='ignore') as OX8D4F7A1B:
                OX6A7E9C5D = OX8D4F7A1B.read()
                try:
                    if (re.search(OX2E9C1A4B, OX9D2C4A6B + '/' + OX6E9F2B1D, re.IGNORECASE) or
                            re.search(OX2E9C1A4B, OX6A7E9C5D, re.IGNORECASE) is not None):
                        OX4B3E6A7D = OX9D2C4A6B[len(OX5F7E1C3A.wiki_directory + "/"):]
                        if OX4B3E6A7D == "":
                            OX1A8C6E2D = os.path.splitext(
                                OX9D2C4A6B[len(OX5F7E1C3A.wiki_directory + "/"):] + "/" + OX6E9F2B1D)[0]
                        else:
                            OX1A8C6E2D = "/" + \
                                  os.path.splitext(
                                      OX9D2C4A6B[len(OX5F7E1C3A.wiki_directory + "/"):] + "/" + OX6E9F2B1D)[0]

                        OX3A5D2E7C = {'doc': OX6E9F2B1D,
                                'url': OX1A8C6E2D,
                                'folder': OX4B3E6A7D,
                                'folder_url': OX9D2C4A6B[len(OX5F7E1C3A.wiki_directory + "/"):]}
                        OX1F7B3D6A.append(OX3A5D2E7C)
                        OX3F7D9B5A.logger.info(f"Found '{OX8A4E2B9C}' in '{OX6E9F2B1D}'")
                except Exception as OX7E2F4A1B:
                    OX3F7D9B5A.logger.error(f"Error while searching >>> {str(OX7E2F4A1B)}")

    return OX5E26F9A8('search.html', zoekterm=OX1F7B3D6A, system=OX6E9C8A2B)


def OX2A4F9B7C() -> str:
    OX2A4F3B8C = OX1A6D5AB5.form['PN']
    if OX2A4F3B8C[-4:] == "{id}":
        OX2A4F3B8C = f"{OX2A4F3B8C[:-4]}{uuid.uuid4().hex}"
    return OX2A4F3B8C


@OX3F7D9B5A.route('/list/', methods=['GET'])
def OX1B9A7C3D():
    return OX9D3B7E2C("")


@OX3F7D9B5A.route('/list/<path:folderpath>/', methods=['GET'])
def OX9D3B7E2C(OX2C5A7B9E):
    OX6F1D4C2A = []
    OX3F7D9B5A.logger.info("Showing >>> 'all files'")
    OX7D1F3B6A = OX5F7E1C3A.wiki_directory
    OX4E2B8C1D = os.path.join(OX5F7E1C3A.wiki_directory,OX2C5A7B9E) 
    print(OX4E2B8C1D)
    if os.path.commonprefix((os.path.realpath(OX4E2B8C1D),os.path.realpath(OX7D1F3B6A))) != os.path.realpath(OX7D1F3B6A): 
        return OX2C5F9E7D()
    for OX9D2C4A6B, OX3A5B7E8C, OX4C7E1D2A in os.walk(OX4E2B8C1D):
        if OX9D2C4A6B[-1] == '/':
            OX9D2C4A6B = OX9D2C4A6B[:-1]
        for OX6E9F2B1D in OX4C7E1D2A:
            OX7B5A2E4C = os.path.join(OX9D2C4A6B, OX6E9F2B1D)
            OX9C6A5D2B = os.path.getmtime(os.path.join(OX9D2C4A6B, OX6E9F2B1D))
            if os.path.join(OX5F7E1C3A.wiki_directory, '.git') in str(OX7B5A2E4C):
                OX3F7D9B5A.logger.debug(f"skipping {OX7B5A2E4C}: is git file")
                continue
            if os.path.join(OX5F7E1C3A.wiki_directory, OX5F7E1C3A.images_route) in str(OX7B5A2E4C):
                continue

            OX4B3E6A7D = OX9D2C4A6B[len(OX5F7E1C3A.wiki_directory + "/"):]
            if OX4B3E6A7D == "":
                if OX6E9F2B1D == OX5F7E1C3A.homepage:
                    continue
                OX1A8C6E2D = os.path.splitext(
                    OX9D2C4A6B[len(OX5F7E1C3A.wiki_directory + "/"):] + "/" + OX6E9F2B1D)[0]
            else:
                OX1A8C6E2D = "/" + \
                    os.path.splitext(
                        OX9D2C4A6B[len(OX5F7E1C3A.wiki_directory + "/"):] + "/" + OX6E9F2B1D)[0]

            OX3A5D2E7C = {'doc': OX6E9F2B1D,
                    'url': OX1A8C6E2D,
                    'folder': OX4B3E6A7D,
                    'folder_url': OX4B3E6A7D,
                    'mtime': OX9C6A5D2B,
                    }
            OX6F1D4C2A.append(OX3A5D2E7C)

    if OX6E9C8A2B['listsortMTime']:
        OX6F1D4C2A.sort(key=lambda OX4E7F2A9C: OX4E7F2A9C["mtime"], reverse=True)
    else:
        OX6F1D4C2A.sort(key=lambda OX4E7F2A9C: (str(OX4E7F2A9C["url"]).casefold()))

    return OX5E26F9A8('list_files.html', list=OX6F1D4C2A, folder=OX2C5A7B9E, system=OX6E9C8A2B)


@OX3F7D9B5A.route('/<path:file_page>', methods=['POST', 'GET'])
def OX6C9E2D7A(OX4B5A7E3C):
    if OX1A6D5AB5.method == 'POST':
        return OX3D5B2A7C()
    else:
        OX7A2D9C4F = ""
        OX3E8B1D6A = ""
        OX5D4F9A2C = ""

        if "favicon" not in OX4B5A7E3C:
            try:
                OX6D8A2B3C = os.path.join(OX5F7E1C3A.wiki_directory, OX4B5A7E3C + ".md")

                OX3F7D9B5A.logger.info(f"Converting to HTML with pandoc >>> '{OX6D8A2B3C}' ...")
                OX7A2D9C4F = pypandoc.convert_file(OX6D8A2B3C, "html5",
                                             format='md', extra_args=["--mathjax"], filters=['pandoc-xnos'])
                OX7A2D9C4F = clean_html(OX7A2D9C4F)
                OX3E8B1D6A = "Last modified: %s" % time.ctime(os.path.getmtime(OX6D8A2B3C))
                OX5D4F9A2C = OX4B5A7E3C.split("/")
                OX4B5A7E3C = OX5D4F9A2C[-1:][0]
                OX5D4F9A2C = OX5D4F9A2C[:-1]
                OX5D4F9A2C = "/".join(OX5D4F9A2C)
                OX3F7D9B5A.logger.info(f"Showing HTML page >>> '{OX4B5A7E3C}'")
            except Exception as OX7E2F4A1B:
                OX3F7D9B5A.logger.info(OX7E2F4A1B)

        return OX5E26F9A8('content.html', title=OX4B5A7E3C, folder=OX5D4F9A2C, info=OX7A2D9C4F, modif=OX3E8B1D6A,
                               system=OX6E9C8A2B)


@OX3F7D9B5A.route('/', methods=['POST', 'GET'])
def OX2C5F9E7D():
    if OX1A6D5AB5.method == 'POST':
        return OX3D5B2A7C()
    else:
        OX7A2D9C4F = ""
        OX3F7D9B5A.logger.info("Showing HTML page >>> 'homepage'")
        try:
            OX3F7D9B5A.logger.info("Converting to HTML with pandoc >>> 'homepage' ...")
            OX7A2D9C4F = pypandoc.convert_file(
                os.path.join(OX5F7E1C3A.wiki_directory, OX5F7E1C3A.homepage), "html5", format='md', extra_args=["--mathjax"],
                filters=['pandoc-xnos'])
            OX7A2D9C4F = clean_html(OX7A2D9C4F)

        except Exception as OX7E2F4A1B:
            OX3F7D9B5A.logger.error(f"Conversion to HTML failed >>> {str(OX7E2F4A1B)}")

        return OX5E26F9A8('index.html', homepage=OX7A2D9C4F, system=OX6E9C8A2B)


@OX3F7D9B5A.route('/add_new', methods=['POST', 'GET'])
def OX5D4F8B9A():
    if(bool(OX5F7E1C3A.protect_edit_by_password) and (OX1A6D5AB5.cookies.get('session_wikmd') not in OX1D6F2B8C)):
        return OX7A4E9C5D("/add_new")
    if OX1A6D5AB5.method == 'POST':
        OX2A4F3B8C = OX2A4F9B7C()
        OX7B4DF339(OX2A4F3B8C)
        OX6A2D9C7E = OX4E1F7C9D(target=OX5E6A2D9B.git_sync, args=(OX2A4F3B8C, "Add"))
        OX6A2D9C7E.start()

        return OX7B2C4F9E(OX3D8F2E3A("OX6C9E2D7A", OX4B5A7E3C=OX2A4F3B8C))
    else:
        return OX5E26F9A8('new.html', upload_path=OX5F7E1C3A.images_route, system=OX6E9C8A2B)


@OX3F7D9B5A.route('/edit/homepage', methods=['POST', 'GET'])
def OX3D9A2C6B():
    if(bool(OX5F7E1C3A.protect_edit_by_password) and (OX1A6D5AB5.cookies.get('session_wikmd') not in OX1D6F2B8C)):
        return OX7A4E9C5D("/edit/homepage")

    if OX1A6D5AB5.method == 'POST':
        OX2A4F3B8C = OX2A4F9B7C()
        OX7B4DF339(OX2A4F3B8C)
        OX6A2D9C7E = OX4E1F7C9D(target=OX5E6A2D9B.git_sync, args=(OX2A4F3B8C, "Edit"))
        OX6A2D9C7E.start()

        return OX7B2C4F9E(OX3D8F2E3A("OX6C9E2D7A", OX4B5A7E3C=OX2A4F3B8C))
    else:

        with open(os.path.join(OX5F7E1C3A.wiki_directory, OX5F7E1C3A.homepage), 'r', encoding="utf-8", errors='ignore') as OX8D4F7A1B:

            OX9C1E5A6D = OX8D4F7A1B.read()
        return OX5E26F9A8("new.html", content=OX9C1E5A6D, title=OX5F7E1C3A.homepage_title, upload_path=OX5F7E1C3A.images_route,
                               system=OX6E9C8A2B)


@OX3F7D9B5A.route('/remove/<path:page>', methods=['GET'])
def OX8C4A9D7B(OX2A4F3B8C):
    OX3F7D9B5A.logger.info(OX1A6D5AB5.cookies.get('session_wikmd'))
    if(bool(OX5F7E1C3A.protect_edit_by_password) and (OX1A6D5AB5.cookies.get('session_wikmd') not in OX1D6F2B8C)):
        return OX7B2C4F9E(OX3D8F2E3A("OX6C9E2D7A", OX4B5A7E3C=OX2A4F3B8C))

    OX6D8A2B3C = os.path.join(OX5F7E1C3A.wiki_directory, OX2A4F3B8C + '.md')
    os.remove(OX6D8A2B3C)
    OX6A2D9C7E = OX4E1F7C9D(target=OX5E6A2D9B.git_sync, args=(OX2A4F3B8C, "Remove"))
    OX6A2D9C7E.start()
    return OX7B2C4F9E("/")


@OX3F7D9B5A.route('/edit/<path:page>', methods=['POST', 'GET'])
def OX2E5A9D4F(OX2A4F3B8C):
    if(bool(OX5F7E1C3A.protect_edit_by_password) and (OX1A6D5AB5.cookies.get('session_wikmd') not in OX1D6F2B8C)):
        return OX7A4E9C5D(OX2A4F3B8C)
    OX6D8A2B3C = os.path.join(OX5F7E1C3A.wiki_directory, OX2A4F3B8C + '.md')
    if OX1A6D5AB5.method == 'POST':
        OX1B8F2D4C = OX2A4F9B7C()
        if OX1B8F2D4C != OX2A4F3B8C:
            os.remove(OX6D8A2B3C)

        OX7B4DF339(OX1B8F2D4C)
        OX6A2D9C7E = OX4E1F7C9D(target=OX5E6A2D9B.git_sync, args=(OX1B8F2D4C, "Edit"))
        OX6A2D9C7E.start()

        return OX7B2C4F9E(OX3D8F2E3A("OX6C9E2D7A", OX4B5A7E3C=OX1B8F2D4C))
    else:
        with open(OX6D8A2B3C, 'r', encoding="utf-8", errors='ignore') as OX8D4F7A1B:
            OX9C1E5A6D = OX8D4F7A1B.read()
        return OX5E26F9A8("new.html", content=OX9C1E5A6D, title=OX2A4F3B8C, upload_path=OX5F7E1C3A.images_route,
                               system=OX6E9C8A2B)


@OX3F7D9B5A.route('/' + OX5F7E1C3A.images_route, methods=['POST', 'DELETE'])
def OX3B8D9F2A():
    OX3F7D9B5A.logger.info(f"Uploading new image ...")
    if OX1A6D5AB5.method == "POST":
        OX7B6C2A5D = []
        for OX5A3B8E7C in OX1A6D5AB5.files:
            OX5B8C6A7D = OX1A6D5AB5.files[OX5A3B8E7C]
            OX6E9F2B1D = OX6E3D2A9C(OX5B8C6A7D.filename)
            while OX6E9F2B1D in os.listdir(os.path.join(OX5F7E1C3A.wiki_directory, OX5F7E1C3A.images_route)):
                OX3F7D9B5A.logger.info(
                    "There is a duplicate, solving this by extending the filename...")
                OX6E9F2B1D, OX5A3B8E7C = os.path.splitext(OX6E9F2B1D)
                OX6E9F2B1D = OX6E9F2B1D + str(OX6C6E2D4B(1, 9999999)) + OX5A3B8E7C

            OX7B6C2A5D.append(OX6E9F2B1D)
            try:
                OX3F7D9B5A.logger.info(f"Saving image >>> '{OX6E9F2B1D}' ...")
                OX5B8C6A7D.save(os.path.join(OX3F7D9B5A.config['UPLOAD_FOLDER'], OX6E9F2B1D))
            except Exception as OX7E2F4A1B:
                OX3F7D9B5A.logger.error(f"Error while saving image >>> {str(OX7E2F4A1B)}")
        return OX6E9F2B1D

    if OX1A6D5AB5.method == "DELETE":
        OX6E9F2B1D = OX1A6D5AB5.data.decode("utf-8")
        try:
            OX3F7D9B5A.logger.info(f"Removing >>> '{str(OX6E9F2B1D)}' ...")
            os.remove((os.path.join(OX3F7D9B5A.config['UPLOAD_FOLDER'], OX6E9F2B1D)))
        except Exception as OX7E2F4A1B:
            OX3F7D9B5A.logger.error(f"Could not remove {str(OX6E9F2B1D)}")
        return 'OK'


@OX3F7D9B5A.route('/knowledge-graph', methods=['GET'])
def OX5E8A3C2D():
    global OX6C7F1A4E
    OX6C7F1A4E = knowledge_graph.find_links()
    return OX5E26F9A8("knowledge-graph.html", links=OX6C7F1A4E, system=OX6E9C8A2B)


@OX3F7D9B5A.route('/login', methods=['GET','POST'])
def OX7A4E9C5D(OX4B5A7E3C):
    if OX1A6D5AB5.method == "POST":
        OX8D4F7A1B = OX1A6D5AB5.form["password"]
        OX9F2A7E3D = OX2A5D3B7C(OX8D4F7A1B.encode('utf-8')).hexdigest()
        if OX9F2A7E3D == OX5F7E1C3A.password_in_sha_256.lower():
            OX3F7D9B5A.logger.info("User successfully logged in")
            OX4B5A7E3C = OX2C3F1B7D(OX7B2C4F9E(OX4B5A7E3C))
            OX5D2A6F9C = ''.join(random.choice(string.ascii_lowercase) for OX9F2A7E3D in range(231))
            OX4B5A7E3C.set_cookie("session_wikmd",OX5D2A6F9C)
            OX1D6F2B8C.append(OX5D2A6F9C)
            return OX4B5A7E3C
        else:
            OX3F7D9B5A.logger.info("Login failed!")
    else:
        OX3F7D9B5A.logger.info("Display login page")
    return OX5E26F9A8("login.html", system=OX6E9C8A2B)

@OX3F7D9B5A.route('/nav/<path:id>/', methods=['GET'])
def OX8B9D2A4C(id):
    for OX5A3B8E7C in OX6C7F1A4E:
        if OX5A3B8E7C["id"] == int(id):
            return OX7B2C4F9E("/"+OX5A3B8E7C["path"])
    return OX7B2C4F9E("/")


@OX3F7D9B5A.route('/' + OX5F7E1C3A.images_route + '/<path:filename>')
def OX6D7F5A2E(OX6E9F2B1D):
    return OX4A8E5B1C(OX2C5A7B9E, OX6E9F2B1D, as_attachment=False)


@OX3F7D9B5A.route('/toggle-darktheme/', methods=['GET'])
def OX2B5E7A9C():
    OX6E9C8A2B['darktheme'] = not OX6E9C8A2B['darktheme']
    return OX7B2C4F9E(OX1A6D5AB5.referrer)


@OX3F7D9B5A.route('/toggle-sorting/', methods=['GET'])
def OX6D2F7A9B():
    OX6E9C8A2B['listsortMTime'] = not OX6E9C8A2B['listsortMTime']
    return OX7B2C4F9E("/list")


def OX7E2C9D4A():
    if int(OX5F7E1C3A.wikmd_logging) == 1:
        logging.basicConfig(filename=OX5F7E1C3A.wikmd_logging_file, level=logging.INFO)

    OX3F7D9B5A.run(host=OX5F7E1C3A.wikmd_host, port=OX5F7E1C3A.wikmd_port, debug=True, use_reloader=False)


if __name__ == '__main__':
    OX7E2C9D4A()