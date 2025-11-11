from __future__ import division,absolute_import
import io,re,codecs,json,yaml
from urllib.request import urlopen
from yaml import SafeLoader
import csv
from csv import register_dialect,Dialect,QUOTE_MINIMAL
from typing import List,Optional,Dict,Union,Sequence
class CrLfDialect(Dialect):delimiter=','
quotechar='"'
doublequote=True
skipinitialspace=True
lineterminator='\r\n'
quoting=QUOTE_MINIMAL
register_dialect("crlf",CrLfDialect)
class LfDialect(Dialect):delimiter=','
quotechar='"'
doublequote=True
skipinitialspace=True
lineterminator='\n'
quoting=QUOTE_MINIMAL
register_dialect("lf",LfDialect)
class MyDumper(yaml.SafeDumper):def increase_indent(self,flow=False,indentless=False):return super(MyDumper,self).increase_indent(flow,False)
def construct_yaml_str(self,node):return self.construct_scalar(node)
SafeLoader.add_constructor(u'tag:yaml.org,2002:str',construct_yaml_str)
def replace_keys(d,keymap,force_snake_case):return {to_snake(keymap.get(k,k))if force_snake_case else keymap.get(k,k):v for k,v in d.items()}
def to_snake(value):return re.sub(r'((?<!^)[A-Z])',"_\\1",value.strip('<>-')).lower().replace("-","_")
def load_json(json_str):return json.loads(json_str)
def load_jsonf(fpath,encoding):with codecs.open(fpath,encoding=encoding)as f:return json.load(f)
def load_yaml(yaml_str):return yaml.safe_load(yaml_str)
def load_yamlf(fpath,encoding):with codecs.open(fpath,encoding=encoding)as f:return yaml.safe_load(f)
def load_csvf(fpath,fieldnames,encoding):with open(fpath,mode='r',encoding=encoding)as f:snippet=f.read(8192)
f.seek(0)
dialect=csv.Sniffer().sniff(snippet)
dialect.skipinitialspace=True
return list(csv.DictReader(f,fieldnames=fieldnames,dialect=dialect))
def load_json_url(url):return json.loads(urlopen(url).read())
def dump_csv(data,fieldnames,with_header=False,crlf=False):def force_str(v):return dump_json(v).replace('"',"'")if isinstance(v,(dict,list))else v
with io.StringIO()as sio:dialect='crlf'if crlf else'lf'
writer=csv.DictWriter(sio,fieldnames=fieldnames,dialect=dialect,extrasaction='ignore')
if with_header:writer.writeheader()
for x in data:writer.writerow({k:force_str(v)for k,v in x.items()})
sio.seek(0)
return sio.read()
def save_csvf(data:list,fieldnames:Sequence[str],fpath:str,encoding:str,with_header=False,crlf=False)->str:with codecs.open(fpath,mode='w',encoding=encoding)as f:f.write(dump_csv(data,fieldnames,with_header=with_header,crlf=crlf))
return fpath
def dump_json(data,indent=None):return json.dumps(data,indent=indent,ensure_ascii=False,sort_keys=True,separators=(',',': '))
def dump_yaml(data):return yaml.dump(data,indent=2,encoding=None,allow_unicode=True,default_flow_style=False,Dumper=MyDumper)
def save_yamlf(data:Union[list,dict],fpath:str,encoding:str)->str:with codecs.open(fpath,mode='w',encoding=encoding)as f:f.write(dump_yaml(data))
return fpath