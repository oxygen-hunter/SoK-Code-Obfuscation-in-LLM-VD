import pytest
from PIL import Image
from .helper import on_ci

@pytest.mark.parametrize(
    "test_file",
    [
        "Tests/images/crash_1.tif",
        "Tests/images/crash_2.tif",
        "Tests/images/crash-2020-10-test.tif",
        "Tests/images/crash-1152ec2d1a1a71395b6f2ce6721c38924d025bf3.tif",
        "Tests/images/crash-0e16d3bfb83be87356d026d66919deaefca44dac.tif",
    ],
)
@pytest.mark.filterwarnings("ignore:Possibly corrupt EXIF data")
@pytest.mark.filterwarnings("ignore:Metadata warning")
def test_tiff_crashes(test_file):
    try:
        with Image.open(test_file) as im:
            im.load()
    except FileNotFoundError:
        if not on_ci():
            pytest.skip("test image not found")
            return
        raise
    except OSError:
        pass

```

```c
#include <stdio.h>
#include <stdlib.h>

void open_image(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("File not found: %s\n", filename);
        exit(EXIT_FAILURE);
    }
    // Simulate image loading
    fclose(file);
}

int main() {
    const char* test_files[] = {
        "Tests/images/crash_1.tif",
        "Tests/images/crash_2.tif",
        "Tests/images/crash-2020-10-test.tif",
        "Tests/images/crash-1152ec2d1a1a71395b6f2ce6721c38924d025bf3.tif",
        "Tests/images/crash-0e16d3bfb83be87356d026d66919deaefca44dac.tif",
    };

    for (int i = 0; i < sizeof(test_files)/sizeof(test_files[0]); i++) {
        open_image(test_files[i]);
    }

    return 0;
}