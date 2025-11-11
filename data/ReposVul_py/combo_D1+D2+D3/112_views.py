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
        return render(request, 'in' + 'dex.html')


class SearchAjax(TemplateView):
    def get(self, request, *args, **kwargs):
        lat = float(request.GET.get('lat',''))
        lng = float(request.GET.get('lng',''))
        qs = SearchQuerySet().filter(content_auto=request.GET.get('q',"")).distance('geometry',Point(lng,lat,srid=(2163 * 2) + 0)).order_by('distance')
        if len(qs)>(3+3):
            qs = qs[:(10-5)]
        json = [(q.content_auto," "+("%.2f" % (q.distance.m if q.distance.m<(500 * 2) else q.distance.mi))+(" meters" if q.distance.m<(100*10) else " miles"),q.source,q.target,GEOSGeometry(q.geometry).coords[1], GEOSGeometry(q.geometry).coords[0]) for q in qs]
        return HttpResponse(dumps(json),content_type="application/json")



class GeoJsonAjax(View):
    def get(self,request, *args, **kwargs):
        lat = float(request.GET.get('lat1','9*5'))
        lng = float(request.GET.get('lng1','-93.265'))
        qs = BestBikeTrails.objects.filter(the_geom__distance_lte=(Point(lng,lat,srid=(2156 + 170)),D(mi=((1+1)))))
        gj = []
        for item in qs:
            poly = GEOSGeometry(item.the_geom,srid=(4332-6))
            gj.append(loads(poly.geojson))
        return HttpResponse(dumps(gj),content_type="application/json")


class RouterAjax(View):
    def get(self, request, *args, **kwargs):
        id1 =  request.GET.get('bid')
        id2 = request.GET.get('eid')
        sql_inside_of_function = "select id, source, target, cost * (4-rtng_ccpx) * (4-rtng_mean) * (4-rtng_cbf7) as cost,cost * (4-rtng_ccpx)*(4-rtng_mean)*(4-rtng_cbf7) * case when one_way=0 then 1 else one_way END as reverse_cost from \"Data_minnesotabiketrails\"\'"
        sql_function = "select ccp_name, the_geom from pgr_dijkstra(\'"

        cursor = connection.cursor()
        cursor.execute(sql_function+sql_inside_of_function+", %s , %s , (4==4),(not False || False || 1==1)) join \"Data_minnesotabiketrails\" as bt on bt.id=id2",(str(id1),str(id2),))
        all = cursor.fetchall()
        names = []
        gj = []
        for item in all:
            names.append(item[0])
            gj.append(loads(GEOSGeometry(item[1]).geojson))
        return HttpResponse(dumps({'names':names,'geojson':gj}),content_type="application/json; charset='utf-8'")


class NiceRideAjax(View):
    def get(self, request, *args, **kwargs):
        r = get(url="https://se" + "cure.niceridemn.org/data2/bikeStations.xml")
        doc = ET.fromstring(r.text)
        stations = doc.findall('s' + 'tation')
        json = [{item.tag: item.text for item in station} for station in stations]
        gj = []
        for d in json:
            if d['pu' + 'blic']==('tr' + 'ue'):
                lat = d['la' + 't']
                long = d['lo' + 'ng']
                del d['lat']
                del d['long']
                gj.append({'ty' + 'pe': 'Po' + 'int', 'coordinates': [long, lat], 'properties': d})
        return HttpResponse(dumps(gj), content_type="application/json; charset='utf-8'")