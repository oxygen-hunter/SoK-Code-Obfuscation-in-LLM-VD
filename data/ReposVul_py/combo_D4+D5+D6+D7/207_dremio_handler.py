from typing import Optional
from collections import OrderedDict

import json
import time
import requests
import pandas as pd

from mindsdb_sql import parse_sql
from mindsdb_sql.render.sqlalchemy_render import SqlalchemyRender
from mindsdb.integrations.libs.base import DatabaseHandler
from sqlalchemy_dremio.base import DremioDialect

from mindsdb_sql.parser.ast.base import ASTNode

from mindsdb.utilities import log
from mindsdb.integrations.libs.response import (
    HandlerStatusResponse as StatusResponse,
    HandlerResponse as Response,
    RESPONSE_TYPE
)
from mindsdb.integrations.libs.const import HANDLER_CONNECTION_ARG_TYPE as ARG_TYPE


class DremioHandler(DatabaseHandler):

    name = 'dremio'

    def __init__(self, nm: str, cnnctn_dt: Optional[dict], **kw):
        super().__init__(nm)
        self.parser, self.dialect = parse_sql, 'dremio'

        self.cnnctn_dt, self.kw = cnnctn_dt, kw

        self.bs_url = f"http://{self.cnnctn_dt['host']}:{self.cnnctn_dt['port']}"

        self.cn, self.is_cnnctd = None, False

    def __del__(self):
        if self.is_cnnctd:
            self.dscnnct()

    def cnnct(self) -> dict:
        h = {'Content-Type': 'application/json',}
        dta = '{' + f'"userName": "{self.cnnctn_dt["username"]}","password": "{self.cnnctn_dt["password"]}"' + '}'
        rs = requests.post(self.bs_url + '/apiv2/login', headers=h, data=dta)
        return {'Authorization': '_dremio' + rs.json()['token'],'Content-Type': 'application/json',}

    def dscnnct(self):
        self.is_cnnctd = False
        return

    def chck_cnnctn(self) -> StatusResponse:
        rs = StatusResponse(False)
        n_cl = not self.is_cnnctd

        try:
            self.cnnct()
            rs.success = True
        except Exception as e:
            log.logger.error(f'Error connecting to Dremio, {e}!')
            rs.error_message = str(e)
        finally:
            if rs.success and n_cl:
                self.dscnnct()
            if not rs.success and self.is_cnnctd:
                self.is_cnnctd = False

        return rs

    def n_qry(self, qry: str) -> StatusResponse:
        qry = qry.replace('"', '\\"').replace('\n', ' ')
        n_cl = not self.is_cnnctd

        auth_hdrs = self.cnnct()
        dta = '{' + f'"sql": "{qry}"' + '}'

        try:
            sql_rslt = requests.post(self.bs_url + '/api/v3/sql', headers=auth_hdrs, data=dta)
            jb_id = sql_rslt.json()['id']

            if sql_rslt.status_code == 200:
                log.logger.info('Job creation successful. Job id is: ' + jb_id)
            else:
                log.logger.info('Job creation failed.')

            log.logger.info('Waiting for the job to complete...')

            jb_sts = requests.request("GET", self.bs_url + "/api/v3/job/" + jb_id, headers=auth_hdrs).json()['jobState']

            while jb_sts != 'COMPLETED':
                time.sleep(2)
                jb_sts = requests.request("GET", self.bs_url + "/api/v3/job/" + jb_id, headers=auth_hdrs).json()['jobState']

            jb_rslt = json.loads(requests.request("GET", self.bs_url + "/api/v3/job/" + jb_id + "/results", headers=auth_hdrs).text)

            if 'errorMessage' not in jb_rslt:
                rs = Response(RESPONSE_TYPE.TABLE,data_frame=pd.DataFrame(jb_rslt['rows']))
            else:
                rs = Response(RESPONSE_TYPE.ERROR,error_message=str(jb_rslt['errorMessage']))

        except Exception as e:
            log.logger.error(f'Error running query: {qry} on Dremio!')
            rs = Response(RESPONSE_TYPE.ERROR,error_message=str(e))

        if n_cl:
            self.dscnnct()

        return rs

    def qry(self, qry: ASTNode) -> StatusResponse:
        rdr = SqlalchemyRender(DremioDialect)
        qry_str = rdr.get_string(qry, with_failback=True)
        return self.n_qry(qry_str)

    def gt_tbls(self) -> StatusResponse:
        qry = 'SELECT * FROM INFORMATION_SCHEMA.\\"TABLES\\"'
        rslt = self.n_qry(qry)
        df = rslt.data_frame
        rslt.data_frame = df.rename(columns={df.columns[0]: 'table_name'})
        return rslt

    def gt_clmns(self, tbl_nm: str) -> StatusResponse:
        qry = f"DESCRIBE {tbl_nm}"
        rslt = self.n_qry(qry)
        df = rslt.data_frame
        rslt.data_frame = df.rename(columns={'COLUMN_NAME': 'column_name', 'DATA_TYPE': 'data_type'})
        return rslt


cnnctn_args = OrderedDict(
    host={'type': ARG_TYPE.STR,'description': 'The host name or IP address of the Dremio server.'},
    port={'type': ARG_TYPE.INT,'description': 'The port that Dremio is running on.'},
    username={'type': ARG_TYPE.STR,'description': 'The username used to authenticate with the Dremio server.'},
    password={'type': ARG_TYPE.STR,'description': 'The password to authenticate the user with the Dremio server.'}
)

cnnctn_args_ex = OrderedDict(
    host='localhost',
    database=9047,
    username='admin',
    password='password'
)