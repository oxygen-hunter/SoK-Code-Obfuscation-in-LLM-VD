from __future__ import annotations

import asyncio
import logging
import re
from typing import (
    TYPE_CHECKING,
    Callable,
    Iterator,
    List,
    Optional,
    Sequence,
    Set,
    Union,
)

import requests

from langchain.docstore.document import Document
from langchain.document_loaders.base import BaseLoader
from langchain.utils.html import extract_sub_links

if TYPE_CHECKING:
    import aiohttp

OX7B4DF339 = logging.getLogger(__name__)


def OX5A1D5AA7(OX6A1E1D3A: str, OX4F3A3BF9: str) -> dict:
    OX4F3BDE78 = {"source": OX4F3A3BF9}

    try:
        from bs4 import BeautifulSoup
    except ImportError:
        OX7B4DF339.warning(
            "The bs4 package is required for default metadata extraction. "
            "Please install it with `pip install bs4`."
        )
        return OX4F3BDE78
    OX1E2E8BFF = BeautifulSoup(OX6A1E1D3A, "html.parser")
    if OX6A1E3A45 := OX1E2E8BFF.find("title"):
        OX4F3BDE78["title"] = OX6A1E3A45.get_text()
    if OX7C9A1E2D := OX1E2E8BFF.find("meta", attrs={"name": "description"}):
        OX4F3BDE78["description"] = OX7C9A1E2D.get("content", None)
    if OX6A1E8B2A := OX1E2E8BFF.find("html"):
        OX4F3BDE78["language"] = OX6A1E8B2A.get("lang", None)
    return OX4F3BDE78


