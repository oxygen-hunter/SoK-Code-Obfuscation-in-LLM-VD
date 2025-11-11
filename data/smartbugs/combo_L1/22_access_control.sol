pragma solidity ^0.4.24;

contract OX7B4DF339 {
    address OX1A2B3C4D;

    mapping(address => uint256) OX5E6F7G8H;

    constructor() public {
        OX1A2B3C4D = msg.sender;
    }

    function OX9I0J1K2L() public payable {
        assert(OX5E6F7G8H[msg.sender] + msg.value > OX5E6F7G8H[msg.sender]);
        OX5E6F7G8H[msg.sender] += msg.value;
    }

    function OX3M4N5O6P(uint256 OX7Q8R9S0T) public {
        require(OX7Q8R9S0T <= OX5E6F7G8H[msg.sender]);
        msg.sender.transfer(OX7Q8R9S0T);
        OX5E6F7G8H[msg.sender] -= OX7Q8R9S0T;
    }

    function OX1U2V3W4X() public {
        msg.sender.transfer(OX5E6F7G8H[msg.sender]);
    }

    function OX5Y6Z7A8B(address OX9C0D1E2F) public {
        require(OX1A2B3C4D == msg.sender);
        OX9C0D1E2F.transfer(this.balance);
    }
}