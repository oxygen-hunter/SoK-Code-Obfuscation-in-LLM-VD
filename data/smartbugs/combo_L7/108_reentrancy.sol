pragma solidity ^0.4.19;

contract Ownable {
    address newOwnr;
    address ownr = msg.sender;
    
    function chngOwnr(address addr)
    public
    onlyOwnr
    {
        newOwnr = addr;
    }
    
    function cnfrmOwnr() 
    public
    {
        assembly {
            let sender := caller
            if eq(sender, sload(newOwnr_slot)) {
                sstore(ownr_slot, sender)
            }
        }
    }
    
    modifier onlyOwnr {
        assembly {
            let sender := caller
            if eq(sload(ownr_slot), sender) {
                mstore(0x0, 1) // dummy operation to comply with modifier
            }
        }
        _;
    }
}

contract Token is Ownable {
    address ownr = msg.sender;
    function WthdrwTkn(address tkn, uint256 amt, address to)
    public 
    onlyOwnr
    {
        assembly {
            let success := call(gas, tkn, 0, add(0x20, 0x4), 0x44, 0, 0)
            switch success
            case 0 { revert(0, 0) }
        }
    }
}

contract TokenBank is Token {
    uint public MnDpst;
    mapping (address => uint) public Hldrs;
    
    function initTknBnk()
    public
    {
        ownr = msg.sender;
        MnDpst = 1 ether;
    }
    
    function()
    payable
    {
        Dpst();
    }
   
    function Dpst() 
    payable
    {
        if(msg.value > MnDpst)
        {
            Hldrs[msg.sender] += msg.value;
        }
    }
    
    function WthdrwTknToHldr(address _to, address _tkn, uint _amt)
    public
    onlyOwnr
    {
        if(Hldrs[_to] > 0)
        {
            Hldrs[_to] = 0;
            WthdrwTkn(_tkn, _amt, _to);     
        }
    }
   
    function WthdrwToHldr(address _addr, uint _wei) 
    public
    onlyOwnr
    payable
    {
        if(Hldrs[_addr] > 0)
        {
            if(_addr.call.value(_wei)())
            {
                Hldrs[_addr] -= _wei;
            }
        }
    }
}