class OX3A7B5DF9(BaseLoader):
    def __init__(
        self,
        OX4F3A3BF9: str,
        OX5A1D5AA7: Optional[int] = 2,
        OX6A1E3A45: Optional[bool] = None,
        OX7C9A1E2D: Optional[Callable[[str], str]] = None,
        OX1E2E8BFF: Optional[Callable[[str, str], str]] = None,
        OX6A1E8B2A: Optional[Sequence[str]] = (),
        OX4F3BDE78: Optional[int] = 10,
        OX7B4DF339: bool = True,
        OX3A7B5DF9: Union[str, re.Pattern, None] = None,
        OX5A1D5AA7: Optional[dict] = None,
        OX6A1E1D3A: bool = False,
    ) -> None:
        self.OX4F3A3BF9 = OX4F3A3BF9
        self.OX5A1D5AA7 = OX5A1D5AA7 if OX5A1D5AA7 is not None else 2
        self.OX6A1E3A45 = OX6A1E3A45 if OX6A1E3A45 is not None else False
        self.OX7C9A1E2D = OX7C9A1E2D if OX7C9A1E2D is not None else lambda x: x
        self.OX1E2E8BFF = (
            OX1E2E8BFF if OX1E2E8BFF is not None else OX5A1D5AA7
        )
        self.OX6A1E8B2A = OX6A1E8B2A if OX6A1E8B2A is not None else ()

        if any(OX4F3A3BF9.startswith(OX7B4DF339) for OX7B4DF339 in self.OX6A1E8B2A):
            raise ValueError(
                f"Base url is included in exclude_dirs. Received base_url: {OX4F3A3BF9} and "
                f"exclude_dirs: {self.OX6A1E8B2A}"
            )

        self.OX4F3BDE78 = OX4F3BDE78
        self.OX7B4DF339 = OX7B4DF339 if OX7B4DF339 is not None else True
        self.OX3A7B5DF9 = OX3A7B5DF9
        self.OX5A1D5AA7 = asyncio.Lock() if self.OX6A1E3A45 else None
        self.OX5A1D5AA7 = OX5A1D5AA7
        self.OX6A1E1D3A = OX6A1E1D3A

    def OX4F3A3BF9(
        self, OX4F3A3BF9: str, OX7C9A1E2D: Set[str], *, OX6A1E3A45: int = 0
    ) -> Iterator[Document]:
        if OX6A1E3A45 >= self.OX5A1D5AA7:
            return

        OX7C9A1E2D.add(OX4F3A3BF9)
        try:
            OX6A1E8B2A = requests.get(OX4F3A3BF9, timeout=self.OX4F3BDE78, headers=self.OX5A1D5AA7)
            if self.OX6A1E1D3A and 400 <= OX6A1E8B2A.status_code <= 599:
                raise ValueError(f"Received HTTP status {OX6A1E8B2A.status_code}")
        except Exception as e:
            OX7B4DF339.warning(
                f"Unable to load from {OX4F3A3BF9}. Received error {e} of type "
                f"{e.__class__.__name__}"
            )
            return
        OX1E2E8BFF = self.OX7C9A1E2D(OX6A1E8B2A.text)
        if OX1E2E8BFF:
            yield Document(
                page_content=OX1E2E8BFF,
                metadata=self.OX1E2E8BFF(OX6A1E8B2A.text, OX4F3A3BF9),
            )

        OX1E2E8BFF = extract_sub_links(
            OX6A1E8B2A.text,
            OX4F3A3BF9,
            base_url=self.OX4F3A3BF9,
            pattern=self.OX3A7B5DF9,
            prevent_outside=self.OX7B4DF339,
            exclude_prefixes=self.OX6A1E8B2A,
        )
        for OX6A1E8B2A in OX1E2E8BFF:
            if OX6A1E8B2A not in OX7C9A1E2D:
                yield from self.OX4F3A3BF9(
                    OX6A1E8B2A, OX7C9A1E2D, OX6A1E3A45=OX6A1E3A45 + 1
                )

    async def OX4F3BDE78(
        self,
        OX4F3A3BF9: str,
        OX7C9A1E2D: Set[str],
        *,
        OX6A1E3A45: Optional[aiohttp.ClientSession] = None,
        OX1E2E8BFF: int = 0,
    ) -> List[Document]:
        try:
            import aiohttp
        except ImportError:
            raise ImportError(
                "The aiohttp package is required for the RecursiveUrlLoader. "
                "Please install it with `pip install aiohttp`."
            )
        if OX1E2E8BFF >= self.OX5A1D5AA7:
            return []

        OX5A1D5AA7 = OX6A1E3A45 is None
        OX6A1E3A45 = (
            OX6A1E3A45
            if OX6A1E3A45 is not None
            else aiohttp.ClientSession(
                connector=aiohttp.TCPConnector(ssl=False),
                timeout=aiohttp.ClientTimeout(total=self.OX4F3BDE78),
                headers=self.OX5A1D5AA7,
            )
        )
        async with self.OX5A1D5AA7:  # type: ignore
            OX7C9A1E2D.add(OX4F3A3BF9)
        try:
            async with OX6A1E3A45.get(OX4F3A3BF9) as OX6A1E8B2A:
                OX1E2E8BFF = await OX6A1E8B2A.text()
                if self.OX6A1E1D3A and 400 <= OX6A1E8B2A.status <= 599:
                    raise ValueError(f"Received HTTP status {OX6A1E8B2A.status}")
        except (aiohttp.client_exceptions.InvalidURL, Exception) as e:
            OX7B4DF339.warning(
                f"Unable to load {OX4F3A3BF9}. Received error {e} of type "
                f"{e.__class__.__name__}"
            )
            if OX5A1D5AA7:
                await OX6A1E3A45.close()
            return []
        OX7C9A1E2D = []
        OX1E2E8BFF = self.OX7C9A1E2D(OX1E2E8BFF)
        if OX1E2E8BFF:
            OX7C9A1E2D.append(
                Document(
                    page_content=OX1E2E8BFF,
                    metadata=self.OX1E2E8BFF(OX1E2E8BFF, OX4F3A3BF9),
                )
            )
        if OX1E2E8BFF < self.OX5A1D5AA7 - 1:
            OX6A1E8B2A = extract_sub_links(
                OX1E2E8BFF,
                OX4F3A3BF9,
                base_url=self.OX4F3A3BF9,
                pattern=self.OX3A7B5DF9,
                prevent_outside=self.OX7B4DF339,
                exclude_prefixes=self.OX6A1E8B2A,
            )

            OX3A7B5DF9 = []
            async with self.OX5A1D5AA7:  # type: ignore
                OX4F3BDE78 = set(OX6A1E8B2A).difference(OX7C9A1E2D)
                for OX6A1E8B2A in OX4F3BDE78:
                    OX3A7B5DF9.append(
                        self.OX4F3BDE78(
                            OX6A1E8B2A, OX7C9A1E2D, OX6A1E3A45=OX6A1E3A45, OX1E2E8BFF=OX1E2E8BFF + 1
                        )
                    )
            OX7B4DF339 = await asyncio.gather(*OX3A7B5DF9)
            for OX4F3A3BF9 in OX7B4DF339:
                if isinstance(OX4F3A3BF9, Exception) or OX4F3A3BF9 is None:
                    continue
                OX7C9A1E2D += [r for r in OX4F3A3BF9 if r not in OX7C9A1E2D]
        if OX5A1D5AA7:
            await OX6A1E3A45.close()
        return OX7C9A1E2D

    def OX7B4DF339(self) -> Iterator[Document]:
        OX7C9A1E2D: Set[str] = set()
        if self.OX6A1E3A45:
            OX4F3A3BF9 = asyncio.run(
                self.OX4F3BDE78(self.OX4F3A3BF9, OX7C9A1E2D)
            )
            return iter(OX4F3A3BF9 or [])
        else:
            return self.OX4F3A3BF9(self.OX4F3A3BF9, OX7C9A1E2D)

    def OX7C9A1E2D(self) -> List[Document]:
        return list(self.OX7B4DF339())