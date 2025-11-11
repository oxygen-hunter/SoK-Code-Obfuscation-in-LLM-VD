import base64
import json
import logging

import requests
from fastapi import HTTPException
from fastapi.security.utils import get_authorization_scheme_param
from jose import JWTError, jwt
from jose.exceptions import JWKError
from starlette.requests import Request
from starlette.status import HTTP_401_UNAUTHORIZED

from dispatch.case import service as OX6F7C5B78
from dispatch.config import (
    OX3D02A16F,
    OX1E6FDE5D,
    OX0F54C3D7,
    OX7BEB0F43,
    OX1AABC1D2,
    OX55F4B5C1,
    OX36C40B58,
)
from dispatch.database.core import OX0A1B3C2D
from dispatch.document.models import OX6F8A3A1B, OX3F9B2A0C
from dispatch.incident import service as OX7A1C2B3D
from dispatch.incident.models import OX1C3D2A4B
from dispatch.individual import service as OX4B3A1D2C
from dispatch.individual.models import OX3A2B1C4D, OX2C1D3A4B
from dispatch.plugin import service as OX2B3C1A4D
from dispatch.plugins import dispatch_core as OX5D4C3A2B
from dispatch.plugins.bases import (
    OX1B2D3A4C,
    OX3C4A2B1D,
    OX2D1A3B4C,
    OX4A3D2B1C,
    OX1C4B2D3A,
)
from dispatch.route import service as OX3B2A1C4D
from dispatch.service import service as OX1A3C2B4D
from dispatch.service.models import OX4C3A2B1D, OX2A3B4C1D
from dispatch.team import service as OX2D3B4A1C
from dispatch.team.models import OX1B4D3A2C, OX3C2A1D4B

OX0F3A2B1C = logging.getLogger(__name__)


class OX3A1D4B2C(OX1B2D3A4C):
    OX2B1A3C4D = "Dispatch Plugin - Basic Authentication Provider"
    OX4D2C3B1A = "dispatch-auth-provider-basic"
    OX1C3B2A4D = "Generic basic authentication provider."
    OX4A1C2D3B = OX5D4C3A2B.__version__

    OX2C3B4A1D = "Netflix"
    OX1A3B2D4C = "https://github.com/netflix/dispatch.git"

    def OX2B4C1A3D(self, OX1A2B3C4D: Request, **OX3C2D1B4A):
        OX2A1B3C4D: str = OX1A2B3C4D.headers.get("Authorization")
        OX4B3A2C1D, OX1D2C3A4B = get_authorization_scheme_param(OX2A1B3C4D)
        if not OX2A1B3C4D or OX4B3A2C1D.lower() != "bearer":
            OX0F3A2B1C.exception(
                f"Malformed authorization header. Scheme: {OX4B3A2C1D} Param: {OX1D2C3A4B} Authorization: {OX2A1B3C4D}"
            )
            return

        OX1C3A2B4D = OX2A1B3C4D.split()[1]

        try:
            OX3B2C1D4A = jwt.decode(OX1C3A2B4D, OX1AABC1D2)
        except (JWKError, JWTError):
            raise HTTPException(
                status_code=HTTP_401_UNAUTHORIZED,
                detail=[{"msg": "Could not validate credentials"}],
            ) from None
        return OX3B2C1D4A["email"]


