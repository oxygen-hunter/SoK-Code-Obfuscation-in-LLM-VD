import os
from io import BytesIO

import willow
from django.conf import settings
from django.core.exceptions import ValidationError
from django.core.validators import FileExtensionValidator
from django.forms.fields import FileField, ImageField
from django.template.defaultfilters import filesizeformat
from django.utils.translation import gettext_lazy as _


def get_allowed_image_extensions():
    return getattr(
        settings, "WAGTAILIMAGES_EXTENSIONS", ["gif", "jpg", "jpeg", "png", "webp"]
    )


def ImageFileExtensionValidator(value):
    return FileExtensionValidator(get_allowed_image_extensions())(value)


class WagtailImageField(ImageField):
    default_validators = [ImageFileExtensionValidator]

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.max_upload_size = self._get_max_upload_size()
        self.max_image_pixels = self._get_max_image_pixels()
        self.max_upload_size_text = self._get_max_upload_size_text()

        self.allowed_image_extensions = get_allowed_image_extensions()

        self.supported_formats_text = self._get_supported_formats_text()

        if self.max_upload_size is not None:
            self.help_text = self._get_help_text_with_max_size()
        else:
            self.help_text = self._get_help_text_without_max_size()

        self.error_messages["invalid_image_extension"] = self._get_error_message_invalid_image_extension()

        self.error_messages["invalid_image_known_format"] = self._get_error_message_invalid_image_known_format()

        self.error_messages["file_too_large"] = self._get_error_message_file_too_large()

        self.error_messages["file_too_many_pixels"] = self._get_error_message_file_too_many_pixels()

        self.error_messages["file_too_large_unknown_size"] = self._get_error_message_file_too_large_unknown_size()

    def _get_max_upload_size(self):
        return getattr(
            settings, "WAGTAILIMAGES_MAX_UPLOAD_SIZE", 10 * 1024 * 1024
        )

    def _get_max_image_pixels(self):
        return getattr(
            settings, "WAGTAILIMAGES_MAX_IMAGE_PIXELS", 128 * 1000000
        )

    def _get_max_upload_size_text(self):
        return filesizeformat(self.max_upload_size)

    def _get_supported_formats_text(self):
        return ", ".join(self.allowed_image_extensions).upper()

    def _get_help_text_with_max_size(self):
        return _(
            "Supported formats: %(supported_formats)s. Maximum filesize: %(max_upload_size)s."
        ) % {
            "supported_formats": self.supported_formats_text,
            "max_upload_size": self.max_upload_size_text,
        }

    def _get_help_text_without_max_size(self):
        return _("Supported formats: %(supported_formats)s.") % {
            "supported_formats": self.supported_formats_text,
        }

    def _get_error_message_invalid_image_extension(self):
        return _(
            "Not a supported image format. Supported formats: %(supported_formats)s."
        ) % {"supported_formats": self.supported_formats_text}

    def _get_error_message_invalid_image_known_format(self):
        return _(
            "Not a valid .%(extension)s image. The extension does not match the file format (%(image_format)s)"
        )

    def _get_error_message_file_too_large(self):
        return _(
            "This file is too big (%(file_size)s). Maximum filesize %(max_filesize)s."
        )

    def _get_error_message_file_too_many_pixels(self):
        return _(
            "This file has too many pixels (%(num_pixels)s). Maximum pixels %(max_pixels_count)s."
        )

    def _get_error_message_file_too_large_unknown_size(self):
        return _(
            "This file is too big. Maximum filesize %(max_filesize)s."
        ) % {"max_filesize": self.max_upload_size_text}

    def check_image_file_format(self, f):
        extension = self._get_file_extension(f)

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

    def _get_file_extension(self, f):
        return os.path.splitext(f.name)[1].lower()[1:]

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

        file = self._prepare_file_for_willow(data)

        try:
            f.image = willow.Image.open(file)
            f.content_type = self._get_content_type(f.image.format_name)

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

    def _prepare_file_for_willow(self, data):
        if hasattr(data, "temporary_file_path"):
            return data
        else:
            if hasattr(data, "read"):
                return BytesIO(data.read())
            else:
                return BytesIO(data["content"])

    def _get_content_type(self, format_name):
        return image_format_name_to_content_type(format_name)


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