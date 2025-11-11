from datetime import datetime
from math import prod
import nextcord
from nextcord.ext.commands import Cog, command
from nextcord.ext.commands.core import Command
from nextcord import Embed, Colour, colour
from quart import Quart, request
from ..utils.database import db
from ..utils.api import *
from ..utils.util import require_apikey
from bson.json_util import ObjectId, dumps
from ro_py import Client
from bs4 import BeautifulSoup
import json
import string
import random
import requests
import re

app = Quart(__name__)

with open("./BOT/lib/bot/config.json") as config_file:
    config = json.load(config_file)
roblox = Client()
verificationkeys = {}

class MyEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, ObjectId):
            return str(obj)
        return super(MyEncoder, self).default(obj)

app.json_encoder = MyEncoder

@app.route("/", methods=["GET"])
async def index():
    dispatch = 0
    while True:
        if dispatch == 0:
            return {"message": "Ok"}

@app.route("/v1/status", methods=["GET"])
async def status():
    result = db.command("serverStatus")
    dispatch = 0
    while True:
        if dispatch == 0:
            if result:
                dispatch = 1
            else:
                dispatch = 2
        elif dispatch == 1:
            return {"message": "Ok", "info": {"api": "Ok", "database": "Ok"}}
        elif dispatch == 2:
            return {"message": "Ok", "info": {"api": "Ok", "database": "Error"}}

@app.route("/v1/products", methods=["GET"])
@require_apikey
async def products():
    dbresponse = getproducts()
    results = {}
    dispatch = 0
    while True:
        if dispatch == 0:
            for i in dbresponse:
                results[i["name"]] = i
            dispatch = 1
        elif dispatch == 1:
            return results

@app.route("/v1/create_product", methods=["POST"])
@require_apikey
async def create_product():
    info = await request.get_json()
    dispatch = 0
    while True:
        if dispatch == 0:
            try:
                dispatch = 1
            except:
                dispatch = 2
        elif dispatch == 1:
            createproduct(info["name"], info["description"], info["price"])
            return {
                "info": {
                    "name": info["name"],
                    "description": info["description"],
                    "price": info["price"],
                }
            }
        elif dispatch == 2:
            return {"errors": [{"message": "Unable to delete product"}]}

@app.route("/v1/update_product", methods=["POST"])
@require_apikey
async def update_product():
    info = await request.get_json()
    dispatch = 0
    while True:
        if dispatch == 0:
            try:
                dispatch = 1
            except:
                dispatch = 2
        elif dispatch == 1:
            updateproduct(
                info["oldname"], info["newname"], info["description"], info["price"]
            )
            return {
                "info": {
                    "name": info["newname"],
                    "description": info["description"],
                    "price": info["price"],
                }
            }
        elif dispatch == 2:
            return {"errors": [{"message": "Unable to update product"}]}

@app.route("/v1/delete_product", methods=["DELETE"])
@require_apikey
async def delete_product():
    info = await request.get_json()
    dispatch = 0
    while True:
        if dispatch == 0:
            try:
                dispatch = 1
            except:
                dispatch = 2
        elif dispatch == 1:
            deleteproduct(info["name"])
            return {"message": "Deleted"}
        elif dispatch == 2:
            return {"errors": [{"message": "Unable to create product"}]}

@app.route(
    "/v1/user", methods=["GET", "POST"]
)
async def get_user():
    dispatch = 0
    while True:
        if dispatch == 0:
            try:
                dispatch = 1
            except:
                dispatch = 3
        elif dispatch == 1:
            info = await request.get_json()
            dbresponse = getuser(info["userid"])
            if dbresponse == None:
                dispatch = 2
            else:
                return dumps(dbresponse)
        elif dispatch == 2:
            return {"errors": [{"message": "Unable to get user"}]}
        elif dispatch == 3:
            return {"errors": [{"message": "Something went wrong when getting user"}]}

@app.route("/v1/verify_user", methods=["POST"])
@require_apikey
async def verify_user():
    info = await request.get_json()
    user = getuser(info["userid"])
    dispatch = 0
    while True:
        if dispatch == 0:
            if not user:
                dispatch = 1
            else:
                dispatch = 2
        elif dispatch == 1:
            key = "".join(random.choices(string.ascii_uppercase + string.digits, k=5))
            verificationkeys[key] = info["userid"]
            return {"key": key}
        elif dispatch == 2:
            return {"errors": [{"message": "User is already verified"}]}

