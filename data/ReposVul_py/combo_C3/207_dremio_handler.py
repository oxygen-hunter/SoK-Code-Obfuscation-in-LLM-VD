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

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.registers = {}

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            getattr(self, instr[0])(*instr[1:])
            self.pc += 1

    def PUSH(self, value):
        self.stack.append(value)

    def POP(self):
        return self.stack.pop()

    def LOAD(self, reg):
        self.stack.append(self.registers.get(reg, None))

    def STORE(self, reg):
        self.registers[reg] = self.stack.pop()

    def ADD(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def SUB(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def JMP(self, addr):
        self.pc = addr - 1

    def JZ(self, addr):
        if self.stack.pop() == 0:
            self.pc = addr - 1

class DremioHandler(DatabaseHandler):
    name = 'dremio'

    def __init__(self, name: str, connection_data: Optional[dict], **kwargs):
        super().__init__(name)
        self.connection_data = connection_data
        self.kwargs = kwargs
        self.base_url = f"http://{self.connection_data['host']}:{self.connection_data['port']}"
        self.connection = None
        self.is_connected = False
        self.vm = VM()

    def __del__(self):
        if self.is_connected is True:
            self.disconnect()

    def connect(self) -> dict:
        headers = {'Content-Type': 'application/json'}
        data = '{' + f'"userName": "{self.connection_data["username"]}","password": "{self.connection_data["password"]}"' + '}'
        response = requests.post(self.base_url + '/apiv2/login', headers=headers, data=data)
        return {'Authorization': '_dremio' + response.json()['token'], 'Content-Type': 'application/json'}

    def disconnect(self):
        self.is_connected = False
        return

    def check_connection(self) -> StatusResponse:
        response = StatusResponse(False)
        need_to_close = self.is_connected is False
        try:
            self.connect()
            response.success = True
        except Exception as e:
            log.logger.error(f'Error connecting to Dremio, {e}!')
            response.error_message = str(e)
        finally:
            if response.success is True and need_to_close:
                self.disconnect()
            if response.success is False and self.is_connected is True:
                self.is_connected = False
        return response

    def native_query(self, query: str) -> StatusResponse:
        query = query.replace('"', '\\"').replace('\n', ' ')
        need_to_close = self.is_connected is False
        auth_headers = self.connect()
        data = '{' + f'"sql": "{query}"' + '}'
        try:
            sql_result = requests.post(self.base_url + '/api/v3/sql', headers=auth_headers, data=data)
            job_id = sql_result.json()['id']
            if sql_result.status_code == 200:
                log.logger.info('Job creation successful. Job id is: ' + job_id)
            else:
                log.logger.info('Job creation failed.')
            log.logger.info('Waiting for the job to complete...')
            job_status = requests.request("GET", self.base_url + "/api/v3/job/" + job_id, headers=auth_headers).json()[
                'jobState']
            while job_status != 'COMPLETED':
                time.sleep(2)
                job_status = requests.request("GET", self.base_url + "/api/v3/job/" + job_id, headers=auth_headers).json()[
                    'jobState']
            job_result = json.loads(requests.request("GET", self.base_url + "/api/v3/job/" + job_id + "/results", headers=auth_headers).text)
            if 'errorMessage' not in job_result:
                response = Response(
                    RESPONSE_TYPE.TABLE,
                    data_frame=pd.DataFrame(
                        job_result['rows']
                    )
                )
            else:
                response = Response(
                    RESPONSE_TYPE.ERROR,
                    error_message=str(job_result['errorMessage'])
                )
        except Exception as e:
            log.logger.error(f'Error running query: {query} on Dremio!')
            response = Response(
                RESPONSE_TYPE.ERROR,
                error_message=str(e)
            )
        if need_to_close is True:
            self.disconnect()
        return response

    def query(self, query: ASTNode) -> StatusResponse:
        renderer = SqlalchemyRender(DremioDialect)
        query_str = renderer.get_string(query, with_failback=True)
        return self.native_query(query_str)

    def get_tables(self) -> StatusResponse:
        query = 'SELECT * FROM INFORMATION_SCHEMA.\\"TABLES\\"'
        result = self.native_query(query)
        df = result.data_frame
        result.data_frame = df.rename(columns={df.columns[0]: 'table_name'})
        return result

    def get_columns(self, table_name: str) -> StatusResponse:
        query = f"DESCRIBE {table_name}"
        result = self.native_query(query)
        df = result.data_frame
        result.data_frame = df.rename(columns={'COLUMN_NAME': 'column_name', 'DATA_TYPE': 'data_type'})
        return result

connection_args = OrderedDict(
    host={
        'type': ARG_TYPE.STR,
        'description': 'The host name or IP address of the Dremio server.'
    },
    port={
        'type': ARG_TYPE.INT,
        'description': 'The port that Dremio is running on.'
    },
    username={
        'type': ARG_TYPE.STR,
        'description': 'The username used to authenticate with the Dremio server.'
    },
    password={
        'type': ARG_TYPE.STR,
        'description': 'The password to authenticate the user with the Dremio server.'
    }
)

connection_args_example = OrderedDict(
    host='localhost',
    database=9047,
    username='admin',
    password='password'
)