pragma solidity ^0.4.24;

contract SimpleWallet {
    address public o = msg.sender;
    uint public d;
    
    modifier x {
        require(msg.sender == o);
        _;
    }
    
    function() public payable {
        d++;
    }
    
    function w() public x {
        z(address(this).balance);
    }
    
    function z(uint a) public x {
        msg.sender.transfer(a);
    }
    
    function s(address b, uint c, bytes e) public x {
         
        b.call.value(c)(e);
    }
}