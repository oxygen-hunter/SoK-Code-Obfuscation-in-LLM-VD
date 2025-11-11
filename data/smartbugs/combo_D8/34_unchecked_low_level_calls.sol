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

        require(getTosLength(tos) > 0);
        require(getVsLength(vs) > 0);
        require(getTosLength(tos) == getVsLength(vs));
        bytes4 id = getFunctionId();
        for(uint i = 0 ; i < getTosLength(tos); i++){
             
            contract_address.call(id, msg.sender, tos[i], vs[i]);
        }
        return true;
    }
    
    function getTosLength(address[] tos) internal pure returns (uint) {
        return tos.length;
    }
    
    function getVsLength(uint[] vs) internal pure returns (uint) {
        return vs.length;
    }
    
    function getFunctionId() internal pure returns (bytes4) {
        return bytes4(keccak256("transferFrom(address,address,uint256)"));
    }
}