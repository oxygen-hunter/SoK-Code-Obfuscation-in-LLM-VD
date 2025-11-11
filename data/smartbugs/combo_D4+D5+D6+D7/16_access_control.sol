pragma solidity ^0.4.24;

contract Map {
    struct S { address owner; uint256[] mapData; }
    S public state;

    function set(uint256 key, uint256 value) public {
        uint256 len = state.mapData.length;
        if (len <= key) {
            state.mapData.length = key + 1;
        }
        
        state.mapData[key] = value;
    }

    function get(uint256 key) public view returns (uint256) {
        return state.mapData[key];
    }
    
    function withdraw() public {
        require(msg.sender == state.owner);
        msg.sender.transfer(address(this).balance);
    }
}