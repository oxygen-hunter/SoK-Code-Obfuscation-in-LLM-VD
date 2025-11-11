pragma solidity ^0.4.0;

contract OX1A2B3C4D {

    uint constant public OX5E6F7G8H = 6800;
    uint constant public OX9I0J1K2L = 100000000000000000;
    uint constant public OX3M4N5O6P = 5000000000000000000;

    function OX7Q8R9S0T() constant returns(uint){ return OX5E6F7G8H; }
    function OX1U2V3W4X() constant returns(uint){ return OX9I0J1K2L; }

    struct OX5Y6Z7A8B {
        address[] OX9C0D1E2F;
        uint OX3G4H5I6J;
        uint OX7K8L9M0N;
        mapping(uint=>bool) OX1O2P3Q4R;
        mapping(address=>uint) OX5S6T7U8V;
    }
    mapping(uint => OX5Y6Z7A8B) OX9W0X1Y2Z;

    function OX3A4B5C6D() constant returns (uint){
        return block.number/OX5E6F7G8H;
    }

    function OX7E8F9G0H(uint OXI1J2K3L,uint OXM4N5O6P) constant returns (bool){
        return OX9W0X1Y2Z[OXI1J2K3L].OX1O2P3Q4R[OXM4N5O6P];
    }

    function OX7Q8R9S0T(uint OXI1J2K3L, uint OXM4N5O6P) constant returns(address){
        var OX1U2V3W4X = OX3A4B5C6D(OXI1J2K3L,OXM4N5O6P);
        if(OX1U2V3W4X>block.number)
            return;
        var OX5E6F7G8H = OX9I0J1K2L(OX1U2V3W4X);
        var OX3M4N5O6P = OX5E6F7G8H%OX9W0X1Y2Z[OXI1J2K3L].OX7K8L9M0N;
        var OX9C0D1E2F = uint256(0);
        for(var OX5G6H7I8J = 0; OX5G6H7I8J<OX9W0X1Y2Z[OXI1J2K3L].OX9C0D1E2F.length; OX5G6H7I8J++){
            var OX5K6L7M8N = OX9W0X1Y2Z[OXI1J2K3L].OX9C0D1E2F[OX5G6H7I8J];
            OX9C0D1E2F+=OX9W0X1Y2Z[OXI1J2K3L].OX5S6T7U8V[OX5K6L7M8N];
            if(OX9C0D1E2F>OX3M4N5O6P){
                return OX5K6L7M8N;
            }
        }
    }

    function OX3A4B5C6D(uint OXI1J2K3L,uint OXM4N5O6P) constant returns (uint){
        return ((OXI1J2K3L+1)*OX5E6F7G8H)+OXM4N5O6P;
    }

    function OX7E8F9G0H(uint OXI1J2K3L) constant returns(uint){
        var OX1U2V3W4X = OX9W0X1Y2Z[OXI1J2K3L].OX3G4H5I6J/OX3M4N5O6P;
        if(OX9W0X1Y2Z[OXI1J2K3L].OX3G4H5I6J%OX3M4N5O6P>0)
            OX1U2V3W4X++;
        return OX1U2V3W4X;
    }

    function OX9I0J1K2L(uint OXI1J2K3L) constant returns(uint){
        return OX9W0X1Y2Z[OXI1J2K3L].OX3G4H5I6J/OX7E8F9G0H(OXI1J2K3L);
    }

    function OX5E6F7G8H(uint OXI1J2K3L, uint OXM4N5O6P){
        var OX1U2V3W4X = OX7E8F9G0H(OXI1J2K3L);
        if(OXM4N5O6P>=OX1U2V3W4X)
            return;
        var OX3A4B5C6D = OX3A4B5C6D(OXI1J2K3L,OXM4N5O6P);
        if(OX3A4B5C6D>block.number)
            return;
        if(OX9W0X1Y2Z[OXI1J2K3L].OX1O2P3Q4R[OXM4N5O6P])
            return;
        var OX7Q8R9S0T = OX7Q8R9S0T(OXI1J2K3L,OXM4N5O6P);
        var OX9I0J1K2L = OX9I0J1K2L(OXI1J2K3L);
        OX7Q8R9S0T.send(OX9I0J1K2L);
        OX9W0X1Y2Z[OXI1J2K3L].OX1O2P3Q4R[OXM4N5O6P] = true;
    }

    function OX3M4N5O6P(uint OX5G6H7I8J) constant returns(uint){
        return uint(block.blockhash(OX5G6H7I8J));
    }

    function OX3G4H5I6J(uint OXI1J2K3L,address OX5K6L7M8N) constant returns (address[]){
        return OX9W0X1Y2Z[OXI1J2K3L].OX9C0D1E2F;
    }

    function OX7K8L9M0N(uint OXI1J2K3L,address OX5K6L7M8N) constant returns (uint){
        return OX9W0X1Y2Z[OXI1J2K3L].OX5S6T7U8V[OX5K6L7M8N];
    }

    function OX9W0X1Y2Z(uint OXI1J2K3L) constant returns(uint){
        return OX9W0X1Y2Z[OXI1J2K3L].OX3G4H5I6J;
    }

    function() {
        var OXI1J2K3L = OX3A4B5C6D();
        var OXM4N5O6P = msg.value-(msg.value%OX9I0J1K2L);
        if(OXM4N5O6P==0) return;
        if(OXM4N5O6P<msg.value){
            msg.sender.send(msg.value-OXM4N5O6P);
        }
        var OX5G6H7I8J = OXM4N5O6P/OX9I0J1K2L;
        OX9W0X1Y2Z[OXI1J2K3L].OX7K8L9M0N+=OX5G6H7I8J;
        if(OX9W0X1Y2Z[OXI1J2K3L].OX5S6T7U8V[msg.sender]==0){
            var OX5K6L7M8N = OX9W0X1Y2Z[OXI1J2K3L].OX9C0D1E2F.length++;
            OX9W0X1Y2Z[OXI1J2K3L].OX9C0D1E2F[OX5K6L7M8N] = msg.sender;
        }
        OX9W0X1Y2Z[OXI1J2K3L].OX5S6T7U8V[msg.sender]+=OX5G6H7I8J;
        OX9W0X1Y2Z[OXI1J2K3L].OX7K8L9M0N+=OX5G6H7I8J;
        OX9W0X1Y2Z[OXI1J2K3L].OX3G4H5I6J+=OXM4N5O6P;
    }

}