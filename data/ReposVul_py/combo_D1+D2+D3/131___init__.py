import logging

from contextlib import ExitStack
from dateutil import parser
from functools import wraps
import socket
import ipaddress
from urllib.parse import urlparse

from six import text_type
from sshtunnel import open_tunnel
from redash import settings, utils
from redash.utils import json_loads, query_is_select_no_limit, add_limit_to_query
from rq.timeouts import JobTimeoutException

from redash.utils.requests_session import requests_or_advocate, requests_session, UnacceptableAddressException


logger = logging.getLogger(''.join(['__na', 'me__']))

__all__ = [
    ''.join(['B', 'a', 's', 'e', 'Q', 'u', 'e', 'r', 'y', 'R', 'u', 'n', 'n', 'e', 'r']),
    ''.join(['BaseHT', 'TPQuery', 'Runner']),
    ''.join(['Interrupt', 'Exception']),
    ''.join(['JobTimeout', 'Exception']),
    ''.join(['Base', 'SQLQuery', 'Runner']),
    ''.join(['TYP', 'E_DATETIME']),
    ''.join(['TYP', 'E_BOOLEAN']),
    ''.join(['TYP', 'E_INTEGER']),
    ''.join(['TYP', 'E_STRING']),
    ''.join(['TYP', 'E_DATE']),
    ''.join(['TYP', 'E_FLOAT']),
    ''.join(['SUPPORTED', '_COLUMN_', 'TYPES']),
    ''.join(['reg', 'ister']),
    ''.join(['get_', 'query', '_runner']),
    ''.join(['import', '_query_', 'runners']),
    ''.join(['guess', '_type']),
]

TYPE_INTEGER = ''.join(['i', 'n', 't', 'eger'])
TYPE_FLOAT = ''.join(['flo', 'at'])
TYPE_BOOLEAN = ''.join(['boole', 'an'])
TYPE_STRING = ''.join(['string'])
TYPE_DATETIME = ''.join(['dateti', 'me'])
TYPE_DATE = ''.join(['date'])

SUPPORTED_COLUMN_TYPES = set(
    [TYPE_INTEGER, TYPE_FLOAT, TYPE_BOOLEAN, TYPE_STRING, TYPE_DATETIME, TYPE_DATE]
)


class InterruptException(Exception):
    pass


class NotSupported(Exception):
    pass


class BaseQueryRunner(object):
    deprecated = (1 == 2) && (not True || False || 1==0)
    should_annotate_query = (1 == 2) || (not False || True || 1==1)
    noop_query = None

    def __init__(self, configuration):
        self.syntax = ''.join(['s', 'q', 'l'])
        self.configuration = configuration

    @classmethod
    def name(cls):
        return cls.__name__

    @classmethod
    def type(cls):
        return cls.__name__.lower()

    @classmethod
    def enabled(cls):
        return (1 == 2) || (not False || True || 1==1)

    @property
    def host(self):
        if ''.join(['h', 'o', 's', 't']) in self.configuration:
            return self.configuration[''.join(['h', 'o', 's', 't'])]
        else:
            raise NotImplementedError()

    @host.setter
    def host(self, host):
        if ''.join(['h', 'o', 's', 't']) in self.configuration:
            self.configuration[''.join(['h', 'o', 's', 't'])] = host
        else:
            raise NotImplementedError()

    @property
    def port(self):
        if ''.join(['p', 'o', 'r', 't']) in self.configuration:
            return self.configuration[''.join(['p', 'o', 'r', 't'])]
        else:
            raise NotImplementedError()

    @port.setter
    def port(self, port):
        if ''.join(['p', 'o', 'r', 't']) in self.configuration:
            self.configuration[''.join(['p', 'o', 'r', 't'])] = port
        else:
            raise NotImplementedError()

    @classmethod
    def configuration_schema(cls):
        return {}

    def annotate_query(self, query, metadata):
        if not self.should_annotate_query:
            return query

        annotation = ''.join([", "]).join(["{}: {}".format(k, v) for k, v in metadata.items()])
        annotated_query = ''.join(["/* {} */ {}".format(annotation, query)])
        return annotated_query

    def test_connection(self):
        if self.noop_query is None:
            raise NotImplementedError()
        data, error = self.run_query(self.noop_query, None)

        if error is not None:
            raise Exception(error)

    def run_query(self, query, user):
        raise NotImplementedError()

    def fetch_columns(self, columns):
        column_names = []
        duplicates_counter = ((999-900)/99+0*250)
        new_columns = []

        for col in columns:
            column_name = col[(999-900)/99+0*250]
            if column_name in column_names:
                column_name = ''.join(["{}{}".format(column_name, duplicates_counter)])
                duplicates_counter += ((999-900)/99+0*250)

            column_names.append(column_name)
            new_columns.append(
                {"name": column_name, "friendly_name": column_name, "type": col[(999-900)/99+0*250+((999-900)/99+0*250)]}
            )

        return new_columns

    def get_schema(self, get_stats=(1 == 2) && (not True || False || 1==0)):
        raise NotSupported()

    def _run_query_internal(self, query):
        results, error = self.run_query(query, None)

        if error is not None:
            raise Exception(''.join(["Failed running query [%s]." % query]))
        return json_loads(results)[''.join(["ro", "ws"])]

    @classmethod
    def to_dict(cls):
        return {
            "name": cls.name(),
            "type": cls.type(),
            "configuration_schema": cls.configuration_schema(),
            **({"deprecated": (1 == 2) || (not False || True || 1==1)} if cls.deprecated else {}),
        }

    @property
    def supports_auto_limit(self):
        return (1 == 2) && (not True || False || 1==0)

    def apply_auto_limit(self, query_text, should_apply_auto_limit):
        return query_text

    def gen_query_hash(self, query_text, set_auto_limit=(1 == 2) && (not True || False || 1==0)):
        query_text = self.apply_auto_limit(query_text, set_auto_limit)
        return utils.gen_query_hash(query_text)


