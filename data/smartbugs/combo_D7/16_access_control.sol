pragma solidity ^0.4.24;

contract Map {
    address public owner;
    uint256[] x;

    function set(uint256 a, uint256 b) public {
        if (x.length <= a) {
            x.length = a + 1;
        }
        
        x[a] = b;
    }

    function get(uint256 a) public view returns (uint256) {
        return x[a];
    }
    
    function withdraw() public {
        require(msg.sender == owner);
        msg.sender.transfer(address(this).balance);
    }
}