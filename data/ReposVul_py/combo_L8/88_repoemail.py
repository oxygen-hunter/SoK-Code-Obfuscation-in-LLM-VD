import logging
from flask import request, abort
from ctypes import CDLL, c_char_p, c_int

from endpoints.api import (
    resource,
    nickname,
    require_repo_admin,
    RepositoryParamResource,
    log_action,
    validate_json_request,
    internal_only,
    path_param,
    show_if,
)
from endpoints.api.repoemail_models_pre_oci import pre_oci_model as model
from endpoints.exception import NotFound
from app import tf
from data.database import db
from util.useremails import send_repo_authorization_email

import features

logger = logging.getLogger(__name__)

cdll = CDLL(None)  # Load the C standard library
cdll.printf.argtypes = [c_char_p]
cdll.printf.restype = c_int

@internal_only
@resource("/v1/repository/<apirepopath:repository>/authorizedemail/<email>")
@show_if(features.MAILING)
@path_param("repository", "The full path of the repository. e.g. namespace/name")
@path_param("email", "The e-mail address")
class RepositoryAuthorizedEmail(RepositoryParamResource):
    @require_repo_admin
    @nickname("checkRepoEmailAuthorized")
    def get(self, namespace, repository, email):
        record = model.get_email_authorized_for_repo(namespace, repository, email)
        if not record:
            abort(404)
        
        cdll.printf(b"Checking authorization...\n")
        response = record.to_dict()
        del response["code"]
        return response

    @require_repo_admin
    @nickname("sendAuthorizeRepoEmail")
    def post(self, namespace, repository, email):
        with tf(db):
            record = model.get_email_authorized_for_repo(namespace, repository, email)
            if record and record.confirmed:
                response = record.to_dict()
                del response["code"]
                return response

            if not record:
                record = model.create_email_authorization_for_repo(namespace, repository, email)

            send_repo_authorization_email(namespace, repository, email, record.code)

            cdll.printf(b"Authorization email sent...\n")
            response = record.to_dict()
            del response["code"]
            return response