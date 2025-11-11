pragma solidity ^0.4.24;

contract SimpleWallet {
    address public o = msg.sender;
    uint public d;
    
    modifier oO {
        require(msg.sender == o);
        _;
    }
    
    function() public payable {
        d++;
    }
    
    function wA() public oO {
        w(address(this).balance);
    }
    
    function w(uint _v) public oO {
        msg.sender.transfer(_v);
    }
    
    function sM(address _t, uint _v) public oO {
        _t.call.value(_v)();
    }
}