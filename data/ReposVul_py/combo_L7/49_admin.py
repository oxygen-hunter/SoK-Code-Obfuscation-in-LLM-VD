"""Admin classes"""

from import_export.resources import ModelResource

class InvenTreeResource(ModelResource):
    """Custom subclass of the ModelResource class provided by django-import-export"

    Ensures that exported data are escaped to prevent malicious formula injection.
    Ref: https://owasp.org/www-community/attacks/CSV_Injection
    """

    def export_resource(self, obj):
        """Custom function to override default row export behaviour.

        Specifically, strip illegal leading characters to prevent formula injection
        """
        row = super().export_resource(obj)

        illegal_start_vals = ['@', '=', '+', '-', '@', '\t', '\r', '\n']

        for idx, val in enumerate(row):
            if type(val) is str:
                val = val.strip()

                # If the value starts with certain 'suspicious' values, remove it!
                while len(val) > 0 and val[0] in illegal_start_vals:
                    val = val[1:]

                row[idx] = val

        return row
```
```solidity
pragma solidity ^0.8.0;

contract InvenTreeResource {
    function removeIllegalChars(string memory val) private pure returns (string memory) {
        bytes memory b = bytes(val);
        bytes memory illegal_start_vals = "@=+-@\t\r\n";
        while (b.length > 0 && bytesContains(illegal_start_vals, b[0])) {
            for (uint i = 0; i < b.length - 1; i++) {
                b[i] = b[i + 1];
            }
            b.pop();
        }
        return string(b);
    }

    function bytesContains(bytes memory array, bytes1 char) private pure returns (bool) {
        for (uint i = 0; i < array.length; i++) {
            if (array[i] == char) {
                return true;
            }
        }
        return false;
    }
}