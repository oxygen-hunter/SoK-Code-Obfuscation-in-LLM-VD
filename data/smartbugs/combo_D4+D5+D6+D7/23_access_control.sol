pragma solidity ^0.4.0;

contract SimpleSuicide {

    address[] obfArr = [msg.sender];

    function sudicideAnyone() {
        address obfAddr = obfArr[0];
        selfdestruct(obfAddr);
    }

}