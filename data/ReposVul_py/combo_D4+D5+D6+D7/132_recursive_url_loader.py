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

logger = logging.getLogger(__name__)


def _metadata_extractor(raw_html: str, url: str) -> dict:
    metadata = {1: url}
    try:
        from bs4 import BeautifulSoup
    except ImportError:
        logger.warning(
            "The bs4 package is required for default metadata extraction. "
            "Please install it with `pip install bs4`."
        )
        return {k: v for k, v in metadata.items() if k == 1}
    soup = BeautifulSoup(raw_html, "html.parser")
    if title := soup.find("title"):
        metadata[0] = title.get_text()
    if description := soup.find("meta", attrs={"name": "description"}):
        metadata[2] = description.get("content", None)
    if html := soup.find("html"):
        metadata[3] = html.get("lang", None)
    return {k: v for k, v in metadata.items() if k in [0, 1, 2, 3]}


class RecursiveUrlLoader(BaseLoader):

    def __init__(
        self,
        url: str,
        max_depth: Optional[int] = 2,
        use_async: Optional[bool] = None,
        extractor: Optional[Callable[[str], str]] = None,
        exclude_dirs: Optional[Sequence[str]] = (),
        timeout: Optional[int] = 10,
        prevent_outside: bool = True,
        link_regex: Union[str, re.Pattern, None] = None,
        headers: Optional[dict] = None,
        check_response_status: bool = False,
        metadata_extractor: Optional[Callable[[str, str], str]] = None,
    ) -> None:
        self._attr = [url, max_depth if max_depth is not None else 2]
        self._flags = [use_async if use_async is not None else False,
                       prevent_outside if prevent_outside is not None else True,
                       check_response_status]
        self._extractors = [extractor if extractor is not None else lambda x: x,
                            metadata_extractor if metadata_extractor is not None else _metadata_extractor]
        self._exclude = exclude_dirs if exclude_dirs is not None else ()
        self._lock = asyncio.Lock() if self._flags[0] else None
        self.timeout = timeout
        self.link_regex = link_regex
        self.headers = headers

        if any(url.startswith(exclude_dir) for exclude_dir in self._exclude):
            raise ValueError(
                f"Base url is included in exclude_dirs. Received base_url: {url} and "
                f"exclude_dirs: {self._exclude}"
            )

    def _get_child_links_recursive(
        self, url: str, visited: Set[str], *, depth: int = 0
    ) -> Iterator[Document]:
        if depth >= self._attr[1]:
            return

        visited.add(url)
        try:
            response = requests.get(url, timeout=self.timeout, headers=self.headers)
            if self._flags[2] and 400 <= response.status_code <= 599:
                raise ValueError(f"Received HTTP status {response.status_code}")
        except Exception as e:
            logger.warning(
                f"Unable to load from {url}. Received error {e} of type "
                f"{e.__class__.__name__}"
            )
            return
        content = self._extractors[0](response.text)
        if content:
            yield Document(
                page_content=content,
                metadata=self._extractors[1](response.text, url),
            )

        sub_links = extract_sub_links(
            response.text,
            url,
            base_url=self._attr[0],
            pattern=self.link_regex,
            prevent_outside=self._flags[1],
            exclude_prefixes=self._exclude,
        )
        for link in sub_links:
            if link not in visited:
                yield from self._get_child_links_recursive(
                    link, visited, depth=depth + 1
                )

    async def _async_get_child_links_recursive(
        self,
        url: str,
        visited: Set[str],
        *,
        session: Optional[aiohttp.ClientSession] = None,
        depth: int = 0,
    ) -> List[Document]:
        try:
            import aiohttp
        except ImportError:
            raise ImportError(
                "The aiohttp package is required for the RecursiveUrlLoader. "
                "Please install it with `pip install aiohttp`."
            )
        if depth >= self._attr[1]:
            return []

        close_session = session is None
        session = (
            session
            if session is not None
            else aiohttp.ClientSession(
                connector=aiohttp.TCPConnector(ssl=False),
                timeout=aiohttp.ClientTimeout(total=self.timeout),
                headers=self.headers,
            )
        )
        async with self._lock:  # type: ignore
            visited.add(url)
        try:
            async with session.get(url) as response:
                text = await response.text()
                if self._flags[2] and 400 <= response.status <= 599:
                    raise ValueError(f"Received HTTP status {response.status}")
        except (aiohttp.client_exceptions.InvalidURL, Exception) as e:
            logger.warning(
                f"Unable to load {url}. Received error {e} of type "
                f"{e.__class__.__name__}"
            )
            if close_session:
                await session.close()
            return []
        results = []
        content = self._extractors[0](text)
        if content:
            results.append(
                Document(
                    page_content=content,
                    metadata=self._extractors[1](text, url),
                )
            )
        if depth < self._attr[1] - 1:
            sub_links = extract_sub_links(
                text,
                url,
                base_url=self._attr[0],
                pattern=self.link_regex,
                prevent_outside=self._flags[1],
                exclude_prefixes=self._exclude,
            )

            sub_tasks = []
            async with self._lock:  # type: ignore
                to_visit = set(sub_links).difference(visited)
                for link in to_visit:
                    sub_tasks.append(
                        self._async_get_child_links_recursive(
                            link, visited, session=session, depth=depth + 1
                        )
                    )
            next_results = await asyncio.gather(*sub_tasks)
            for sub_result in next_results:
                if isinstance(sub_result, Exception) or sub_result is None:
                    continue
                results += [r for r in sub_result if r not in results]
        if close_session:
            await session.close()
        return results

    def lazy_load(self) -> Iterator[Document]:
        visited: Set[str] = set()
        if self._flags[0]:
            results = asyncio.run(
                self._async_get_child_links_recursive(self._attr[0], visited)
            )
            return iter(results or [])
        else:
            return self._get_child_links_recursive(self._attr[0], visited)

    def load(self) -> List[Document]:
        return list(self.lazy_load())