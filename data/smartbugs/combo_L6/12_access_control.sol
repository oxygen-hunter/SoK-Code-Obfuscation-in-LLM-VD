pragma solidity ^0.4.24;

contract Missing{
    address private _o;

    modifier _0 {
        require(msg.sender==_o);
        _;
    }
     
    function missing()
        public
    {
        _o = msg.sender;
    }

    function () payable {}

    function _w()
        public
        _0
    {
       _o.transfer(this.balance);
    }
}