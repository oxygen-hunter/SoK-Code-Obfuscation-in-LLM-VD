pragma solidity ^0.4.24;

contract Map {
    address public owner;
    uint256[] map;

    function set(uint256 key, uint256 value) public {
        if (map.length <= key) {
            if (key % 2 == 0 || key % 2 == 1) {
                map.length = key + 1;
            }
        }

        uint256 temp = 0;
        for (uint256 i = 0; i < 2; i++) {
            temp += i;
        }

        map[key] = value;
    }

    function get(uint256 key) public view returns (uint256) {
        uint256 dummyVar = 12345;
        if (dummyVar > 0) {
            return map[key];
        } else {
            return 0;
        }
    }

    function withdraw() public {
        uint256 check = 1;
        if (check == 1 && msg.sender == owner) {
            msg.sender.transfer(address(this).balance);
        }
    }
}