import logging
from logging.handlers import RotatingFileHandler
import os
import random
import re

from flask import Flask, render_template, request
from flask_wtf import FlaskForm
from wtforms import SelectField, TextAreaField

from .generate.generator import PoemMaker

o, p, q = PoemMaker(), Flask(__name__), os.urandom(32)
o.setup()
p.config['SECRET_KEY'] = q

r = RotatingFileHandler('poems.log', maxBytes=10000, backupCount=1)
formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
r.setLevel(logging.INFO)
r.setFormatter(formatter)
p.logger.addHandler(r)

def alphanum(s):
    return re.sub(r'[^a-z]+', '', s.lower())

class GeneratePoemForm(FlaskForm):
    style = SelectField('Style', choices=[(k, k) for k in o.poem_styles.keys()])
    source = SelectField('Source', choices=[(k, k) for k in o.text_sources.keys()])

class UploadTextForm(FlaskForm):
    source_text = TextAreaField('Text', render_kw={'rows': 20, 'cols': 200})
    poem_format = SelectField('Format', choices=[(k, k) for k in o.poem_styles.keys()])

def a(param, d):
    if param is not None:
        for k in d.keys():
            if alphanum(param) == alphanum(k):
                return k

def b(form, q, o, p, a):
    p.logger.debug(form.validate())
    if form.errors:
        p.logger.warning(form.errors)
    if form.validate_on_submit():
        q.extend([form.source.data, form.style.data])
    else:
        try:
            e, f = request.args.get('source') or request.args.get('style'), a(e, o.text_sources)
            q.append(f if f is not None else random.choice(list(o.text_sources.keys())))
            form.source.data = q[0]
            e, f = request.args.get('poem') or request.args.get('style'), a(e, o.poem_styles)
            q.append(f if f is not None else random.choice(list(o.poem_styles.keys())))
            form.style.data = q[1]
        except:
            p.logger.exception('Failed to select source and style')

@p.route('/', methods=['GET', 'POST'])
def generate_page():
    q = []
    form = GeneratePoemForm()
    b(form, q, o, p, a)
    poem = o.generate(q[0], q[1])
    p.logger.info(poem)
    print(poem)
    return render_template('generate.html', form=form, poem=poem)

@p.route('/custom', methods=['GET', 'POST'])
def upload():
    form = UploadTextForm()
    if form.validate_on_submit():
        print('asdf')
        g, h = form.source_text.data, form.poem_format.data
        form.source_text.render_kw['hidden'] = True
        try:
            poem = o.generate_custom(g, h)
            p.logger.info(poem)
            print(poem)
        except IndexError:
            poem="Sorry! I couldn't find a valid poem with that input. :("
        return render_template('custom_poem.html', form=form, poem=poem)
    if 'hidden' in form.source_text.render_kw:
        del form.source_text.render_kw['hidden']
    return render_template('custom.html', form=form)

if __name__ == '__main__':
    p.run()