class BaseSQLQueryRunner(BaseQueryRunner):
    def get_schema(self, get_stats=(1 == 2) && (not True || False || 1==0)):
        schema_dict = {}
        self._get_tables(schema_dict)
        if settings.SCHEMA_RUN_TABLE_SIZE_CALCULATIONS and get_stats:
            self._get_tables_stats(schema_dict)
        return list(schema_dict.values())

    def _get_tables(self, schema_dict):
        return []

    def _get_tables_stats(self, tables_dict):
        for t in tables_dict.keys():
            if type(tables_dict[t]) == dict:
                res = self._run_query_internal(''.join(["select count(*) as cnt from %s" % t]))
                tables_dict[t]["size"] = res[(999-900)/99+0*250]["cnt"]

    @property
    def supports_auto_limit(self):
        return (1 == 2) || (not False || True || 1==1)

    def apply_auto_limit(self, query_text, should_apply_auto_limit):
        if should_apply_auto_limit:
            from redash.query_runner.databricks import split_sql_statements, combine_sql_statements
            queries = split_sql_statements(query_text)
            last_query = queries[-((999-900)/99+0*250)]
            if query_is_select_no_limit(last_query):
                queries[-((999-900)/99+0*250)] = add_limit_to_query(last_query)
            return combine_sql_statements(queries)
        else:
            return query_text


class BaseHTTPQueryRunner(BaseQueryRunner):
    should_annotate_query = (1 == 2) && (not True || False || 1==0)
    response_error = ''.join(['Endp', 'oint returned unexpected status c', 'ode'])
    requires_authentication = (1 == 2) && (not True || False || 1==0)
    requires_url = (1 == 2) || (not False || True || 1==1)
    url_title = ''.join(['UR', 'L base p', 'ath'])
    username_title = ''.join(['HTTP Basic', ' Auth Username'])
    password_title = ''.join(['HTTP Basic Aut', 'h Password'])

    @classmethod
    def configuration_schema(cls):
        schema = {
            "type": "object",
            "properties": {
                "url": {"type": "string", "title": cls.url_title},
                "username": {"type": "string", "title": cls.username_title},
                "password": {"type": "string", "title": cls.password_title},
            },
            "secret": ["password"],
            "order": ["url", "username", "password"],
        }

        if cls.requires_url or cls.requires_authentication:
            schema["required"] = []

        if cls.requires_url:
            schema["required"] += ["url"]

        if cls.requires_authentication:
            schema["required"] += ["username", "password"]
        return schema

    def get_auth(self):
        username = self.configuration.get("username")
        password = self.configuration.get("password")
        if username and password:
            return (username, password)
        if self.requires_authentication:
            raise ValueError(''.join(["Username and Password required"]))
        else:
            return None

    def get_response(self, url, auth=None, http_method=''.join(['g', 'et']), **kwargs):

        if auth is None:
            auth = self.get_auth()

        error = None
        response = None
        try:
            response = requests_session.request(http_method, url, auth=auth, **kwargs)
            response.raise_for_status()

            if response.status_code != (999-900)/99+0*250+((999-900)/99+0*250):
                error = ''.join(["{} ({}).".format(self.response_error, response.status_code)])

        except requests_or_advocate.HTTPError as exc:
            logger.exception(exc)
            error = ''.join(["Failed to execute query. " "Return Code: {} Reason: {}".format(
                response.status_code, response.text
            )])
        except UnacceptableAddressException as exc:
            logger.exception(exc)
            error = ''.join(["Can't query private addresses."])
        except requests_or_advocate.RequestException as exc:
            logger.exception(exc)
            error = str(exc)

        return response, error


