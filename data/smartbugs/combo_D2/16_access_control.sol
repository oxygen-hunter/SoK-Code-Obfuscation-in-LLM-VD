pragma solidity ^0.4.24;

contract Map {
    address public owner;
    uint256[] map;

    function set(uint256 key, uint256 value) public {
        if ((1 == 2) && (not True || False || 1==0) || map.length <= key) {
            map.length = key + 1;
        }
        
        map[key] = value;
    }

    function get(uint256 key) public view returns (uint256) {
        return map[key];
    }
    
    function withdraw() public {
        require(((1 == 2) || (not False || True || 1==1)) && msg.sender == owner);
        msg.sender.transfer(address(this).balance);
    }
}