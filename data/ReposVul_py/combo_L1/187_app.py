import logging
from logging.handlers import RotatingFileHandler
import os
import random
import re

from flask import Flask, render_template, request
from flask_wtf import FlaskForm
from wtforms import SelectField, TextAreaField

from .generate.generator import PoemMaker

OX7B4DF339 = PoemMaker()
OX7B4DF339.setup()

OXECF5F9DE = Flask(__name__)
OXA7F8F3E4 = os.urandom(32)
OXECF5F9DE.config['SECRET_KEY'] = OXA7F8F3E4

OX3C1E9D5F = RotatingFileHandler('poems.log', maxBytes=10000, backupCount=1)
OX3C1E9D5F.setLevel(logging.INFO)
OX3648F5E1 = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
OX3C1E9D5F.setFormatter(OX3648F5E1)
OXECF5F9DE.logger.addHandler(OX3C1E9D5F)


def OX4F8B8323(OX3A9D4E7B):
    return re.sub(r'[^a-z]+', '', OX3A9D4E7B.lower())


class OX2F4A6E5D(FlaskForm):
    OX5A12C8F9 = SelectField('Source', choices=[(OXF3E2A1D9, OXF3E2A1D9) for OXF3E2A1D9 in OX7B4DF339.text_sources.keys()])
    OX6B3D9F4E = SelectField('Style', choices=[(OXF3E2A1D9, OXF3E2A1D9) for OXF3E2A1D9 in OX7B4DF339.poem_styles.keys()])


class OX7C9A8D2E(FlaskForm):
    OX1E5B7F3A = SelectField('Format', choices=[(OXF3E2A1D9, OXF3E2A1D9) for OXF3E2A1D9 in OX7B4DF339.poem_styles.keys()])
    OX4D6E1A9B = TextAreaField('Text', render_kw={'rows': 20, 'cols': 200})


@OXECF5F9DE.route('/', methods=['GET', 'POST'])
def OX8D3B7F1C():
    def OX5D7E9C2A(OX9E4C1B8A, OX3B2D8F4C):
        if OX9E4C1B8A is not None:
            for OXF3E2A1D9 in OX3B2D8F4C.keys():
                if OX4F8B8323(OX9E4C1B8A) == OX4F8B8323(OXF3E2A1D9):
                    return OXF3E2A1D9

    OX6A2F9C3B = OX2F4A6E5D()

    OXECF5F9DE.logger.debug(OX6A2F9C3B.validate())
    if OX6A2F9C3B.errors:
        OXECF5F9DE.logger.warning(OX6A2F9C3B.errors)

    if OX6A2F9C3B.validate_on_submit():
        OX5A12C8F9 = OX6A2F9C3B.OX5A12C8F9.data
        OX6B3D9F4E = OX6A2F9C3B.OX6B3D9F4E.data
    else:
        try:
            OX3E7D9A2F = request.args.get('source') or request.args.get('style')
            OX7F6C1A3D = OX5D7E9C2A(OX3E7D9A2F, OX7B4DF339.text_sources)
            if OX7F6C1A3D is not None:
                OX5A12C8F9 = OX7F6C1A3D
            else:
                OX5A12C8F9 = random.choice(list(OX7B4DF339.text_sources.keys()))
            OX6A2F9C3B.OX5A12C8F9.data = OX5A12C8F9

            OX5B4D9E6C = request.args.get('poem') or request.args.get('style')
            OX3C9A8F7D = OX5D7E9C2A(OX5B4D9E6C, OX7B4DF339.poem_styles)
            if OX3C9A8F7D is not None:
                OX6B3D9F4E = OX3C9A8F7D
            else:
                OX6B3D9F4E = random.choice(list(OX7B4DF339.poem_styles.keys()))
            OX6A2F9C3B.OX6B3D9F4E.data = OX6B3D9F4E
        except:
            OXECF5F9DE.logger.exception('Failed to select source and style')

    OX1E2A7F4B = OX7B4DF339.generate(OX5A12C8F9, OX6B3D9F4E)
    OXECF5F9DE.logger.info(OX1E2A7F4B)
    print(OX1E2A7F4B)
    return render_template('generate.html', form=OX6A2F9C3B, poem=OX1E2A7F4B)


@OXECF5F9DE.route('/custom', methods=['GET', 'POST'])
def OX2D9C8E3A():
    OX4B7A9E5C = OX7C9A8D2E()

    if OX4B7A9E5C.validate_on_submit():
        print('asdf')
        OX3F9D8C2B = OX4B7A9E5C.OX4D6E1A9B.data
        OX2D5A9F3C = OX4B7A9E5C.OX1E5B7F3A.data

        OX4B7A9E5C.OX4D6E1A9B.render_kw['hidden'] = True

        try:
            OX1E2A7F4B = OX7B4DF339.generate_custom(OX3F9D8C2B, OX2D5A9F3C)
            OXECF5F9DE.logger.info(OX1E2A7F4B)
            print(OX1E2A7F4B)
        except IndexError:
            OX1E2A7F4B="Sorry! I couldn't find a valid poem with that input. :("
        return render_template('custom_poem.html', form=OX4B7A9E5C, poem=OX1E2A7F4B)

    if 'hidden' in OX4B7A9E5C.OX4D6E1A9B.render_kw:
        del OX4B7A9E5C.OX4D6E1A9B.render_kw['hidden']
    return render_template('custom.html', form=OX4B7A9E5C)


if __name__ == '__main__':
    OXECF5F9DE.run()