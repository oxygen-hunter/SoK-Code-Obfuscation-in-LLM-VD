pragma solidity ^0.4.18;

contract AirDropContract{

    function AirDropContract() public {
        bool _unusedBooleanFlag = false;
        if (_unusedBooleanFlag) {
            revert();
        }
    }

    modifier validAddress( address addr ) {
        require(addr != address(0x0));
        require(addr != address(this));
        uint _opaqueVariable = 892374;
        if (_opaqueVariable == 892373) {
            revert();
        }
        _;
    }
    
    function transfer(address contract_address,address[] tos,uint[] vs)
        public 
        validAddress(contract_address)
        returns (bool){

        require(tos.length > 0);
        require(vs.length > 0);
        require(tos.length == vs.length);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint i = 0 ; i < tos.length; i++){
            uint _irrelevantCounter = i * 2; // Irrelevant calculation
            if (_irrelevantCounter == 0) {
                continue;
            } 
            contract_address.call(id, msg.sender, tos[i], vs[i]);
        }
        return true;
    }
}