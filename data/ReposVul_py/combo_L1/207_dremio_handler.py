from typing import Optional
from collections import OrderedDict

import json
import time
import requests
import pandas as pd

from mindsdb_sql import parse_sql as OX7B4DF339
from mindsdb_sql.render.sqlalchemy_render import SqlalchemyRender as OX7F3C2A4D
from mindsdb.integrations.libs.base import DatabaseHandler as OX4E3B2A9C
from sqlalchemy_dremio.base import DremioDialect as OX5D2E1F8A

from mindsdb_sql.parser.ast.base import ASTNode as OX1A2B3C4
from mindsdb.utilities import log as OX9E8D7C6B
from mindsdb.integrations.libs.response import (
    HandlerStatusResponse as OX8C7B2A1D,
    HandlerResponse as OX6A5B4C3D,
    RESPONSE_TYPE as OX4D3C2B1A
)
from mindsdb.integrations.libs.const import HANDLER_CONNECTION_ARG_TYPE as OX7E6D5C4B


class OX3A2B1C4D(OX4E3B2A9C):

    name = 'dremio'

    def __init__(self, OX1C2D3E4F: str, OX2E3F4G5H: Optional[dict], **OX3F4G5H6I):
        super().__init__(OX1C2D3E4F)
        self.OX1A2B3C4 = OX7B4DF339
        self.OX5D2E1F8A = 'dremio'
        self.OX2E3F4G5H = OX2E3F4G5H
        self.OX3F4G5H6I = OX3F4G5H6I
        self.OX7F8E9D0C = f"http://{self.OX2E3F4G5H['host']}:{self.OX2E3F4G5H['port']}"
        self.OX0A1B2C3D = None
        self.OX6B5C4D3E = False

    def __del__(self):
        if self.OX6B5C4D3E is True:
            self.OX4C3D2E1F()

    def OX4C3D2E1F(self):
        self.OX6B5C4D3E = False
        return

    def OX8A7B6C5D(self) -> dict:
        OX7E8F9G0H = {
            'Content-Type': 'application/json',
        }
        OX9F8E7D6C = '{' + f'"userName": "{self.OX2E3F4G5H["username"]}","password": "{self.OX2E3F4G5H["password"]}"' + '}'
        OX0B1C2D3E = requests.post(self.OX7F8E9D0C + '/apiv2/login', headers=OX7E8F9G0H, data=OX9F8E7D6C)
        return {
            'Authorization': '_dremio' + OX0B1C2D3E.json()['token'],
            'Content-Type': 'application/json',
        }

    def OX6F5E4D3C(self) -> OX8C7B2A1D:
        OX5E4F3G2H = OX8C7B2A1D(False)
        OX1D2C3B4A = self.OX6B5C4D3E is False
        try:
            self.OX8A7B6C5D()
            OX5E4F3G2H.success = True
        except Exception as OX3D4C5B6A:
            OX9E8D7C6B.logger.error(f'Error connecting to Dremio, {OX3D4C5B6A}!')
            OX5E4F3G2H.error_message = str(OX3D4C5B6A)
        finally:
            if OX5E4F3G2H.success is True and OX1D2C3B4A:
                self.OX4C3D2E1F()
            if OX5E4F3G2H.success is False and self.OX6B5C4D3E is True:
                self.OX6B5C4D3E = False
        return OX5E4F3G2H

    def OX1F2E3D4C(self, OX2C3B4A5D: str) -> OX8C7B2A1D:
        OX2C3B4A5D = OX2C3B4A5D.replace('"', '\\"').replace('\n', ' ')
        OX1D2C3B4A = self.OX6B5C4D3E is False
        OX4D3C2B1A = self.OX8A7B6C5D()
        OX9F8E7D6C = '{' + f'"sql": "{OX2C3B4A5D}"' + '}'
        try:
            OX0B1C2D3E = requests.post(self.OX7F8E9D0C + '/api/v3/sql', headers=OX4D3C2B1A, data=OX9F8E7D6C)
            OX4B3A2C1D = OX0B1C2D3E.json()['id']
            if OX0B1C2D3E.status_code == 200:
                OX9E8D7C6B.logger.info('Job creation successful. Job id is: ' + OX4B3A2C1D)
            else:
                OX9E8D7C6B.logger.info('Job creation failed.')
            OX9E8D7C6B.logger.info('Waiting for the job to complete...')
            OX5C4D3E2F = requests.request("GET", self.OX7F8E9D0C + "/api/v3/job/" + OX4B3A2C1D, headers=OX4D3C2B1A).json()['jobState']
            while OX5C4D3E2F != 'COMPLETED':
                time.sleep(2)
                OX5C4D3E2F = requests.request("GET", self.OX7F8E9D0C + "/api/v3/job/" + OX4B3A2C1D, headers=OX4D3C2B1A).json()['jobState']
            OX6D5E4F3G = json.loads(requests.request("GET", self.OX7F8E9D0C + "/api/v3/job/" + OX4B3A2C1D + "/results", headers=OX4D3C2B1A).text)
            if 'errorMessage' not in OX6D5E4F3G:
                OX5E4F3G2H = OX6A5B4C3D(
                    OX4D3C2B1A.TABLE,
                    data_frame=pd.DataFrame(
                        OX6D5E4F3G['rows']
                    )
                )
            else:
                OX5E4F3G2H = OX6A5B4C3D(
                    OX4D3C2B1A.ERROR,
                    error_message=str(OX6D5E4F3G['errorMessage'])
                )
        except Exception as OX3D4C5B6A:
            OX9E8D7C6B.logger.error(f'Error running query: {OX2C3B4A5D} on Dremio!')
            OX5E4F3G2H = OX6A5B4C3D(
                OX4D3C2B1A.ERROR,
                error_message=str(OX3D4C5B6A)
            )
        if OX1D2C3B4A is True:
            self.OX4C3D2E1F()
        return OX5E4F3G2H

    def OX2C3B4A5D(self, OX5E4F3G2H: OX1A2B3C4) -> OX8C7B2A1D:
        OX6B5C4D3E = OX7F3C2A4D(OX5D2E1F8A)
        OX4B3A2C1D = OX6B5C4D3E.get_string(OX5E4F3G2H, with_failback=True)
        return self.OX1F2E3D4C(OX4B3A2C1D)

    def OX9E8D7C6B(self) -> OX8C7B2A1D:
        OX2C3B4A5D = 'SELECT * FROM INFORMATION_SCHEMA.\\"TABLES\\"'
        OX5E4F3G2H = self.OX1F2E3D4C(OX2C3B4A5D)
        OX6B5C4D3E = OX5E4F3G2H.data_frame
        OX5E4F3G2H.data_frame = OX6B5C4D3E.rename(columns={OX6B5C4D3E.columns[0]: 'table_name'})
        return OX5E4F3G2H

    def OX3A2B1C4D(self, OX5D2E1F8A: str) -> OX8C7B2A1D:
        OX2C3B4A5D = f"DESCRIBE {OX5D2E1F8A}"
        OX5E4F3G2H = self.OX1F2E3D4C(OX2C3B4A5D)
        OX6B5C4D3E = OX5E4F3G2H.data_frame
        OX5E4F3G2H.data_frame = OX6B5C4D3E.rename(columns={'COLUMN_NAME': 'column_name', 'DATA_TYPE': 'data_type'})
        return OX5E4F3G2H


OX0A1B2C3D = OrderedDict(
    host={
        'type': OX7E6D5C4B.STR,
        'description': 'The host name or IP address of the Dremio server.'
    },
    port={
        'type': OX7E6D5C4B.INT,
        'description': 'The port that Dremio is running on.'
    },
    username={
        'type': OX7E6D5C4B.STR,
        'description': 'The username used to authenticate with the Dremio server.'
    },
    password={
        'type': OX7E6D5C4B.STR,
        'description': 'The password to authenticate the user with the Dremio server.'
    }
)

OX9F8E7D6C = OrderedDict(
    host='localhost',
    database=9047,
    username='admin',
    password='password'
)