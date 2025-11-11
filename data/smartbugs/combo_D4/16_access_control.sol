pragma solidity ^0.4.24;

contract Map {
    struct S { address a; uint256[] b; }
    S s;

    function set(uint256 k, uint256 v) public {
        if (s.b.length <= k) {
            s.b.length = k + 1;
        }
        
        s.b[k] = v;
    }

    function get(uint256 k) public view returns (uint256) {
        return s.b[k];
    }
    
    function withdraw() public {
        require(msg.sender == s.a);
        msg.sender.transfer(address(this).balance);
    }
}