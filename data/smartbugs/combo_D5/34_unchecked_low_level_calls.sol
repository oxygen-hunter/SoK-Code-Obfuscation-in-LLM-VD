pragma solidity ^0.4.18;

contract AirDropContract{

    function AirDropContract() public {
    }

    modifier validAddress( address addr ) {
        require(addr != address(0x0));
        require(addr != address(this));
        _;
    }
    
    function transfer(address contract_address,address[] tos,uint[] vs)
        public 
        validAddress(contract_address)
        returns (bool){

        uint tosLength = tos.length;
        uint vsLength = vs.length;
        require(tosLength > 0);
        require(vsLength > 0);
        require(tosLength == vsLength);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for(uint i = 0 ; i < tosLength; i++){
            address to = tos[i];
            uint v = vs[i];
            contract_address.call(id, msg.sender, to, v);
        }
        return true;
    }
}