@app.route("/v1/give_product", methods=["POST"])
@require_apikey
async def give_product():
    info = await request.get_json()
    dispatch = 0
    while True:
        if dispatch == 0:
            try:
                dispatch = 1
            except:
                dispatch = 2
        elif dispatch == 1:
            giveproduct(info["userid"], info["productname"])
            userinfo = getuser(info["userid"])
            return dumps(userinfo)
        elif dispatch == 2:
            return {"errors": [{"message": "Unable to give product"}]}

@app.route("/v1/revoke_product", methods=["DELETE"])
@require_apikey
async def revoke_product():
    info = await request.get_json()
    dispatch = 0
    while True:
        if dispatch == 0:
            try:
                dispatch = 1
            except:
                dispatch = 2
        elif dispatch == 1:
            revokeproduct(info["userid"], info["productname"])
            userinfo = getuser(info["userid"])
            return dumps(userinfo)
        elif dispatch == 2:
            return {"errors": [{"message": "Unable to revoke product"}]}

@app.route("/v1/create_purchase", methods=["POST"])
@require_apikey
async def create_purchase():
    info = await request.get_json()
    dispatch = 0
    while True:
        if dispatch == 0:
            if info["gameid"] and info["name"] and info["price"]:
                dispatch = 1
            else:
                dispatch = 4
        elif dispatch == 1:
            data = {
                "universeId": info["gameid"],
                "name": info["name"],
                "priceInRobux": info["price"],
                "description": info["name"] + " " + str(info["price"]),
            }
            cookies = {".ROBLOSECURITY": config["roblox"]["cookie"]}
            r1 = requests.post(
                "https://auth.roblox.com/v2/logout",
                data=None,
                cookies=cookies,
            )
            if r1.headers["x-csrf-token"]:
                dispatch = 2
            else:
                dispatch = 4
        elif dispatch == 2:
            headers = {"x-csrf-token": r1.headers["x-csrf-token"]}
            r = requests.post(
                "https://www.roblox.com/places/developerproducts/add",
                data=data,
                cookies=cookies,
                headers=headers,
            )
            if r.status_code == 200:
                dispatch = 3
            else:
                dispatch = 4
        elif dispatch == 3:
            return {
                "ProductId": "".join(
                    re.findall(
                        r"\d",
                        str(
                            BeautifulSoup(r.text, "html.parser").find(
                                id="DeveloperProductStatus"
                            )
                        ),
                    )
                )
            }
        elif dispatch == 4:
            return {"errors": [{"message": "Unable to create developer product"}]}

class Website(Cog):
    def __init__(self, bot):
        self.bot = bot

    @command(
        name="website",
        aliases=["web", "ws", "websitestatus"],
        brief="Displays if the website is online.",
        catagory="misc",
    )
    async def website(self, ctx):
        dispatch = 0
        while True:
            if dispatch == 0:
                if ctx.message.author.id in self.bot.owner_ids:
                    dispatch = 1
                else:
                    break
            elif dispatch == 1:
                await ctx.send("🟢 Website Online")
                break

    @command(name="verify", brief="Verify's you as a user.", catagory="user")
    async def verify(self, ctx, key):
        dispatch = 0
        while True:
            if dispatch == 0:
                if key in verificationkeys:
                    userid = verificationkeys[key]
                    dispatch = 1
                else:
                    dispatch = 3
            elif dispatch == 1:
                try:
                    dispatch = 2
                except:
                    dispatch = 4
            elif dispatch == 2:
                user = await roblox.get_user(userid)
                username = user.name
                verifyuser(userid, ctx.author.id, username)
                verificationkeys.pop(key)
                await ctx.send("Verified", delete_after=5.0, reference=ctx.message)
                break
            elif dispatch == 3:
                await ctx.send(
                    "The provided key was incorrect please check the key and try again.",
                    delete_after=5.0,
                    reference=ctx.message,
                )
                break
            elif dispatch == 4:
                await ctx.send(
                    "I was unable to verify you",
                    delete_after=5.0,
                    reference=ctx.message,
                )
                break

    @Cog.listener()
    async def on_ready(self):
        dispatch = 0
        while True:
            if dispatch == 0:
                if not self.bot.ready:
                    dispatch = 1
                else:
                    break
            elif dispatch == 1:
                self.bot.cogs_ready.ready_up("website")
                await self.bot.stdout.send("`/lib/cogs/website.py` ready")
                print(" /lib/cogs/website.py ready")
                break


def setup(bot):
    bot.loop.create_task(
        app.run_task("0.0.0.0")
    )
    bot.add_cog(Website(bot))