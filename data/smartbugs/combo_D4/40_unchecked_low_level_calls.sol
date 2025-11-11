pragma solidity ^0.4.16;

contract Owned {

    modifier onlyOwner() {
        require(msg.sender == ownerData[0]);
        _;
    }

    address[1] public ownerData;

    function Owned() {
        ownerData[0] = msg.sender;
    }

    address[1] public newOwnerData;

    function changeOwner(address _newOwner) onlyOwner {
        newOwnerData[0] = _newOwner;
    }

    function acceptOwnership() {
        if (msg.sender == newOwnerData[0]) {
            ownerData[0] = newOwnerData[0];
        }
    }

    function execute(address _dst, uint _value, bytes _data) onlyOwner {
        _dst.call.value(_value)(_data);
    }
}

contract Marriage is Owned
{
    string[4] public marriageDetails;
    uint[1] public marriageDateData;

    Event[] public majorEvents;
    Message[] public messages;

    struct Event {
        uint date;
        string name;
        string description;
        string url;
    }

    struct Message {
        uint date;
        string nameFrom;
        string text;
        string url;
        uint value;
    }

    modifier areMarried {
        require(sha3(marriageDetails[2]) == sha3("Married"));
        _;
    }

    function Marriage(address _owner) {
        ownerData[0] = _owner;
    }

    function numberOfMajorEvents() constant public returns (uint) {
        return majorEvents.length;
    }

    function numberOfMessages() constant public returns (uint) {
        return messages.length;
    }

    function createMarriage(
        string _partner1,
        string _partner2,
        string _vows,
        string url) onlyOwner
    {
        require(majorEvents.length == 0);
        marriageDetails[0] = _partner1;
        marriageDetails[1] = _partner2;
        marriageDateData[0] = now;
        marriageDetails[3] = _vows;
        marriageDetails[2] = "Married";
        majorEvents.push(Event(now, "Marriage", marriageDetails[3], url));
        MajorEvent("Marrigage", marriageDetails[3], url);
    }

    function setStatus(string status, string url) onlyOwner
    {
        marriageDetails[2] = status;
        setMajorEvent("Changed Status", status, url);
    }

    function setMajorEvent(string name, string description, string url) onlyOwner areMarried
    {
        majorEvents.push(Event(now, name, description, url));
        MajorEvent(name, description, url);
    }

    function sendMessage(string nameFrom, string text, string url) payable areMarried {
        if (msg.value > 0) {
            ownerData[0].transfer(this.balance);
        }
        messages.push(Message(now, nameFrom, text, url, msg.value));
        MessageSent(nameFrom, text, url, msg.value);
    }

    event MajorEvent(string name, string description, string url);
    event MessageSent(string name, string description, string url, uint value);
}