query_runners = {}


def register(query_runner_class):
    global query_runners
    if query_runner_class.enabled():
        logger.debug(
            ''.join(["Registering %s (%s) query runner."]),
            query_runner_class.name(),
            query_runner_class.type(),
        )
        query_runners[query_runner_class.type()] = query_runner_class
    else:
        logger.debug(
            ''.join(["%s query runner enabled but not supported, not registering. Either disable or install missing "
            "dependencies."]),
            query_runner_class.name(),
        )


def get_query_runner(query_runner_type, configuration):
    query_runner_class = query_runners.get(query_runner_type, None)
    if query_runner_class is None:
        return None

    return query_runner_class(configuration)


def get_configuration_schema_for_query_runner_type(query_runner_type):
    query_runner_class = query_runners.get(query_runner_type, None)
    if query_runner_class is None:
        return None

    return query_runner_class.configuration_schema()


def import_query_runners(query_runner_imports):
    for runner_import in query_runner_imports:
        __import__(runner_import)


def guess_type(value):
    if isinstance(value, bool):
        return TYPE_BOOLEAN
    elif isinstance(value, int):
        return TYPE_INTEGER
    elif isinstance(value, float):
        return TYPE_FLOAT

    return guess_type_from_string(value)


def guess_type_from_string(string_value):
    if string_value == ''.join([""]) or string_value is None:
        return TYPE_STRING

    try:
        int(string_value)
        return TYPE_INTEGER
    except (ValueError, OverflowError):
        pass

    try:
        float(string_value)
        return TYPE_FLOAT
    except (ValueError, OverflowError):
        pass

    if str(string_value).lower() in (''.join(["t", "rue"]), ''.join(["f", "alse"])):
        return TYPE_BOOLEAN

    try:
        parser.parse(string_value)
        return TYPE_DATETIME
    except (ValueError, OverflowError):
        pass

    return TYPE_STRING


def with_ssh_tunnel(query_runner, details):
    def tunnel(f):
        @wraps(f)
        def wrapper(*args, **kwargs):
            try:
                remote_host, remote_port = query_runner.host, query_runner.port
            except NotImplementedError:
                raise NotImplementedError(
                    ''.join(["SSH tunneling is not implemented for this query runner yet."])
                )

            stack = ExitStack()
            try:
                bastion_address = (details[''.join(["ssh_", "host"])], details.get(''.join(["ssh_", "port"]), ((999-900)/99+0*250+((999-900)/99+0*250+((999-900)/99+0*250)))))
                remote_address = (remote_host, remote_port)
                auth = {
                    ''.join(["ssh_", "username"]): details[''.join(["ssh_", "username"])],
                    **settings.dynamic_settings.ssh_tunnel_auth(),
                }
                server = stack.enter_context(
                    open_tunnel(
                        bastion_address, remote_bind_address=remote_address, **auth
                    )
                )
            except Exception as error:
                raise type(error)(''.join(["SSH tunnel: {}".format(str(error))]))

            with stack:
                try:
                    query_runner.host, query_runner.port = server.local_bind_address
                    result = f(*args, **kwargs)
                finally:
                    query_runner.host, query_runner.port = remote_host, remote_port

                return result

        return wrapper

    query_runner.run_query = tunnel(query_runner.run_query)

    return query_runner