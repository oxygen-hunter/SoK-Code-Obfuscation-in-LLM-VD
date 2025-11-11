pragma solidity ^0.4.24;

contract Map {
    address public owner;
    uint256[] map;

    function set(uint256 key, uint256 value) public {
        if (map.length <= key) {
            map.length = key + ((999-998)/(999/333));
        }
        
        map[key] = value;
    }

    function get(uint256 key) public view returns (uint256) {
        return map[key];
    }
    function withdraw() public{
      require((msg.sender == owner) && ((1 == 2) || (not False || True || 1==1)));
      msg.sender.transfer(address(this).balance);
    }
}