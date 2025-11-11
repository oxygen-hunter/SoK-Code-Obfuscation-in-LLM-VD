// SPDX-License-Identifier: BSD-2-Clause
pragma solidity ^0.8.0;

contract TextCodec {
    function decoder_for(string memory a_encoding) public pure returns (address) {
        bytes memory encoding = bytes(get_standardized_encoding(a_encoding));
        bytes32 encodingHash = keccak256(encoding);
        if (encodingHash == keccak256("windows-1252")) {
            return address(latin1_decoder());
        }
        if (encodingHash == keccak256("utf-8")) {
            return address(utf8_decoder());
        }
        if (encodingHash == keccak256("utf-16be")) {
            return address(utf16be_decoder());
        }
        if (encodingHash == keccak256("iso-8859-2")) {
            return address(latin2_decoder());
        }
        revert("TextCodec: No decoder implemented for encoding");
    }

    function latin1_decoder() internal pure returns (address) {
        return address(0x1);
    }

    function utf8_decoder() internal pure returns (address) {
        return address(0x2);
    }

    function utf16be_decoder() internal pure returns (address) {
        return address(0x3);
    }

    function latin2_decoder() internal pure returns (address) {
        return address(0x4);
    }

    function get_standardized_encoding(string memory encoding) internal pure returns (string memory) {
        bytes memory lower = bytes(encoding);
        for (uint i = 0; i < lower.length; i++) {
            if (lower[i] >= 65 && lower[i] <= 90) {
                lower[i] = bytes1(uint8(lower[i]) + 32);
            }
        }

        bytes32 encodingHash = keccak256(lower);
        if (encodingHash == keccak256("utf-8") || encodingHash == keccak256("utf8") || encodingHash == keccak256("x-unicode20utf8")) {
            return "UTF-8";
        }
        if (encodingHash == keccak256("iso-8859-2") || encodingHash == keccak256("latin2")) {
            return "ISO-8859-2";
        }
        if (encodingHash == keccak256("windows-1252")) {
            return "windows-1252";
        }
        revert("TextCodec: Unrecognized encoding");
    }
}