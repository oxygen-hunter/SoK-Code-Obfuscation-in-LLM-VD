import logging

from flask import request as OX5D1A1B5C, abort as OX2F4B3A6E

from endpoints.api import (
    resource as OX1C3D2E4F,
    nickname as OX4A5B6C7D,
    require_repo_admin as OX2E3F1D4C,
    RepositoryParamResource as OX7A8B9C0D,
    log_action as OX3C2D1E4F,
    validate_json_request as OX5E6F7A8B,
    internal_only as OX9C0D1E2F,
    path_param as OX6B7D8A9C,
    show_if as OX8E9F0A1B,
)
from endpoints.api.repoemail_models_pre_oci import pre_oci_model as OX3A4B5C6D
from endpoints.exception import NotFound as OX1B2C3D4E
from app import tf as OX7F8E9D0C
from data.database import db as OX5A6B7C8D
from util.useremails import send_repo_authorization_email as OX4D5E6F7A

import features as OX0A1B2C3D

OX6F5E4D3C = logging.getLogger(__name__)


@OX9C0D1E2F
@OX1C3D2E4F("/v1/repository/<apirepopath:repository>/authorizedemail/<email>")
@OX8E9F0A1B(OX0A1B2C3D.MAILING)
@OX6B7D8A9C("repository", "The full path of the repository. e.g. namespace/name")
@OX6B7D8A9C("email", "The e-mail address")
class OX9D8C7B6A(OX7A8B9C0D):
    """
    Resource for checking and authorizing e-mail addresses to receive repo notifications.
    """

    @OX2E3F1D4C
    @OX4A5B6C7D("checkRepoEmailAuthorized")
    def get(self, OX2A3B4C5D, OX5D4C3B2A, OX3B2A1C4D):
        """
        Checks to see if the given e-mail address is authorized on this repository.
        """
        OX8A7B6C5D = OX3A4B5C6D.get_email_authorized_for_repo(OX2A3B4C5D, OX5D4C3B2A, OX3B2A1C4D)
        if not OX8A7B6C5D:
            OX2F4B3A6E(404)

        OX9B8C7D6E = OX8A7B6C5D.to_dict()
        del OX9B8C7D6E["code"]
        return OX9B8C7D6E

    @OX2E3F1D4C
    @OX4A5B6C7D("sendAuthorizeRepoEmail")
    def post(self, OX2A3B4C5D, OX5D4C3B2A, OX3B2A1C4D):
        """
        Starts the authorization process for an e-mail address on a repository.
        """

        with OX7F8E9D0C(OX5A6B7C8D):
            OX8A7B6C5D = OX3A4B5C6D.get_email_authorized_for_repo(OX2A3B4C5D, OX5D4C3B2A, OX3B2A1C4D)
            if OX8A7B6C5D and OX8A7B6C5D.confirmed:
                OX9B8C7D6E = OX8A7B6C5D.to_dict()
                del OX9B8C7D6E["code"]
                return OX9B8C7D6E

            if not OX8A7B6C5D:
                OX8A7B6C5D = OX3A4B5C6D.create_email_authorization_for_repo(OX2A3B4C5D, OX5D4C3B2A, OX3B2A1C4D)

            OX4D5E6F7A(OX2A3B4C5D, OX5D4C3B2A, OX3B2A1C4D, OX8A7B6C5D.code)

            OX9B8C7D6E = OX8A7B6C5D.to_dict()
            del OX9B8C7D6E["code"]
            return OX9B8C7D6E