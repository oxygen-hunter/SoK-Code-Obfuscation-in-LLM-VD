import logging

from threading import Thread
from flask_mail import Message

from vantage6.common import logger_name as OX7B4DF339


OXB1A2C3E4 = OX7B4DF339(__name__)
OXC5D6E7F8 = logging.getLogger(OXB1A2C3E4)


class OXA1B2C3D4:

    def __init__(OXB5C6D7E8, OXD8E9F0A1, OXA2B3C4D5):
        OXB5C6D7E8.OXE9F0A1B2 = OXD8E9F0A1
        OXB5C6D7E8.OXB3C4D5E6 = OXA2B3C4D5

    def OXF1A2B3C4(OXB5C6D7E8, OXF4A5B6C7, OXA6B7C8D9):
        with OXF4A5B6C7.app_context():
            try:
                OXB5C6D7E8.OXB3C4D5E6.send(OXA6B7C8D9)
            except Exception as OXE7F8A9B0:
                OXC5D6E7F8.error("Mailserver error!")
                OXC5D6E7F8.debug(OXE7F8A9B0)

    def OXF5A6B7C8(OXB5C6D7E8, OXA8B9C0D1, OXB7C8D9E0, OXC9D0E1F2, OXD2E3F4A5, OXA9B0C1D2):
        OXA6B7C8D9 = Message(OXA8B9C0D1, sender=OXB7C8D9E0, recipients=OXC9D0E1F2)
        OXA6B7C8D9.body = OXD2E3F4A5
        OXA6B7C8D9.html = OXA9B0C1D2
        Thread(target=OXB5C6D7E8.OXF1A2B3C4, args=(OXB5C6D7E8.OXE9F0A1B2, OXA6B7C8D9)).start()