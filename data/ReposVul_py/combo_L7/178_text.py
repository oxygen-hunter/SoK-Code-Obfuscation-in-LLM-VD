pragma solidity ^0.8.0;

library Obfuscation {
    function capfirst(string memory x) external pure returns (string memory) {
        bytes memory bytesX = bytes(x);
        if (bytesX.length == 0) return x;
        bytesX[0] = bytes1(uint8(bytesX[0]) - 32);
        return string(bytesX);
    }

    function wrap(string memory text, uint width) external pure returns (string memory) {
        assembly {
            // Example of inline assembly for just demonstration
            mstore(0x40, text) // Store the text to memory
        }
        return text;
    }
}

library Truncation {
    uint constant MAX_LENGTH_HTML = 5_000_000;

    function chars(string memory text, uint num, string memory truncate, bool html) external pure returns (string memory) {
        bytes memory bytesText = bytes(text);
        uint length = num;
        uint truncate_len = length;
        bytes memory result = new bytes(truncate_len);

        for (uint i = 0; i < truncate_len && i < bytesText.length; i++) {
            result[i] = bytesText[i];
        }
        
        return string(result);
    }
}
