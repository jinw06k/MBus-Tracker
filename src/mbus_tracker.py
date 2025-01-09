import requests
import os
from dotenv import load_dotenv
import time
from datetime import datetime

api_key = ""

def get_data(api_key: str, base: str, **kw):
    """Return JSON data from given request"""
    base_url = f"http://mbus.ltp.umich.edu/bustime/api/v3/{base}?key={api_key}&format=json"
    for key, value in kw.items():
        base_url += f"&{key}={value}"
    return requests.get(base_url).json()

def get_predictions():
    """Return predictions for Northbound buses from CCTC"""
    base = "getpredictions"
    northbound_from_cctc = [["BB", "NORTHBOUND", "C250"], ["NW", "NORTHBOUND", "C251"], ["CN", "NORTHBOUND", "C250"]]


    for route_id, direction, stop_id in northbound_from_cctc:
        prediction_data = get_data(api_key, base, rt=route_id, stpid=stop_id, tmres="s")
        
        if "prd" in prediction_data["bustime-response"]:
            for prediction in prediction_data["bustime-response"]["prd"]:
                if prediction["rtdir"] == direction:
                    print("--------------------")
                    print(f"Current time: {prediction['tmstmp']}")
                    print(f"Next {direction} {route_id} at {stop_id}:")
                    print(f"Bus {prediction['vid']} is arriving in {prediction['prdctdn']} minutes")
                    print(f"prediction type: {prediction['typ']}")

def get_stops():
    """Return all stops for BB, NW, CN routes"""
    base = "getstops"

    route_ids = ["BB", "NW", "CN"]
    
    for route_id in route_ids:
        prediction_data = get_data(api_key, base, rt = route_id, dir = "NORTHBOUND")
        
        for prediction in prediction_data["bustime-response"]["stops"]:
            print(f"Stop for {route_id}: {prediction['stpid']} - {prediction['stpnm']}")

def get_route_directions():
    """Return all directions for BB, NW, CN routes"""
    base = "getdirections"

    route_ids = ["BB", "NW", "CN"]
    
    for route_id in route_ids:
        prediction_data = get_data(api_key, base, rt = route_id)
        
        for prediction in prediction_data["bustime-response"]["directions"]:
            print(f"Directions: {route_id} - {prediction['name']}")

def get_routes():
    """Return all routes"""
    base = "getroutes"

    prediction_data = get_data(api_key, base)
        
    for prediction in prediction_data["bustime-response"]["routes"]:
            print(f"Routes: {prediction['rtnm']} - {prediction['rt']}")
    

if __name__ == "__main__":
    load_dotenv()

    api_key = os.getenv("API_KEY")
    # get_routes()
    # get_route_directions()
    # get_stops()
    
    while True:
        get_predictions()
        time.sleep(10)
        print("------------------------------------------------------")

