pragma solidity ^0.4.24;

contract Map {
    address public owner;
    uint256 mapLength;
    mapping(uint256 => uint256) mapKeyToValue;

    function set(uint256 key, uint256 value) public {
        if (mapLength <= key) {
            mapLength = key + 1;
        }
        
        mapKeyToValue[key] = value;
    }

    function get(uint256 key) public view returns (uint256) {
        return mapKeyToValue[key];
    }
    
    function withdraw() public {
        require(msg.sender == owner);
        msg.sender.transfer(address(this).balance);
    }
}