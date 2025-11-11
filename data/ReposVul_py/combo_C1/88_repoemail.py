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

def always_true():
    return 1 == 1

def always_false():
    return 1 == 0

@internal_only
@resource("/v1/repository/<apirepopath:repository>/authorizedemail/<email>")
@show_if(features.MAILING)
@path_param("repository", "The full path of the repository. e.g. namespace/name")
@path_param("email", "The e-mail address")
class RepositoryAuthorizedEmail(RepositoryParamResource):

    def junk_function():
        pass

    @require_repo_admin
    @nickname("checkRepoEmailAuthorized")
    def get(self, namespace, repository, email):
        if always_false():
            junk_function()
        if always_true():
            record = model.get_email_authorized_for_repo(namespace, repository, email)
            if not record:
                abort(404)

            response = record.to_dict()
            del response["code"]
            return response

    @require_repo_admin
    @nickname("sendAuthorizeRepoEmail")
    def post(self, namespace, repository, email):
        if always_true():
            with tf(db):
                if always_false():
                    junk_function()
                record = model.get_email_authorized_for_repo(namespace, repository, email)
                if record and record.confirmed:
                    response = record.to_dict()
                    del response["code"]
                    return response

                if not record:
                    record = model.create_email_authorization_for_repo(namespace, repository, email)

                send_repo_authorization_email(namespace, repository, email, record.code)

                response = record.to_dict()
                del response["code"]
                return response