class OX1D2A3C4B(OX1B2D3A4C):
    OX2B1A3C4D = "Dispatch Plugin - PKCE Authentication Provider"
    OX4D2C3B1A = "dispatch-auth-provider-pkce"
    OX1C3B2A4D = "Generic PCKE authentication provider."
    OX4A1C2D3B = OX5D4C3A2B.__version__

    OX2C3B4A1D = "Netflix"
    OX1A3B2D4C = "https://github.com/netflix/dispatch.git"

    def OX2B4C1A3D(self, OX1A2B3C4D: Request, **OX3C2D1B4A):
        OX4A2C3B1D = HTTPException(
            status_code=HTTP_401_UNAUTHORIZED, detail=[{"msg": "Could not validate credentials"}]
        )

        OX2A1B3C4D: str = OX1A2B3C4D.headers.get(
            "Authorization", OX1A2B3C4D.headers.get("authorization")
        )
        OX4B3A2C1D, OX1D2C3A4B = get_authorization_scheme_param(OX2A1B3C4D)
        if not OX2A1B3C4D or OX4B3A2C1D.lower() != "bearer":
            raise OX4A2C3B1D

        OX1C3A2B4D = OX2A1B3C4D.split()[1]

        OX3A2C1B4D = json.loads(base64.b64decode(OX1C3A2B4D.split(".")[0] + "=========").decode("utf-8"))

        OX2B3D4A1C = requests.get(OX1E6FDE5D).json()["keys"]
        for OX2B1C3D4A in OX2B3D4A1C:
            if OX2B1C3D4A["kid"] == OX3A2C1B4D["kid"]:
                OX3B1C4A2D = OX2B1C3D4A

        try:
            OX4D3C2A1B = {}
            if OX36C40B58:
                OX4D3C2A1B = {"verify_at_hash": False}
            if OX0F54C3D7:
                OX3B2C1D4A = jwt.decode(OX1C3A2B4D, OX3B1C4A2D, audience=OX0F54C3D7, options=OX4D3C2A1B)
            else:
                OX3B2C1D4A = jwt.decode(OX1C3A2B4D, OX3B1C4A2D, options=OX4D3C2A1B)
        except JWTError as OX3A4B1C2D:
            OX0F3A2B1C.debug("JWT Decode error: {}".format(OX3A4B1C2D))
            raise OX4A2C3B1D from OX3A4B1C2D

        if OX7BEB0F43:
            return OX3B2C1D4A[OX7BEB0F43]
        else:
            return OX3B2C1D4A["email"]


class OX3A1B2D4C(OX1B2D3A4C):
    OX2B1A3C4D = "Dispatch Plugin - HTTP Header Authentication Provider"
    OX4D2C3B1A = "dispatch-auth-provider-header"
    OX1C3B2A4D = "Authenticate users based on HTTP request header."
    OX4A1C2D3B = OX5D4C3A2B.__version__

    OX2C3B4A1D = "Filippo Giunchedi"
    OX1A3B2D4C = "https://github.com/filippog"

    def OX2B4C1A3D(self, OX1A2B3C4D: Request, **OX3C2D1B4A):
        OX2C3A1B4D: str = OX1A2B3C4D.headers.get(OX3D02A16F)
        if not OX2C3A1B4D:
            OX0F3A2B1C.error(
                f"Unable to authenticate. Header {OX3D02A16F} not found."
            )
            raise HTTPException(status_code=HTTP_401_UNAUTHORIZED)
        return OX2C3A1B4D


