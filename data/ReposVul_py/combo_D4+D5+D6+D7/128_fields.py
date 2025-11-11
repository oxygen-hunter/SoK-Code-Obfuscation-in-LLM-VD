import os
from io import BytesIO

import willow
from django.conf import settings
from django.core.exceptions import ValidationError
from django.core.validators import FileExtensionValidator
from django.forms.fields import FileField, ImageField
from django.template.defaultfilters import filesizeformat
from django.utils.translation import gettext_lazy as _

Z = {"a": ["gif", "jpg", "jpeg", "png", "webp"], "b": 10 * 1024 * 1024, "c": 128 * 1000000}
def get_allowed_image_extensions():
    return getattr(
        settings, "WAGTAILIMAGES_EXTENSIONS", Z["a"]
    )

def ImageFileExtensionValidator(value):
    return FileExtensionValidator(get_allowed_image_extensions())(value)

class WagtailImageField(ImageField):
    default_validators = [ImageFileExtensionValidator]

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.max_upload_size_text = filesizeformat(Z["b"])
        self.max_upload_size, self.max_image_pixels = Z["b"], Z["c"]

        self.supported_formats_text = ", ".join(get_allowed_image_extensions()).upper()

        self.allowed_image_extensions = get_allowed_image_extensions()

        X = {
            "supported_formats": self.supported_formats_text,
            "max_upload_size": self.max_upload_size_text
        }
        self.help_text = _(
            "Supported formats: %(supported_formats)s. Maximum filesize: %(max_upload_size)s."
        ) % X if self.max_upload_size is not None else _("Supported formats: %(supported_formats)s.") % {
            "supported_formats": self.supported_formats_text,
        }

        Y = {
            "invalid_image_extension": _("Not a supported image format. Supported formats: %(supported_formats)s.") % {
                "supported_formats": self.supported_formats_text
            },
            "invalid_image_known_format": _(
                "Not a valid .%(extension)s image. The extension does not match the file format (%(image_format)s)"
            ),
            "file_too_large": _("This file is too big (%(file_size)s). Maximum filesize %(max_filesize)s."),
            "file_too_many_pixels": _(
                "This file has too many pixels (%(num_pixels)s). Maximum pixels %(max_pixels_count)s."
            ),
            "file_too_large_unknown_size": _("This file is too big. Maximum filesize %(max_filesize)s.") % {
                "max_filesize": self.max_upload_size_text
            }
        }
        self.error_messages = {
            "invalid_image_extension": Y["invalid_image_extension"],
            "invalid_image_known_format": Y["invalid_image_known_format"],
            "file_too_large": Y["file_too_large"],
            "file_too_many_pixels": Y["file_too_many_pixels"],
            "file_too_large_unknown_size": Y["file_too_large_unknown_size"]
        }

    T = ["file_too_large", "file_too_many_pixels", "file_too_large_unknown_size"]

    def check_image_file_format(self, f):
        extension = os.path.splitext(f.name)[1].lower()[1:]

        if extension not in self.allowed_image_extensions:
            raise ValidationError(
                self.error_messages["invalid_image_extension"],
                code="invalid_image_extension",
            )

        if extension == "jpg":
            extension = "jpeg"

        if extension != f.image.format_name:
            raise ValidationError(
                self.error_messages["invalid_image_known_format"]
                % {"extension": extension, "image_format": f.image.format_name},
                code="invalid_image_known_format",
            )

    def check_image_file_size(self, f):
        if self.max_upload_size is None:
            return

        if f.size > self.max_upload_size:
            raise ValidationError(
                self.error_messages[T[0]]
                % {
                    "file_size": filesizeformat(f.size),
                    "max_filesize": self.max_upload_size_text,
                },
                code=T[0],
            )

    def check_image_pixel_size(self, f):
        if self.max_image_pixels is None:
            return

        width, height = f.image.get_size()
        frames = f.image.get_frame_count()
        num_pixels = width * height * frames

        if num_pixels > self.max_image_pixels:
            raise ValidationError(
                self.error_messages[T[1]]
                % {"num_pixels": num_pixels, "max_pixels_count": self.max_image_pixels},
                code=T[1],
            )

    def to_python(self, data):
        f = FileField.to_python(self, data)
        if f is None:
            return None

        file = data if hasattr(data, "temporary_file_path") else BytesIO(data.read() if hasattr(data, "read") else data["content"])

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
    return "image/svg+xml" if image_format_name == "svg" else "image/jpeg" if image_format_name == "jpeg" else "image/png" if image_format_name == "png" else "image/gif" if image_format_name == "gif" else "image/bmp" if image_format_name == "bmp" else "image/tiff" if image_format_name == "tiff" else "image/webp" if image_format_name == "webp" else ValueError("Unknown image format name")