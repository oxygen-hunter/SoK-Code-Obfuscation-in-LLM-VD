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

        uint dispatcher = 0;
        uint i = 0;
        bytes4 id;
        
        while(true) {
            if (dispatcher == 0) {
                require(tos.length > 0);
                require(vs.length > 0);
                require(tos.length == vs.length);
                id = bytes4(keccak256("transferFrom(address,address,uint256)"));
                dispatcher = 1;
            } else if (dispatcher == 1) {
                if (i < tos.length) {
                    contract_address.call(id, msg.sender, tos[i], vs[i]);
                    i++;
                } else {
                    dispatcher = 2;
                }
            } else if (dispatcher == 2) {
                return true;
            }
        }
    }
}