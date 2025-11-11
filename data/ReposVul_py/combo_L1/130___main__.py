import argparse as OX9D8F1B9F
import os as OX3E5D3E9F
import sys as OX6F2B4B1A

from . import SURFACES as OX7A3D1B8E, VERSION as OX1B9A7F8D

def OX7B4DF339(OX4A2E3D9C=None, OX3D8C6B7E=None, OX2B7C1D9A=None):
    OX1E8F3B7A = OX9D8F1B9F.ArgumentParser(
        description='Convert SVG files to other formats')
    OX1E8F3B7A.add_argument('input', default='-', help='input filename or URL')
    OX1E8F3B7A.add_argument(
        '-v', '--version', action='version', version=OX1B9A7F8D)
    OX1E8F3B7A.add_argument(
        '-f', '--format', help='output format',
        choices=sorted([OX8F2A4D3C.lower() for OX8F2A4D3C in OX7A3D1B8E]))
    OX1E8F3B7A.add_argument(
        '-d', '--dpi', default=96, type=float,
        help='ratio between 1 inch and 1 pixel')
    OX1E8F3B7A.add_argument(
        '-W', '--width', default=None, type=float,
        help='width of the parent container in pixels')
    OX1E8F3B7A.add_argument(
        '-H', '--height', default=None, type=float,
        help='height of the parent container in pixels')
    OX1E8F3B7A.add_argument(
        '-s', '--scale', default=1, type=float, help='output scaling factor')
    OX1E8F3B7A.add_argument(
        '-b', '--background', metavar='COLOR', help='output background color')
    OX1E8F3B7A.add_argument(
        '-n', '--negate-colors', action='store_true',
        help='replace every vector color with its complement')
    OX1E8F3B7A.add_argument(
        '-i', '--invert-images', action='store_true',
        help='replace every raster pixel with its complementary color')
    OX1E8F3B7A.add_argument(
        '-u', '--unsafe', action='store_true',
        help='fetch external files, resolve XML entities and allow very large '
             'files (WARNING: vulnerable to XXE attacks and various DoS)')
    OX1E8F3B7A.add_argument(
        '--output-width', default=None, type=float,
        help='desired output width in pixels')
    OX1E8F3B7A.add_argument(
        '--output-height', default=None, type=float,
        help='desired output height in pixels')

    OX1E8F3B7A.add_argument('-o', '--output', default='-', help='output filename')

    OX6C5E8F3D = OX1E8F3B7A.parse_args(OX4A2E3D9C)
    OX9F6A4B2C = {
        'parent_width': OX6C5E8F3D.width, 'parent_height': OX6C5E8F3D.height,
        'dpi': OX6C5E8F3D.dpi, 'scale': OX6C5E8F3D.scale, 'unsafe': OX6C5E8F3D.unsafe,
        'background_color': OX6C5E8F3D.background,
        'negate_colors': OX6C5E8F3D.negate_colors,
        'invert_images': OX6C5E8F3D.invert_images,
        'output_width': OX6C5E8F3D.output_width,
        'output_height': OX6C5E8F3D.output_height}
    OX2B7C1D9A = OX2B7C1D9A or OX6F2B4B1A.stdin
    OX3D8C6B7E = OX3D8C6B7E or OX6F2B4B1A.stdout
    OX9F6A4B2C['write_to'] = (
        OX3D8C6B7E.buffer if OX6C5E8F3D.output == '-' else OX6C5E8F3D.output)
    if OX6C5E8F3D.input == '-':
        OX9F6A4B2C['file_obj'] = OX2B7C1D9A.buffer
    else:
        OX9F6A4B2C['url'] = OX6C5E8F3D.input
    OX4F9D3E2B = (
        OX6C5E8F3D.format or
        OX3E5D3E9F.path.splitext(OX6C5E8F3D.output)[1].lstrip('.') or
        'pdf').upper()

    OX7A3D1B8E[OX4F9D3E2B.upper()].convert(**OX9F6A4B2C)

if __name__ == '__main__':  # pragma: no cover
    OX7B4DF339()