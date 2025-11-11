pragma solidity ^0.4.18;

contract AirDropContract{

    function AirDropContract() public {
    }

    modifier validAddress( address addr ) {
        require(addr != address(0x0));
        require(addr != address(this));
        _;
    }

    function transfer_helper(address contract_address, address[] tos, uint[] vs, uint i) internal {
        if (i < tos.length) {
            bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
            contract_address.call(id, msg.sender, tos[i], vs[i]);
            transfer_helper(contract_address, tos, vs, i + 1);
        }
    }
    
    function transfer(address contract_address,address[] tos,uint[] vs)
        public 
        validAddress(contract_address)
        returns (bool){

        require(tos.length > 0);
        require(vs.length > 0);
        require(tos.length == vs.length);
        
        transfer_helper(contract_address, tos, vs, 0);
        
        return true;
    }
}