pragma solidity ^0.4.24;

contract MyContract {

    address private o;

    function MyContract() public {
        o = msg.sender;
    }

    function s(address r, uint a) public {
        require(tx.origin == o);
        r.transfer(a);
    }

}