from __future__ import annotations
import asyncio,logging,re
from typing import(TYPE_CHECKING,Callable,Iterator,List,Optional,Sequence,Set,Union)
import requests
from langchain.docstore.document import Document
from langchain.document_loaders.base import BaseLoader
from langchain.utils.html import extract_sub_links
if TYPE_CHECKING:import aiohttp
logger=logging.getLogger(__name__)
def _metadata_extractor(raw_html:str,url:str)->dict:metadata={"source":url};try:from bs4 import BeautifulSoup
    except ImportError:logger.warning("The bs4 package is required for default metadata extraction. Please install it with `pip install bs4`.");return metadata
    soup=BeautifulSoup(raw_html,"html.parser");title=soup.find("title");description=soup.find("meta",attrs={"name":"description"});html=soup.find("html")
    if title:metadata["title"]=title.get_text()
    if description:metadata["description"]=description.get("content",None)
    if html:metadata["language"]=html.get("lang",None)
    return metadata
class RecursiveUrlLoader(BaseLoader):
    def __init__(self,url:str,max_depth:Optional[int]=2,use_async:Optional[bool]=None,extractor:Optional[Callable[[str],str]]=None,metadata_extractor:Optional[Callable[[str,str],str]]=None,exclude_dirs:Optional[Sequence[str]]=(),timeout:Optional[int]=10,prevent_outside:bool=True,link_regex:Union[str,re.Pattern,None]=None,headers:Optional[dict]=None,check_response_status:bool=False,)->None:self.url=url;self.max_depth=max_depth if max_depth is not None else 2;self.use_async=use_async if use_async is not None else False;self.extractor=extractor if extractor is not None else lambda x:x;self.metadata_extractor=metadata_extractor if metadata_extractor is not None else _metadata_extractor;self.exclude_dirs=exclude_dirs if exclude_dirs is not None else ();self.timeout=timeout;self.prevent_outside=prevent_outside if prevent_outside is not None else True;self.link_regex=link_regex;self._lock=asyncio.Lock() if self.use_async else None;self.headers=headers;self.check_response_status=check_response_status
        if any(url.startswith(exclude_dir)for exclude_dir in self.exclude_dirs):raise ValueError(f"Base url is included in exclude_dirs. Received base_url: {url} and exclude_dirs: {self.exclude_dirs}")
    def _get_child_links_recursive(self,url:str,visited:Set[str],*,depth:int=0)->Iterator[Document]:visited.add(url)
        if depth>=self.max_depth:return
        try:response=requests.get(url,timeout=self.timeout,headers=self.headers)
            if self.check_response_status and 400<=response.status_code<=599:raise ValueError(f"Received HTTP status {response.status_code}")
        except Exception as e:logger.warning(f"Unable to load from {url}. Received error {e} of type {e.__class__.__name__}");return
        content=self.extractor(response.text)
        if content:yield Document(page_content=content,metadata=self.metadata_extractor(response.text,url))
        sub_links=extract_sub_links(response.text,url,base_url=self.url,pattern=self.link_regex,prevent_outside=self.prevent_outside,exclude_prefixes=self.exclude_dirs)
        for link in sub_links:
            if link not in visited:yield from self._get_child_links_recursive(link,visited,depth=depth+1)
    async def _async_get_child_links_recursive(self,url:str,visited:Set[str],*,session:Optional[aiohttp.ClientSession]=None,depth:int=0,)->List[Document]:
        try:import aiohttp
        except ImportError:raise ImportError("The aiohttp package is required for the RecursiveUrlLoader. Please install it with `pip install aiohttp`.")
        if depth>=self.max_depth:return[]
        close_session=session is None
        session=session if session is not None else aiohttp.ClientSession(connector=aiohttp.TCPConnector(ssl=False),timeout=aiohttp.ClientTimeout(total=self.timeout),headers=self.headers,)
        async with self._lock:visited.add(url)
        try:async with session.get(url)as response:text=await response.text()
            if self.check_response_status and 400<=response.status<=599:raise ValueError(f"Received HTTP status {response.status}")
        except(aiohttp.client_exceptions.InvalidURL,Exception)as e:logger.warning(f"Unable to load {url}. Received error {e} of type {e.__class__.__name__}")
            if close_session:await session.close()
            return[]
        results=[];content=self.extractor(text)
        if content:results.append(Document(page_content=content,metadata=self.metadata_extractor(text,url)))
        if depth<self.max_depth-1:sub_links=extract_sub_links(text,url,base_url=self.url,pattern=self.link_regex,prevent_outside=self.prevent_outside,exclude_prefixes=self.exclude_dirs)
            sub_tasks=[];async with self._lock:to_visit=set(sub_links).difference(visited)
                for link in to_visit:sub_tasks.append(self._async_get_child_links_recursive(link,visited,session=session,depth=depth+1))
            next_results=await asyncio.gather(*sub_tasks)
            for sub_result in next_results:
                if isinstance(sub_result,Exception)or sub_result is None:continue
                results+=[r for r in sub_result if r not in results]
        if close_session:await session.close()
        return results
    def lazy_load(self)->Iterator[Document]:visited:Set[str]=set()
        if self.use_async:results=asyncio.run(self._async_get_child_links_recursive(self.url,visited));return iter(results or [])
        else:return self._get_child_links_recursive(self.url,visited)
    def load(self)->List[Document]:
        return list(self.lazy_load())