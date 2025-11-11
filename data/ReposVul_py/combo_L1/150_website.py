from datetime import datetime as OX1A
from math import prod as OX1B
import nextcord as OX1C
from nextcord.ext.commands import Cog as OX1D, command as OX1E
from nextcord.ext.commands.core import Command as OX1F
from nextcord import Embed as OX1G, Colour as OX1H, colour as OX1I
from quart import Quart as OX1J, request as OX1K
from ..utils.database import db as OX1L
from ..utils.api import * 
from ..utils.util import require_apikey as OX1M
from bson.json_util import ObjectId as OX1N, dumps as OX1O
from ro_py import Client as OX1P
from bs4 import BeautifulSoup as OX1Q
import json as OX1R
import string as OX1S
import random as OX1T
import requests as OX1U
import re as OX1V

OX1W = OX1J(__name__)

with open("./BOT/lib/bot/config.json") as OX1X:
    OX1Y = OX1R.load(OX1X)
OX1Z = OX1P()
OX2A = {}

class OX2B(OX1R.JSONEncoder):
    def default(self, OX2C):
        if isinstance(OX2C, OX1N):
            return str(OX2C)
        return super(OX2B, self).default(OX2C)

OX1W.json_encoder = OX2B

@OX1W.route("/", methods=["GET"])
async def OX2D():
    return {"message": "Ok"}

@OX1W.route("/v1/status", methods=["GET"])
async def OX2E():
    OX2F = OX1L.command("serverStatus")
    if OX2F:
        return {"message": "Ok", "info": {"api": "Ok", "database": "Ok"}}
    else:
        return {"message": "Ok", "info": {"api": "Ok", "database": "Error"}}

@OX1W.route("/v1/products", methods=["GET"])
@OX1M
async def OX2G():
    OX2H = getproducts()
    OX2I = {}
    for OX2J in OX2H:
        OX2I[OX2J["name"]] = OX2J
    return OX2I

@OX1W.route("/v1/create_product", methods=["POST"])
@OX1M
async def OX2K():
    OX2L = await OX1K.get_json()
    try:
        createproduct(OX2L["name"], OX2L["description"], OX2L["price"])
        return {
            "info": {
                "name": OX2L["name"],
                "description": OX2L["description"],
                "price": OX2L["price"],
            }
        }
    except:
        return {"errors": [{"message": "Unable to delete product"}]}

@OX1W.route("/v1/update_product", methods=["POST"])
@OX1M
async def OX2M():
    OX2N = await OX1K.get_json()
    try:
        updateproduct(
            OX2N["oldname"], OX2N["newname"], OX2N["description"], OX2N["price"]
        )
        return {
            "info": {
                "name": OX2N["newname"],
                "description": OX2N["description"],
                "price": OX2N["price"],
            }
        }
    except:
        return {"errors": [{"message": "Unable to update product"}]}

@OX1W.route("/v1/delete_product", methods=["DELETE"])
@OX1M
async def OX2O():
    OX2P = await OX1K.get_json()
    try:
        deleteproduct(OX2P["name"])
        return {"message": "Deleted"}
    except:
        return {"errors": [{"message": "Unable to create product"}]}

@OX1W.route(
    "/v1/user", methods=["GET", "POST"]
)
async def OX2Q():
    try:
        OX2R = await OX1K.get_json()
        OX2S = getuser(OX2R["userid"])
        if OX2S == None:
            return {"errors": [{"message": "Unable to get user"}]}
        return OX1O(OX2S)
    except:
        return {"errors": [{"message": "Something went wrong when getting user"}]}

@OX1W.route("/v1/verify_user", methods=["POST"])
@OX1M
async def OX2T():
    OX2U = await OX1K.get_json()
    OX2V = getuser(OX2U["userid"])
    if not OX2V:
        OX2W = "".join(OX1T.choices(OX1S.ascii_uppercase + OX1S.digits, k=5))
        OX2A[OX2W] = OX2U["userid"]
        return {"key": OX2W}
    else:
        return {"errors": [{"message": "User is already verified"}]}

