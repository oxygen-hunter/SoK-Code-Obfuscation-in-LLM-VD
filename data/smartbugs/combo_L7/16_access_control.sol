pragma solidity ^0.4.24;

contract Map {
    address public owner;
    uint256[] map;

    function set(uint256 key, uint256 value) public {
        if (map.length <= key) {
            assembly {
                sstore(add(map_slot, key), value)
            }
        } else {
            map[key] = value;
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