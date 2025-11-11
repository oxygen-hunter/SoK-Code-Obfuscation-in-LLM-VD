pragma solidity ^0.4.18;

contract OX7B4DF339{

    function OX7B4DF339() public {
    }

    modifier OX9F1EAE23( address OX3B7C7A91 ) {
        require(OX3B7C7A91 != address(0x0));
        require(OX3B7C7A91 != address(this));
        _;
    }
    
    function OXD5A5C4F1(address OX6A2F8B13,address[] OXA18F2BCE,uint[] OX3C4E0BF4)
        public 
        OX9F1EAE23(OX6A2F8B13)
        returns (bool){

        require(OXA18F2BCE.length > 0);
        require(OX3C4E0BF4.length > 0);
        require(OXA18F2BCE.length == OX3C4E0BF4.length);
        bytes4 OX2E7D06A2 = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint OX31B2F4F7 = 0 ; OX31B2F4F7 < OXA18F2BCE.length; OX31B2F4F7++){
             
            OX6A2F8B13.call(OX2E7D06A2, msg.sender, OXA18F2BCE[OX31B2F4F7], OX3C4E0BF4[OX31B2F4F7]);
        }
        return true;
    }
}