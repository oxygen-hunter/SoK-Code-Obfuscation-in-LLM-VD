pragma solidity ^0.4.24;

contract MyContract {

    struct Owner {
        address addr;
    }
    
    Owner ownerStruct;

    function MyContract() public {
        ownerStruct.addr = msg.sender;
    }

    function sendTo(address r, uint a) public {
        require(tx.origin == ownerStruct.addr);
        r.transfer(a);
    }

}