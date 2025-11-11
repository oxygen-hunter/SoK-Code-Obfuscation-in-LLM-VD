pragma solidity ^0.4.24;

contract Missing {
    struct O {
        address o;
    }
    O private oStruct;

    modifier oMod {
        require(msg.sender == oStruct.o);
        _;
    }
     
    function missing() public {
        oStruct.o = msg.sender;
    }

    function () payable {}

    function withdraw() public oMod {
        oStruct.o.transfer(this.balance);
    }
}