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
        coords = [float(request.GET.get('lat','')), float(request.GET.get('lng',''))]
        s_q_s = SearchQuerySet().filter(content_auto=request.GET.get('q',"")).distance('geometry',Point(coords[1],coords[0],srid=4326)).order_by('distance')
        if len(s_q_s) > 6:
            s_q_s = s_q_s[:5]
        json = [(q.content_auto," "+("%.2f" % (q.distance.m if q.distance.m<1000 else q.distance.mi))+(" meters" if q.distance.m<1000 else " miles"),q.source,q.target,GEOSGeometry(q.geometry).coords[1], GEOSGeometry(q.geometry).coords[0]) for q in s_q_s]
        return HttpResponse(dumps(json),content_type="application/json")


class GeoJsonAjax(View):
    def get(self,request, *args, **kwargs):
        coords = [float(request.GET.get('lat1','45')), float(request.GET.get('lng1','-93.265'))]
        qs = BestBikeTrails.objects.filter(the_geom__distance_lte=(Point(coords[1],coords[0],srid=4326),D(mi=2)))
        gj = []
        for item in qs:
            poly = GEOSGeometry(item.the_geom,srid=4326)
            gj.append(loads(poly.geojson))
        return HttpResponse(dumps(gj),content_type="application/json")


class RouterAjax(View):
    def get(self, request, *args, **kwargs):
        ids = [request.GET.get('bid'), request.GET.get('eid')]
        sql_inside_of_function = "select id, source, target, cost * (4-rtng_ccpx) * (4-rtng_mean) * (4-rtng_cbf7) as cost,cost * (4-rtng_ccpx)*(4-rtng_mean)*(4-rtng_cbf7) * case when one_way=0 then 1 else one_way END as reverse_cost from \"Data_minnesotabiketrails\"\'"
        sql_function = "select ccp_name, the_geom from pgr_dijkstra(\'"

        cursor = connection.cursor()
        cursor.execute(sql_function+sql_inside_of_function+", %s , %s , true,true) join \"Data_minnesotabiketrails\" as bt on bt.id=id2",(str(ids[0]),str(ids[1]),))
        all_data = cursor.fetchall()
        names = []
        geojson = []
        for item in all_data:
            names.append(item[0])
            geojson.append(loads(GEOSGeometry(item[1]).geojson))
        return HttpResponse(dumps({'names':names,'geojson':geojson}),content_type="application/json; charset='utf-8'")


class NiceRideAjax(View):
    def get(self, request, *args, **kwargs):
        response = get(url="https://secure.niceridemn.org/data2/bikeStations.xml")
        doc = ET.fromstring(response.text)
        stations = doc.findall('station')
        json_data = [{item.tag: item.text for item in station} for station in stations]
        geojson = []
        for d in json_data:
            loc = [d['lat'], d['long']]
            del d['lat']
            del d['long']
            geojson.append({'type': 'Point', 'coordinates': [loc[1], loc[0]], 'properties': d})
        return HttpResponse(dumps(geojson), content_type="application/json; charset='utf-8'")