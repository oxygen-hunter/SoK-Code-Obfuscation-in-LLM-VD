import logging
from logging.handlers import RotatingFileHandler
import os
import random
import re

from flask import Flask, render_template, request
from flask_wtf import FlaskForm
from wtforms import SelectField, TextAreaField

from .generate.generator import PoemMaker

pm = PoemMaker()
pm.setup()

app = Flask(__name__)
SECRET_KEY = os.urandom(32)
app.config['SECRET_KEY'] = SECRET_KEY

handler = RotatingFileHandler('poems.log', maxBytes=10000, backupCount=1)
handler.setLevel(logging.INFO)
formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
handler.setFormatter(formatter)
app.logger.addHandler(handler)

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            if instr[0] == 'PUSH':
                self.stack.append(instr[1])
            elif instr[0] == 'POP':
                self.stack.pop()
            elif instr[0] == 'ADD':
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif instr[0] == 'SUB':
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif instr[0] == 'JMP':
                self.pc = instr[1] - 1
            elif instr[0] == 'JZ':
                v = self.stack.pop()
                if v == 0:
                    self.pc = instr[1] - 1
            elif instr[0] == 'LOAD':
                self.stack.append(instr[1])
            elif instr[0] == 'STORE':
                value = self.stack.pop()
                setattr(self, instr[1], value)
            self.pc += 1

def alphanum_vm(vm, s):
    vm.instructions = [
        ('LOAD', s),
        ('STORE', 'input_str'),
        ('LOAD', ''),
        ('STORE', 'result'),
        ('LOAD', 0),
        ('STORE', 'i'),
        ('JMP', 9),
        ('LOAD', vm.input_str),
        ('PUSH', lambda x: x.lower()),
        ('PUSH', re.sub),
        ('PUSH', r'[^a-z]+'),
        ('PUSH', ''),
        ('PUSH', vm.i),
        ('ADD',),
        ('SUB',),
        ('JZ', 15),
        ('LOAD', vm.result),
        ('PUSH', ' + vm.input_str[i]'),
        ('ADD',),
        ('STORE', 'result'),
        ('LOAD', vm.i),
        ('PUSH', 1),
        ('ADD',),
        ('STORE', 'i'),
        ('JMP', 10),
        ('LOAD', vm.result),
        ('POP',)
    ]
    vm.run()
    return vm.stack.pop()

class GeneratePoemForm(FlaskForm):
    source = SelectField('Source', choices=[(k, k) for k in pm.text_sources.keys()])
    style = SelectField('Style', choices=[(k, k) for k in pm.poem_styles.keys()])

class UploadTextForm(FlaskForm):
    poem_format = SelectField('Format', choices=[(k, k) for k in pm.poem_styles.keys()])
    source_text = TextAreaField('Text', render_kw={'rows': 20, 'cols': 200})

@app.route('/', methods=['GET', 'POST'])
def generate_page():
    def valid_param(param, d):
        if param is not None:
            for k in d.keys():
                if alphanum_vm(VM(), param) == alphanum_vm(VM(), k):
                    return k

    form = GeneratePoemForm()

    app.logger.debug(form.validate())
    if form.errors:
        app.logger.warning(form.errors)

    if form.validate_on_submit():
        source = form.source.data
        style = form.style.data
    else:
        try:
            source_ask = request.args.get('source') or request.args.get('style')
            source_param = valid_param(source_ask, pm.text_sources)
            if source_param is not None:
                source = source_param
            else:
                source = random.choice(list(pm.text_sources.keys()))
            form.source.data = source

            style_ask = request.args.get('poem') or request.args.get('style')
            style_param = valid_param(style_ask, pm.poem_styles)
            if style_param is not None:
                style = style_param
            else:
                style = random.choice(list(pm.poem_styles.keys()))
            form.style.data = style
        except:
            app.logger.exception('Failed to select source and style')

    poem = pm.generate(source, style)
    app.logger.info(poem)
    print(poem)
    return render_template('generate.html', form=form, poem=poem)

@app.route('/custom', methods=['GET', 'POST'])
def upload():
    form = UploadTextForm()

    if form.validate_on_submit():
        print('asdf')
        source_text = form.source_text.data
        poem_format = form.poem_format.data

        form.source_text.render_kw['hidden'] = True

        try:
            poem = pm.generate_custom(source_text, poem_format)
            app.logger.info(poem)
            print(poem)
        except IndexError:
            poem="Sorry! I couldn't find a valid poem with that input. :("
        return render_template('custom_poem.html', form=form, poem=poem)

    if 'hidden' in form.source_text.render_kw:
        del form.source_text.render_kw['hidden']
    return render_template('custom.html', form=form)

if __name__ == '__main__':
    app.run()