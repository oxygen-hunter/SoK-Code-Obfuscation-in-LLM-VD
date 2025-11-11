from django.shortcuts import render
from django.views.generic import TemplateView, View
from haystack.query import SearchQuerySet
from haystack.utils.geo import Point
from django.http import HttpResponse
from django.contrib.gis.measure import D
from django.contrib.gis.geos import GEOSGeometry
from django.db import connection

from Data.models import BestBikeTrails, MinnesotaBikeTrails

from requests import get
import xml.etree.ElementTree as ET
from json import dumps, loads

class MainPage(TemplateView):
    def get(self, request, *args, **kwargs):
        return render(request, 'index.html')

class SearchAjax(TemplateView):
    def get(self, request, *args, **kwargs):
        def getLat():
            return float(request.GET.get('lat',''))
        
        def getLng():
            return float(request.GET.get('lng',''))
        
        def getQuery():
            return request.GET.get('q',"")
        
        def getJsonResult(qs):
            return [(q.content_auto," "+("%.2f" % (q.distance.m if q.distance.m<1000 else q.distance.mi))+(" meters" if q.distance.m<1000 else " miles"),q.source,q.target,GEOSGeometry(q.geometry).coords[1], GEOSGeometry(q.geometry).coords[0]) for q in qs]
        
        lat = getLat()
        lng = getLng()
        qs = SearchQuerySet().filter(content_auto=getQuery()).distance('geometry',Point(lng,lat,srid=4326)).order_by('distance')
        if len(qs)>6:
            qs = qs[:5]
        json = getJsonResult(qs)
        return HttpResponse(dumps(json),content_type="application/json")

class GeoJsonAjax(View):
    def get(self,request, *args, **kwargs):
        def getLat1():
            return float(request.GET.get('lat1','45'))
        
        def getLng1():
            return float(request.GET.get('lng1','-93.265'))
        
        lat = getLat1()
        lng = getLng1()
        qs = BestBikeTrails.objects.filter(the_geom__distance_lte=(Point(lng,lat,srid=4326),D(mi=2)))
        gj = []
        for item in qs:
            poly = GEOSGeometry(item.the_geom,srid=4326)
            gj.append(loads(poly.geojson))
        return HttpResponse(dumps(gj),content_type="application/json")

class RouterAjax(View):
    def get(self, request, *args, **kwargs):
        def getBid():
            return request.GET.get('bid')
        
        def getEid():
            return request.GET.get('eid')
        
        id1 = getBid()
        id2 = getEid()
        sql_inside_of_function = "select id, source, target, cost * (4-rtng_ccpx) * (4-rtng_mean) * (4-rtng_cbf7) as cost,cost * (4-rtng_ccpx)*(4-rtng_mean)*(4-rtng_cbf7) * case when one_way=0 then 1 else one_way END as reverse_cost from \"Data_minnesotabiketrails\"\'"
        sql_function = "select ccp_name, the_geom from pgr_dijkstra(\'"

        cursor = connection.cursor()
        cursor.execute(sql_function+sql_inside_of_function+", %s , %s , true,true) join \"Data_minnesotabiketrails\" as bt on bt.id=id2",(str(id1),str(id2),))
        all = cursor.fetchall()
        names = []
        gj = []
        for item in all:
            names.append(item[0])
            gj.append(loads(GEOSGeometry(item[1]).geojson))
        return HttpResponse(dumps({'names':names,'geojson':gj}),content_type="application/json; charset='utf-8'")

class NiceRideAjax(View):
    def get(self, request, *args, **kwargs):
        r = get(url="https://secure.niceridemn.org/data2/bikeStations.xml")
        doc = ET.fromstring(r.text)
        stations = doc.findall('station')
        json = [{item.tag: item.text for item in station} for station in stations]
        gj = []
        for d in json:
            if d['public']=='true':
                lat = d['lat']
                long = d['long']
                del d['lat']
                del d['long']
                gj.append({'type': 'Point', 'coordinates': [long, lat], 'properties': d})
        return HttpResponse(dumps(gj), content_type="application/json; charset='utf-8'")