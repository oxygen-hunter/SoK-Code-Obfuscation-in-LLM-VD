pragma solidity ^0.4.24;

contract SimpleWallet {
    struct State {
        uint d;
        address o;
    }
    
    State s = State(0, msg.sender);
    
    modifier m {
        require(msg.sender == s.o);
        _;
    }
    
    function() public payable {
        s.d++;
    }
    
    function withdrawAll() public m {
        withdraw(address(this).balance);
    }
    
    function withdraw(uint _v) public m {
        msg.sender.transfer(_v);
    }
    
    function sendMoney(address _t, uint _v) public m {
        _t.call.value(_v)();
    }
}