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
        lat = float(request.GET.get('lat',''))
        lng = float(request.GET.get('lng',''))
        qs = SearchQuerySet().filter(content_auto=request.GET.get('q',"")).distance('geometry',Point(lng,lat,srid=4326)).order_by('distance')
        qs = self.limit_queryset(qs, 5)
        json = [(q.content_auto," "+("%.2f" % (q.distance.m if q.distance.m<1000 else q.distance.mi))+(" meters" if q.distance.m<1000 else " miles"),q.source,q.target,GEOSGeometry(q.geometry).coords[1], GEOSGeometry(q.geometry).coords[0]) for q in qs]
        return HttpResponse(dumps(json),content_type="application/json")
    
    def limit_queryset(self, qs, limit):
        def limit_recursion(qs, index, limit):
            if index >= len(qs) or index >= limit:
                return []
            return [qs[index]] + limit_recursion(qs, index + 1, limit)
        return limit_recursion(qs, 0, limit)


class GeoJsonAjax(View):
    def get(self,request, *args, **kwargs):
        lat = float(request.GET.get('lat1','45'))
        lng = float(request.GET.get('lng1','-93.265'))
        qs = BestBikeTrails.objects.filter(the_geom__distance_lte=(Point(lng,lat,srid=4326),D(mi=2)))
        gj = self.build_geojson(qs)
        return HttpResponse(dumps(gj),content_type="application/json")
    
    def build_geojson(self, qs):
        def geojson_recursion(qs, index):
            if index >= len(qs):
                return []
            poly = GEOSGeometry(qs[index].the_geom,srid=4326)
            return [loads(poly.geojson)] + geojson_recursion(qs, index + 1)
        return geojson_recursion(qs, 0)


class RouterAjax(View):
    def get(self, request, *args, **kwargs):
        id1 =  request.GET.get('bid')
        id2 = request.GET.get('eid')
        sql_inside_of_function = "select id, source, target, cost * (4-rtng_ccpx) * (4-rtng_mean) * (4-rtng_cbf7) as cost,cost * (4-rtng_ccpx)*(4-rtng_mean)*(4-rtng_cbf7) * case when one_way=0 then 1 else one_way END as reverse_cost from \"Data_minnesotabiketrails\"\'"
        sql_function = "select ccp_name, the_geom from pgr_dijkstra(\'"

        cursor = connection.cursor()
        cursor.execute(sql_function+sql_inside_of_function+", %s , %s , true,true) join \"Data_minnesotabiketrails\" as bt on bt.id=id2",(str(id1),str(id2),))
        all = cursor.fetchall()
        names, gj = self.process_routes(all)
        return HttpResponse(dumps({'names':names,'geojson':gj}),content_type="application/json; charset='utf-8'")
    
    def process_routes(self, all):
        def process_recursion(all, index):
            if index >= len(all):
                return [], []
            names, gj = process_recursion(all, index + 1)
            return [all[index][0]] + names, [loads(GEOSGeometry(all[index][1]).geojson)] + gj
        return process_recursion(all, 0)


class NiceRideAjax(View):
    def get(self, request, *args, **kwargs):
        r = get(url="https://secure.niceridemn.org/data2/bikeStations.xml")
        doc = ET.fromstring(r.text)
        stations = doc.findall('station')
        json = [{item.tag: item.text for item in station} for station in stations]
        gj = self.filter_and_format_stations(json)
        return HttpResponse(dumps(gj), content_type="application/json; charset='utf-8'")
    
    def filter_and_format_stations(self, json):
        def filter_recursion(json, index):
            if index >= len(json):
                return []
            d = json[index]
            if d['public'] == 'true':
                lat = d['lat']
                long = d['long']
                del d['lat']
                del d['long']
                return [{'type': 'Point', 'coordinates': [long, lat], 'properties': d}] + filter_recursion(json, index + 1)
            return filter_recursion(json, index + 1)
        return filter_recursion(json, 0)