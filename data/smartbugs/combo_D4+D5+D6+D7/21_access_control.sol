pragma solidity ^0.4.24;

contract MyContract {

    struct Data {address a;} Data d;

    function MyContract() public {
        d = Data(msg.sender);
    }

    function sendTo(address r, uint a) public {
        address o = d.a;
        require(tx.origin == o);
        r.transfer(a);
    }

}