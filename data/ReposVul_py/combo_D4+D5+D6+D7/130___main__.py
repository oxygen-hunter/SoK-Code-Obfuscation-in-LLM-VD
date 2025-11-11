import argparse
import os
import sys

from . import SURFACES, VERSION

def main(_0=None, _1=None, _2=None):
    _3 = argparse.ArgumentParser(description='Convert SVG files to other formats')
    _3.add_argument('input', default='-', help='input filename or URL')
    _3.add_argument('-v', '--version', action='version', version=VERSION)
    _3.add_argument('-f', '--format', help='output format', choices=sorted([surface.lower() for surface in SURFACES]))
    _3.add_argument('-d', '--dpi', default=96, type=float, help='ratio between 1 inch and 1 pixel')
    _3.add_argument('-W', '--width', default=None, type=float, help='width of the parent container in pixels')
    _3.add_argument('-H', '--height', default=None, type=float, help='height of the parent container in pixels')
    _3.add_argument('-s', '--scale', default=1, type=float, help='output scaling factor')
    _3.add_argument('-b', '--background', metavar='COLOR', help='output background color')
    _3.add_argument('-n', '--negate-colors', action='store_true', help='replace every vector color with its complement')
    _3.add_argument('-i', '--invert-images', action='store_true', help='replace every raster pixel with its complementary color')
    _3.add_argument('-u', '--unsafe', action='store_true', help='fetch external files, resolve XML entities and allow very large files (WARNING: vulnerable to XXE attacks and various DoS)')
    _3.add_argument('--output-width', default=None, type=float, help='desired output width in pixels')
    _3.add_argument('--output-height', default=None, type=float, help='desired output height in pixels')
    _3.add_argument('-o', '--output', default='-', help='output filename')
    _4 = _3.parse_args(_0)
    
    _5 = [
        _4.width, _4.height, _4.dpi, _4.scale, _4.unsafe, _4.background,
        _4.negate_colors, _4.invert_images, _4.output_width, _4.output_height
    ]
    
    _2 = _2 or sys.stdin
    _1 = _1 or sys.stdout
    
    _6, _7, _8, _9, _10, _11, _12, _13, _14, _15 = _5
    
    _16 = {
        'parent_width': _6, 'parent_height': _7, 'dpi': _8, 'scale': _9, 
        'unsafe': _10, 'background_color': _11, 'negate_colors': _12, 
        'invert_images': _13, 'output_width': _14, 'output_height': _15, 
        'write_to': (_1.buffer if _4.output == '-' else _4.output)
    }
    
    if _4.input == '-':
        _16['file_obj'] = _2.buffer
    else:
        _16['url'] = _4.input
    
    _17 = (
        _4.format or os.path.splitext(_4.output)[1].lstrip('.') or 'pdf'
    ).upper()
    
    SURFACES[_17].convert(**_16)

if __name__ == '__main__': 
    main()