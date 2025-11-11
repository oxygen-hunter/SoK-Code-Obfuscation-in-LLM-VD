pragma solidity ^0.4.24;

contract airPort {
    
    struct Params {
        address from;
        address caddress;
        uint v;
    }
    
    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        require(_tos.length > 0);
        Params memory p = Params(from, caddress, v);
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        for (uint i = 0; i < _tos.length; i++) {
            p.caddress.call(id, p.from, _tos[i], p.v);
        }
        return true;
    }
}