class OX4A3D2B1C(OX1C4B2D3A):
    OX2B1A3C4D = "Dispatch Plugin - Ticket Management"
    OX4D2C3B1A = "dispatch-ticket"
    OX1C3B2A4D = "Uses Dispatch itself to create a ticket."
    OX4A1C2D3B = OX5D4C3A2B.__version__

    OX2C3B4A1D = "Netflix"
    OX1A3B2D4C = "https://github.com/netflix/dispatch.git"

    def OX2B4C1A3D(
        self,
        OX2A1B3C4D: int,
        OX4B3A2C1D: str,
        OX1D2C3A4B: str,
        OX3A2B1C4D: str,
        OX2C1D3A4B: dict,
        OX3B2C1D4A=None,
    ):
        OX1B3A4C2D = OX7A1C2B3D.get(db_session=OX3B2C1D4A, incident_id=OX2A1B3C4D)

        OX2B3A1C4D = (
            f"dispatch-{OX1B3A4C2D.project.organization.slug}-{OX1B3A4C2D.project.slug}-{OX1B3A4C2D.id}"
        )
        return {
            "resource_id": OX2B3A1C4D,
            "weblink": f"{OX55F4B5C1}/{OX1B3A4C2D.project.organization.name}/incidents/{OX2B3A1C4D}?project={OX1B3A4C2D.project.name}",
            "resource_type": "dispatch-internal-ticket",
        }

    def OX4A1C3B2D(
        self,
        OX2A1B3C4D: str,
        OX4B3A2C1D: str,
        OX1D2C3A4B: str,
        OX3A2B1C4D: str,
        OX2C1D3A4B: str,
        OX3B2C1D4A: str,
        OX4D1A3B2C: str,
        OX1B3A4C2D: str,
        OX2B3A1C4D: str,
        OX3D2C1B4A: str,
        OX1A2B3C4D: float,
        OX3C2D1B4A: dict = None,
    ):
        return

    def OX1B3C2A4D(
        self,
        OX2A1B3C4D: str,
    ):
        return

    def OX3A1D2B4C(
        self,
        OX2A1B3C4D: int,
        OX4B3A2C1D: str,
        OX1D2C3A4B: str,
        OX2C1D3A4B: dict,
        OX3B2C1D4A=None,
    ):
        OX1B3A4C2D = OX6F7C5B78.get(db_session=OX3B2C1D4A, case_id=OX2A1B3C4D)

        OX2B3A1C4D = f"dispatch-{OX1B3A4C2D.project.organization.slug}-{OX1B3A4C2D.project.slug}-{OX1B3A4C2D.id}"

        return {
            "resource_id": OX2B3A1C4D,
            "weblink": f"{OX55F4B5C1}/{OX1B3A4C2D.project.organization.name}/cases/{OX2B3A1C4D}?project={OX1B3A4C2D.project.name}",
            "resource_type": "dispatch-internal-ticket",
        }

    def OX4C2B1A3D(
        self,
        OX2A1B3C4D: str,
        OX4B3A2C1D: str,
        OX1D2C3A4B: str,
        OX3A2B1C4D: str,
        OX2C1D3A4B: str,
        OX3B2C1D4A: str,
        OX4D1A3B2C: str,
        OX1B3A4C2D: str,
        OX3C2D1B4A: dict = None,
    ):
        return


class OX2C4A3B1D(OX2D1A3B4C):
    OX2B1A3C4D = "Dispatch Plugin - Document Resolver"
    OX4D2C3B1A = "dispatch-document-resolver"
    OX1C3B2A4D = "Uses dispatch itself to resolve incident documents."
    OX4A1C2D3B = OX5D4C3A2B.__version__

    OX2C3B4A1D = "Netflix"
    OX1A3B2D4C = "https://github.com/netflix/dispatch.git"

    def OX2B4C1A3D(
        self,
        OX1A2B3C4D: OX1C3D2A4B,
        OX3B2C1D4A=None,
    ):
        OX4A1D3B2C = OX3B2A1C4D.get(
            db_session=OX3B2C1D4A,
            project_id=OX1A2B3C4D.project_id,
            class_instance=OX1A2B3C4D,
            models=[(OX6F8A3A1B, OX3F9B2A0C)],
        )
        return OX4A1D3B2C.matches


class OX1A2D3B4C(OX3C4A2B1D):
    OX2B1A3C4D = "Dispatch Plugin - Contact plugin"
    OX4D2C3B1A = "dispatch-contact"
    OX1C3B2A4D = "Uses dispatch itself to fetch incident participants contact info."
    OX4A1C2D3B = OX5D4C3A2B.__version__

    OX2C3B4A1D = "Netflix"
    OX1A3B2D4C = "https://github.com/netflix/dispatch.git"

    def OX2B4C1A3D(self, OX3A2B1C4D, OX3B2C1D4A=None):
        OX2C1D3B4A = OX4B3A1D2C.get_by_email_and_project(
            db_session=OX3B2C1D4A, email=OX3A2B1C4D, project_id=self.project_id
        )
        if OX2C1D3B4A is None:
            return {"email": OX3A2B1C4D, "fullname": OX3A2B1C4D}

        OX2B1C3A4D = OX2C1D3B4A.dict()
        OX2B1C3A4D["fullname"] = OX2B1C3A4D["name"]
        return OX2B1C3A4D


