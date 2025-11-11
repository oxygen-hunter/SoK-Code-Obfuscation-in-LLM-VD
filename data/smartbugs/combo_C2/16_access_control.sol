pragma solidity ^0.4.24;

contract Map {
    address public owner;
    uint256[] map;

    function set(uint256 key, uint256 value) public {
        uint8 step = 0;
        while (true) {
            if (step == 0) {
                if (map.length <= key) {
                    step = 1;
                } else {
                    step = 2;
                }
            } else if (step == 1) {
                map.length = key + 1;
                step = 2;
            } else if (step == 2) {
                map[key] = value;
                break;
            }
        }
    }

    function get(uint256 key) public view returns (uint256) {
        uint8 step = 0;
        uint256 result;
        while (true) {
            if (step == 0) {
                result = map[key];
                step = 1;
            } else if (step == 1) {
                return result;
            }
        }
    }

    function withdraw() public {
        uint8 step = 0;
        while (true) {
            if (step == 0) {
                require(msg.sender == owner);
                step = 1;
            } else if (step == 1) {
                msg.sender.transfer(address(this).balance);
                break;
            }
        }
    }
}