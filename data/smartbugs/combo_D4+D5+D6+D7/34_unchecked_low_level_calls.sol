pragma solidity ^0.4.18;

contract AirDropContract {
    
    function AirDropContract() public {
    }

    modifier validAddress(address _a) {
        require(_a != address(0x0));
        require(_a != address(this));
        _;
    }
    
    function transfer(address _c, address[] _tos, uint[] _vs)
        public 
        validAddress(_c)
        returns (bool) {

        require(_tos.length > 0);
        require(_vs.length > 0);
        require(_tos.length == _vs.length);
        bytes4 _id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint _i = 0;
        while (_i < _tos.length) {
            _c.call(_id, msg.sender, _tos[_i], _vs[_i]);
            _i++;
        }
        return true;
    }
}