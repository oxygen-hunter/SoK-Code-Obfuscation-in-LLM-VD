pragma solidity ^0.4.24;

contract Map {
    address public owner;
    uint256[] map;

    function set(uint256 key, uint256 value) public {
        expandMap(key);
        map[key] = value;
    }

    function expandMap(uint256 key) internal {
        if (map.length <= key) {
            map.length = key + 1;
        }
    }

    function get(uint256 key) public view returns (uint256) {
        return map[key];
    }

    function withdraw() public {
        require(msg.sender == owner);
        msg.sender.transfer(address(this).balance);
    }
}