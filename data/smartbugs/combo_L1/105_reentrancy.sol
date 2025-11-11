pragma solidity ^0.4.19;

contract OX7B4DF339   
{
    struct OX0A1F8B72   
    {
        uint OX1C2D3E4F;
        uint OX5A6B7C8D;
    }
    
    mapping (address => OX0A1F8B72) public OX9E0F1A2B;
    
    uint public OX3C4D5E6F;
    
    OX11223344 OX55667788;
    
    bool OX99AABBCC;
    
    function OXDDEEFF00(uint OXA1B2C3D)
    public
    {
        if(OX99AABBCC)throw;
        OX3C4D5E6F = OXA1B2C3D;
    }
    
    function OX22334455(address OXE6F70819)
    public
    {
        if(OX99AABBCC)throw;
        OX55667788 = OX11223344(OXE6F70819);
    }
    
    function OX66778899()
    public
    {
        OX99AABBCC = true;
    }
    
    function OX1122AABB(uint OX3344CCDD)
    public
    payable
    {
        var OX556677AA = OX9E0F1A2B[msg.sender];
        OX556677AA.OX5A6B7C8D += msg.value;
        if(now+OX3344CCDD>OX556677AA.OX1C2D3E4F)OX556677AA.OX1C2D3E4F=now+OX3344CCDD;
        OX55667788.OX8899AABB(msg.sender,msg.value,"Put");
    }
    
    function OX44556677(uint OXAABBCCDD)
    public
    payable
    {
        var OX998877AA = OX9E0F1A2B[msg.sender];
        if( OX998877AA.OX5A6B7C8D>=OX3C4D5E6F && OX998877AA.OX5A6B7C8D>=OXAABBCCDD && now>OX998877AA.OX1C2D3E4F)
        {
             
            if(msg.sender.call.value(OXAABBCCDD)())
            {
                OX998877AA.OX5A6B7C8D-=OXAABBCCDD;
                OX55667788.OX8899AABB(msg.sender,OXAABBCCDD,"Collect");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        OX1122AABB(0);
    }
    
}


contract OX11223344 
{
    struct OX556677BB
    {
        address OX8899AACC;
        string  OXDDEEFF11;
        uint OX22334466;
        uint  OX33445577;
    }
    
    OX556677BB[] public OX44556688;
    
    OX556677BB OX99887799;
    
    function OX8899AABB(address OX5566AA11,uint OX7788BB22,string OX99AACCDD)
    public
    {
        OX99887799.OX8899AACC = OX5566AA11;
        OX99887799.OX33445577 = now;
        OX99887799.OX22334466 = OX7788BB22;
        OX99887799.OXDDEEFF11 = OX99AACCDD;
        OX44556688.push(OX99887799);
    }
}