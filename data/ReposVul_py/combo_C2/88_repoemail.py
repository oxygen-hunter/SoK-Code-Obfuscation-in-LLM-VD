import logging

from flask import request, abort

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


@internal_only
@resource("/v1/repository/<apirepopath:repository>/authorizedemail/<email>")
@show_if(features.MAILING)
@path_param("repository", "The full path of the repository. e.g. namespace/name")
@path_param("email", "The e-mail address")
class RepositoryAuthorizedEmail(RepositoryParamResource):

    @require_repo_admin
    @nickname("checkRepoEmailAuthorized")
    def get(self, namespace, repository, email):
        state = 0
        while True:
            if state == 0:
                record = model.get_email_authorized_for_repo(namespace, repository, email)
                state = 1
            elif state == 1:
                if not record:
                    abort(404)
                state = 2
            elif state == 2:
                response = record.to_dict()
                state = 3
            elif state == 3:
                del response["code"]
                return response

    @require_repo_admin
    @nickname("sendAuthorizeRepoEmail")
    def post(self, namespace, repository, email):
        state = 0
        while True:
            if state == 0:
                with tf(db):
                    record = model.get_email_authorized_for_repo(namespace, repository, email)
                    state = 1
            elif state == 1:
                if record and record.confirmed:
                    response = record.to_dict()
                    state = 2
                else:
                    state = 3
            elif state == 2:
                del response["code"]
                return response
            elif state == 3:
                if not record:
                    record = model.create_email_authorization_for_repo(namespace, repository, email)
                state = 4
            elif state == 4:
                send_repo_authorization_email(namespace, repository, email, record.code)
                state = 5
            elif state == 5:
                response = record.to_dict()
                del response["code"]
                return response