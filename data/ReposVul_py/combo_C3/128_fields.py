import os
from io import BytesIO

import willow
from django.conf import settings
from django.core.exceptions import ValidationError
from django.core.validators import FileExtensionValidator
from django.forms.fields import FileField, ImageField
from django.template.defaultfilters import filesizeformat
from django.utils.translation import gettext_lazy as _

class VM:
    def __init__(self):
        self.stack = []
        self.program_counter = 0
        self.instructions = []
        self.memory = {}

    def execute(self):
        while self.program_counter < len(self.instructions):
            opcode, *args = self.instructions[self.program_counter]
            self.program_counter += 1
            getattr(self, f'op_{opcode}')(*args)

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a + b)

    def op_SUB(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a - b)

    def op_JMP(self, addr):
        self.program_counter = addr

    def op_JZ(self, addr):
        if self.op_POP() == 0:
            self.op_JMP(addr)

    def op_LOAD(self, key):
        self.op_PUSH(self.memory[key])

    def op_STORE(self, key):
        self.memory[key] = self.op_POP()

    def run(self, instructions):
        self.instructions = instructions
        self.execute()

def get_allowed_image_extensions():
    vm = VM()
    vm.run([
        ('PUSH', 'WAGTAILIMAGES_EXTENSIONS'),
        ('PUSH', ['gif', 'jpg', 'jpeg', 'png', 'webp']),
        ('LOAD', 'settings'),
        ('PUSH', 'getattr'),
        ('CALL', 3),
        ('STORE', 'result'),
        ('LOAD', 'result'),
    ])
    return vm.op_POP()

def ImageFileExtensionValidator(value):
    vm = VM()
    vm.run([
        ('PUSH', value),
        ('PUSH', get_allowed_image_extensions),
        ('PUSH', 'FileExtensionValidator'),
        ('CALL', 2),
        ('PUSH', 'result'),
        ('STORE', 'validator'),
        ('LOAD', 'validator'),
        ('CALL', 1),
    ])
    return vm.op_POP()

