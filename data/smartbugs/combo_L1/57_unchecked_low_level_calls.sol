pragma solidity ^0.4.23;

contract OX7B4DF339  {
    modifier OX1A2B3C4D { if (msg.sender == OX5D6E7F8G) _; } address OX5D6E7F8G = msg.sender;
    function OX9H0J1K2L(address OX3M4N5P6Q) public OX1A2B3C4D { OX5D6E7F8G = OX3M4N5P6Q; } 
    function OX7R8S9T0U(address OX1V2W3X4Y, bytes OX5Z6A7B8C) public payable {
        OX1V2W3X4Y.call.value(msg.value)(OX5Z6A7B8C);
    }
}

contract OX2D3E4F5G is OX7B4DF339 {
    address public OX6H7I8J9K;
    mapping (address => uint256) public OX0L1M2N3O;

    function () public payable { }
    
    function OX4P5Q6R7S() public payable {
        if (msg.sender == tx.origin) {
            OX6H7I8J9K = msg.sender;
            OX8T9U0V1W();
        }
    }
    
    function OX8T9U0V1W() public payable {
        if (msg.value > 0.25 ether) {
            OX0L1M2N3O[msg.sender] += msg.value;
        }
    }
    
    function OX2X3Y4Z5A(uint256 OX6B7C8D9E) public OX1A2B3C4D {
        if (OX6B7C8D9E > 0 && OX0L1M2N3O[msg.sender] >= OX6B7C8D9E) {
            msg.sender.transfer(OX6B7C8D9E);
        }
    }
}