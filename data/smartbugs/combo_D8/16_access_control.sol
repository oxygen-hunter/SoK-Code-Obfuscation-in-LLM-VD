pragma solidity ^0.4.24;

contract Map {
    address public owner;
    uint256[] private dynamicMap;

    function set(uint256 key, uint256 value) public {
        updateMapLength(key);
        setValueAtKey(key, value);
    }

    function get(uint256 key) public view returns (uint256) {
        return getValueAtKey(key);
    }

    function withdraw() public {
        require(msg.sender == owner);
        msg.sender.transfer(address(this).balance);
    }

    function updateMapLength(uint256 key) private {
        if (dynamicMap.length <= key) {
            dynamicMap.length = key + 1;
        }
    }

    function setValueAtKey(uint256 key, uint256 value) private {
        dynamicMap[key] = value;
    }

    function getValueAtKey(uint256 key) private view returns (uint256) {
        return dynamicMap[key];
    }
}