@OX1W.route("/v1/give_product", methods=["POST"])
@OX1M
async def OX2X():
    OX2Y = await OX1K.get_json()
    try:
        giveproduct(OX2Y["userid"], OX2Y["productname"])
        OX2Z = getuser(OX2Y["userid"])
        return OX1O(OX2Z)
    except:
        return {"errors": [{"message": "Unable to give product"}]}

@OX1W.route("/v1/revoke_product", methods=["DELETE"])
@OX1M
async def OX3A():
    OX3B = await OX1K.get_json()
    try:
        revokeproduct(OX3B["userid"], OX3B["productname"])
        OX3C = getuser(OX3B["userid"])
        return OX1O(OX3C)
    except:
        return {"errors": [{"message": "Unable to revoke product"}]}

@OX1W.route("/v1/create_purchase", methods=["POST"])
@OX1M
async def OX3D():
    OX3E = await OX1K.get_json()
    if OX3E["gameid"] and OX3E["name"] and OX3E["price"]:
        OX3F = {
            "universeId": OX3E["gameid"],
            "name": OX3E["name"],
            "priceInRobux": OX3E["price"],
            "description": OX3E["name"] + " " + str(OX3E["price"]),
        }
        OX3G = {".ROBLOSECURITY": OX1Y["roblox"]["cookie"]}
        OX3H = OX1U.post(
            "https://auth.roblox.com/v2/logout",
            data=None,
            cookies=OX3G,
        )
        if OX3H.headers["x-csrf-token"]:
            OX3I = {"x-csrf-token": OX3H.headers["x-csrf-token"]}
            OX3J = OX1U.post(
                "https://www.roblox.com/places/developerproducts/add",
                data=OX3F,
                cookies=OX3G,
                headers=OX3I,
            )

            if OX3J.status_code == 200:
                return {
                    "ProductId": "".join(
                        OX1V.findall(
                            r"\d",
                            str(
                                OX1Q(OX3J.text, "html.parser").find(
                                    id="DeveloperProductStatus"
                                )
                            ),
                        )
                    )
                }

    return {"errors": [{"message": "Unable to create developer product"}]}


class OX3K(OX1D):
    def __init__(self, OX3L):
        self.OX3M = OX3L

    @OX1E(
        name="website",
        aliases=["web", "ws", "websitestatus"],
        brief="Displays if the website is online.",
        catagory="misc",
    )
    async def OX3N(self, OX3O):
        if OX3O.message.author.id in self.OX3M.owner_ids:
            await OX3O.send("🟢 Website Online")

    @OX1E(name="verify", brief="Verify's you as a user.", catagory="user")
    async def OX3P(self, OX3Q, OX3R):
        if OX3R in OX2A:
            OX3S = OX2A[OX3R]
            try:
                OX3T = await OX1Z.get_user(OX3S)
                OX3U = OX3T.name
                verifyuser(OX3S, OX3Q.author.id, OX3U)
                OX2A.pop(OX3R)
                await OX3Q.send("Verified", delete_after=5.0, reference=OX3Q.message)
            except:
                await OX3Q.send(
                    "I was unable to verify you",
                    delete_after=5.0,
                    reference=OX3Q.message,
                )
        else:
            await OX3Q.send(
                "The provided key was incorrect please check the key and try again.",
                delete_after=5.0,
                reference=OX3Q.message,
            )

    @OX1D.listener()
    async def OX3V(self):
        if not self.OX3M.ready:
            self.OX3M.cogs_ready.ready_up("website")
            await self.OX3M.stdout.send("`/lib/cogs/website.py` ready")
            print(" /lib/cogs/website.py ready")

def setup(OX3W):
    OX3W.loop.create_task(
        OX1W.run_task("0.0.0.0")
    )  
    OX3W.add_cog(OX3K(OX3W))