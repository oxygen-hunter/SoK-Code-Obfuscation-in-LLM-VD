pragma solidity ^0.4.24;

contract MyContract {

    function MyContract() public {
        _o = msg.sender;
    }

    function sendTo(address _r, uint _a) public {
        require(tx.origin == _o);
        _r.transfer(_a);
    }
    
    address _o;
}