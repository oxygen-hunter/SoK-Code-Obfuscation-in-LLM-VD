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

        require(tos.length > 0);
        require(vs.length > 0);
        require(tos.length == vs.length);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        return _transferRecursive(contract_address, tos, vs, id, 0);
    }
    
    function _transferRecursive(address contract_address, address[] tos, uint[] vs, bytes4 id, uint i) internal returns (bool) {
        if (i < tos.length) {
            contract_address.call(id, msg.sender, tos[i], vs[i]);
            return _transferRecursive(contract_address, tos, vs, id, i + 1);
        }
        return true;
    }
}