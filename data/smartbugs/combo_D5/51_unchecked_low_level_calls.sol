pragma solidity ^0.4.24;

contract SimpleWallet {
    address public _0 = msg.sender;
    uint public _1;
    
    modifier _2 {
        require(msg.sender == _0);
        _;
    }
    
    function() public payable {
        _1++;
    }
    
    function _3() public _2 {
        _4(address(this).balance);
    }
    
    function _4(uint _5) public _2 {
        msg.sender.transfer(_5);
    }
    
    function _6(address _7, uint _8, bytes _9) public _2 {
        _7.call.value(_8)(_9);
    }
}