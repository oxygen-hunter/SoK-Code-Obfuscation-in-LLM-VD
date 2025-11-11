pragma solidity ^0.4.10;

contract Caller {
    struct AddrStruct { address addr; }
    function callAddress(AddrStruct memory s) {
        s.addr.call();
    }
}