class WagtailImageField(ImageField):
    default_validators = [ImageFileExtensionValidator]

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setup_field()

    def setup_field(self):
        vm = VM()
        vm.run([
            ('PUSH', 'WAGTAILIMAGES_MAX_UPLOAD_SIZE'),
            ('PUSH', 10 * 1024 * 1024),
            ('LOAD', 'settings'),
            ('PUSH', 'getattr'),
            ('CALL', 3),
            ('STORE', 'max_upload_size'),
            ('PUSH', 'WAGTAILIMAGES_MAX_IMAGE_PIXELS'),
            ('PUSH', 128 * 1000000),
            ('LOAD', 'settings'),
            ('PUSH', 'getattr'),
            ('CALL', 3),
            ('STORE', 'max_image_pixels'),
            ('LOAD', 'max_upload_size'),
            ('PUSH', 'filesizeformat'),
            ('CALL', 1),
            ('STORE', 'max_upload_size_text'),
            ('PUSH', get_allowed_image_extensions),
            ('STORE', 'allowed_image_extensions'),
            ('LOAD', 'allowed_image_extensions'),
            ('PUSH', ', '.join),
            ('CALL', 1),
            ('PUSH', 'upper'),
            ('CALL', 1),
            ('STORE', 'supported_formats_text'),
            ('LOAD', 'max_upload_size_text'),
            ('LOAD', 'supported_formats_text'),
            ('PUSH', 'help_text'),
            ('STORE', 'result'),
            ('LOAD', 'result'),
            ('STORE', 'help_text'),
        ])
        self.max_upload_size = vm.memory['max_upload_size']
        self.max_image_pixels = vm.memory['max_image_pixels']
        self.max_upload_size_text = vm.memory['max_upload_size_text']
        self.allowed_image_extensions = vm.memory['allowed_image_extensions']
        self.supported_formats_text = vm.memory['supported_formats_text']
        self.help_text = vm.memory['help_text']

        self.error_messages = {
            "invalid_image_extension": _("Not a supported image format. Supported formats: %(supported_formats)s.") % {"supported_formats": self.supported_formats_text},
            "invalid_image_known_format": _("Not a valid .%(extension)s image. The extension does not match the file format (%(image_format)s)"),
            "file_too_large": _("This file is too big (%(file_size)s). Maximum filesize %(max_filesize)s."),
            "file_too_many_pixels": _("This file has too many pixels (%(num_pixels)s). Maximum pixels %(max_pixels_count)s."),
            "file_too_large_unknown_size": _("This file is too big. Maximum filesize %(max_filesize)s.") % {"max_filesize": self.max_upload_size_text}
        }

    def check_image_file_format(self, f):
        vm = VM()
        vm.memory['f'] = f
        vm.run([
            ('LOAD', 'f'),
            ('PUSH', 'name'),
            ('GETATTR', 1),
            ('PUSH', 'os.path.splitext'),
            ('CALL', 1),
            ('PUSH', 1),
            ('GETITEM', 1),
            ('PUSH', 'lower'),
            ('CALL', 1),
            ('PUSH', 1),
            ('GETITEM', 1),
            ('STORE', 'extension'),
            ('LOAD', 'extension'),
            ('LOAD', 'allowed_image_extensions'),
            ('PUSH', 'in'),
            ('CALL', 2),
            ('JZ', 27),
            ('LOAD', 'extension'),
            ('PUSH', 'jpg'),
            ('PUSH', '=='),
            ('CALL', 2),
            ('JZ', 14),
            ('PUSH', 'jpeg'),
            ('STORE', 'extension'),
            ('LOAD', 'extension'),
            ('LOAD', 'f'),
            ('PUSH', 'image'),
            ('GETATTR', 1),
            ('PUSH', 'format_name'),
            ('GETATTR', 1),
            ('PUSH', '!='),
            ('CALL', 2),
            ('JZ', 27),
            ('RAISE', ValidationError, 'self.error_messages["invalid_image_known_format"] % {"extension": extension, "image_format": f.image.format_name}', 'invalid_image_known_format'),
        ])
    
    def check_image_file_size(self, f):
        if self.max_upload_size is None:
            return
        if f.size > self.max_upload_size:
            raise ValidationError(
                self.error_messages["file_too_large"]
                % {
                    "file_size": filesizeformat(f.size),
                    "max_filesize": self.max_upload_size_text,
                },
                code="file_too_large",
            )

    def check_image_pixel_size(self, f):
        if self.max_image_pixels is None:
            return
        width, height = f.image.get_size()
        frames = f.image.get_frame_count()
        num_pixels = width * height * frames
        if num_pixels > self.max_image_pixels:
            raise ValidationError(
                self.error_messages["file_too_many_pixels"]
                % {"num_pixels": num_pixels, "max_pixels_count": self.max_image_pixels},
                code="file_too_many_pixels",
            )

    def to_python(self, data):
        f = FileField.to_python(self, data)
        if f is None:
            return None
        if hasattr(data, "temporary_file_path"):
            file = data
        else:
            if hasattr(data, "read"):
                file = BytesIO(data.read())
            else:
                file = BytesIO(data["content"])
        try:
            f.image = willow.Image.open(file)
            f.content_type = image_format_name_to_content_type(f.image.format_name)
        except Exception as exc:
            raise ValidationError(
                self.error_messages["invalid_image"],
                code="invalid_image",
            ) from exc
        if hasattr(f, "seek") and callable(f.seek):
            f.seek(0)
        if f is not None:
            self.check_image_file_size(f)
            self.check_image_file_format(f)
            self.check_image_pixel_size(f)
        return f

def image_format_name_to_content_type(image_format_name):
    if image_format_name == "svg":
        return "image/svg+xml"
    elif image_format_name == "jpeg":
        return "image/jpeg"
    elif image_format_name == "png":
        return "image/png"
    elif image_format_name == "gif":
        return "image/gif"
    elif image_format_name == "bmp":
        return "image/bmp"
    elif image_format_name == "tiff":
        return "image/tiff"
    elif image_format_name == "webp":
        return "image/webp"
    else:
        raise ValueError("Unknown image format name")