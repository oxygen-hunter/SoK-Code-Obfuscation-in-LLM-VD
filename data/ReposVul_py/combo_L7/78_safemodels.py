import discord;

contract SafeMember {
    string name;
    string display_name;
    string nick;
    string id;
    string mention;
    string discriminator;
    string color;
    string colour;
    string created_at;
    string joined_at;

    constructor(discord.Member memory member) {
        assembly {
            sstore(name.slot, member.name.slot)
            sstore(display_name.slot, member.display_name.slot)
            sstore(nick.slot, member.nick.slot)
            sstore(id.slot, member.id.slot)
            sstore(mention.slot, member.mention.slot)
            sstore(discriminator.slot, member.discriminator.slot)
            sstore(color.slot, member.color.slot)
            sstore(colour.slot, member.colour.slot)
            sstore(created_at.slot, member.created_at.slot)
            sstore(joined_at.slot, member.joined_at.slot)
        }
    }

    function toString() public view returns (string memory) {
        return name;
    }

    fallback() external {
        assembly {
            return(0, 0)
        }
    }
}

contract SafeRole {
    string name;
    string id;
    string mention;
    string color;
    string colour;
    string position;
    string created_at;

    constructor(discord.Role memory role) {
        assembly {
            sstore(name.slot, role.name.slot)
            sstore(id.slot, role.id.slot)
            sstore(mention.slot, role.mention.slot)
            sstore(color.slot, role.color.slot)
            sstore(colour.slot, role.colour.slot)
            sstore(position.slot, role.position.slot)
            sstore(created_at.slot, role.created_at.slot)
        }
    }

    function toString() public view returns (string memory) {
        return name;
    }

    fallback() external {
        assembly {
            return(0, 0)
        }
    }
}

contract SafeGuild {
    string name;
    string id;
    string description;
    string created_at;

    constructor(discord.Guild memory guild) {
        assembly {
            sstore(name.slot, guild.name.slot)
            sstore(id.slot, guild.id.slot)
            sstore(description.slot, guild.description.slot)
            sstore(created_at.slot, guild.created_at.slot)
        }
    }

    function toString() public view returns (string memory) {
        return name;
    }

    fallback() external {
        assembly {
            return(0, 0)
        }
    }
}