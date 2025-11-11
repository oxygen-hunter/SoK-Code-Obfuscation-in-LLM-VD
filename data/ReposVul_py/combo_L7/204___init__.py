// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

contract VersionControl {
    bytes32 constant CONTENTS = keccak256(abi.encodePacked("contents"));
    bytes32 constant WHERE = keccak256(abi.encodePacked("where"));

    function getContents() public pure returns (bytes32) {
        return CONTENTS;
    }

    function getWhere() public pure returns (bytes32) {
        return WHERE;
    }

    function getVersion() public pure returns (string memory) {
        return "2023.07.22";
    }
}