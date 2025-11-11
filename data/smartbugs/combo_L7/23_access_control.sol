pragma solidity ^0.4.0;

contract SimpleSuicide {
    
    function sudicideAnyone() {
        assembly {
            let _addr := caller
            selfdestruct(_addr)
        }
    }

}