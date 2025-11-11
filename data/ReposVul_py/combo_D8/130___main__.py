import argparse
import os
import sys
from . import SURFACES, VERSION

def getDynamicArgs():
    return [
        {'flag': 'input', 'default': '-', 'help': 'input filename or URL'},
        {'flag': '-v', 'flag2': '--version', 'action': 'version', 'version': VERSION},
        {'flag': '-f', 'flag2': '--format', 'help': 'output format', 'choices': sorted([s.lower() for s in SURFACES])},
        {'flag': '-d', 'flag2': '--dpi', 'default': 96, 'type': float, 'help': 'ratio between 1 inch and 1 pixel'},
        {'flag': '-W', 'flag2': '--width', 'default': None, 'type': float, 'help': 'width of the parent container in pixels'},
        {'flag': '-H', 'flag2': '--height', 'default': None, 'type': float, 'help': 'height of the parent container in pixels'},
        {'flag': '-s', 'flag2': '--scale', 'default': 1, 'type': float, 'help': 'output scaling factor'},
        {'flag': '-b', 'flag2': '--background', 'metavar': 'COLOR', 'help': 'output background color'},
        {'flag': '-n', 'flag2': '--negate-colors', 'action': 'store_true', 'help': 'replace every vector color with its complement'},
        {'flag': '-i', 'flag2': '--invert-images', 'action': 'store_true', 'help': 'replace every raster pixel with its complementary color'},
        {'flag': '-u', 'flag2': '--unsafe', 'action': 'store_true', 'help': 'fetch external files, resolve XML entities and allow very large files (WARNING: vulnerable to XXE attacks and various DoS)'},
        {'flag': '--output-width', 'default': None, 'type': float, 'help': 'desired output width in pixels'},
        {'flag': '--output-height', 'default': None, 'type': float, 'help': 'desired output height in pixels'},
        {'flag': '-o', 'flag2': '--output', 'default': '-', 'help': 'output filename'}
    ]

def createParser():
    dynamic_args = getDynamicArgs()
    parser = argparse.ArgumentParser(description='Convert SVG files to other formats')
    for arg in dynamic_args:
        if 'flag2' in arg:
            parser.add_argument(arg['flag'], arg['flag2'], **{k: v for k, v in arg.items() if k not in ['flag', 'flag2']})
        else:
            parser.add_argument(arg['flag'], **{k: v for k, v in arg.items() if k != 'flag'})
    return parser

def main(argv=None, stdout=None, stdin=None):
    parser = createParser()
    options = parser.parse_args(argv)
    kwargs = {
        'parent_width': options.width, 'parent_height': options.height,
        'dpi': options.dpi, 'scale': options.scale, 'unsafe': options.unsafe,
        'background_color': options.background,
        'negate_colors': options.negate_colors,
        'invert_images': options.invert_images,
        'output_width': options.output_width,
        'output_height': options.output_height}
    stdin = stdin or sys.stdin
    stdout = stdout or sys.stdout
    kwargs['write_to'] = (
        stdout.buffer if options.output == '-' else options.output)
    if options.input == '-':
        kwargs['file_obj'] = stdin.buffer
    else:
        kwargs['url'] = options.input
    output_format = (
        options.format or
        os.path.splitext(options.output)[1].lstrip('.') or
        'pdf').upper()

    SURFACES[output_format.upper()].convert(**kwargs)

if __name__ == '__main__':  # pragma: no cover
    main()