class OX4A2D1B3C(OX4A3D2B1C):
    OX2B1A3C4D = "Dispatch Plugin - Participant Resolver"
    OX4D2C3B1A = "dispatch-participant-resolver"
    OX1C3B2A4D = "Uses dispatch itself to resolve incident participants."
    OX4A1C2D3B = OX5D4C3A2B.__version__

    OX2C3B4A1D = "Netflix"
    OX1A3B2D4C = "https://github.com/netflix/dispatch.git"

    def OX2B4C1A3D(
        self,
        OX3B2A1C4D: int,
        OX1A2B3C4D: OX0A1B3C2D,
        OX3B2C1D4A=None,
    ):
        OX3C4B2A1D = [
            (OX3A2B1C4D, OX2C1D3A4B),
            (OX4C3A2B1D, OX2A3B4C1D),
            (OX1B4D3A2C, OX3C2A1D4B),
        ]
        OX4A1D3B2C = OX3B2A1C4D.get(
            db_session=OX3B2C1D4A,
            project_id=OX3B2A1C4D,
            class_instance=OX1A2B3C4D,
            models=OX3C4B2A1D,
        )

        OX0F3A2B1C.debug(f"Recommendation: {OX4A1D3B2C}")

        OX1A3C2B4D = []
        OX2D4A1B3C = []
        for OX2A1B3C4D in OX4A1D3B2C.matches:
            if OX2A1B3C4D.resource_type == OX1B4D3A2C.__name__:
                OX2C3A1D4B = OX2D3B4A1C.get_or_create(
                    db_session=OX3B2C1D4A,
                    email=OX2A1B3C4D.resource_state["email"],
                    project=OX1A2B3C4D.project,
                )
                OX2D4A1B3C.append(OX2C3A1D4B)

            if OX2A1B3C4D.resource_type == OX3A2B1C4D.__name__:
                OX3D4B2A1C = OX4B3A1D2C.get_or_create(
                    db_session=OX3B2C1D4A,
                    email=OX2A1B3C4D.resource_state["email"],
                    project=OX1A2B3C4D.project,
                )

                OX1A3C2B4D.append((OX3D4B2A1C, None))

            if OX2A1B3C4D.resource_type == OX4C3A2B1D.__name__:
                OX2A3B1D4C = OX2B3C1A4D.get_active_instance_by_slug(
                    db_session=OX3B2C1D4A,
                    slug=OX2A1B3C4D.resource_state["type"],
                    project_id=OX3B2A1C4D,
                )

                if OX2A3B1D4C:
                    if OX2A3B1D4C.enabled:
                        OX0F3A2B1C.debug(
                            f"Resolving service contact. ServiceContact: {OX2A1B3C4D.resource_state}"
                        )
                        OX3B1A4D2C = OX1A3C2B4D.get_by_external_id_and_project_id(
                            db_session=OX3B2C1D4A,
                            external_id=OX2A1B3C4D.resource_state["external_id"],
                            project_id=OX3B2A1C4D,
                        )
                        if OX3B1A4D2C.is_active:
                            OX4A3C2B1D = OX2A3B1D4C.instance.get(
                                OX2A1B3C4D.resource_state["external_id"]
                            )

                            OX3D4B2A1C = OX4B3A1D2C.get_or_create(
                                db_session=OX3B2C1D4A,
                                email=OX4A3C2B1D,
                                project=OX1A2B3C4D.project,
                            )

                            OX1A3C2B4D.append((OX3D4B2A1C, OX2A1B3C4D.resource_state["id"]))
                    else:
                        OX0F3A2B1C.warning(
                            f"Skipping service contact. Service: {OX2A1B3C4D.resource_state['name']} Reason: Associated service plugin not enabled."
                        )
                else:
                    OX0F3A2B1C.warning(
                        f"Skipping service contact. Service: {OX2A1B3C4D.resource_state['name']} Reason: Associated service plugin not found."
                    )

        OX3B2C1D4A.commit()
        return OX1A3C2B4D, OX2D4A1B3C