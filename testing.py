import requests
import os
from dotenv import load_dotenv

api_key = ""

def get_data(api_key: str, base: str, **kw):
    """Return JSON data from given request with given kwargs"""
    base_url = f"http://mbus.ltp.umich.edu/bustime/api/v3/{base}?key={api_key}&format=json"
    for key, value in kw.items():
        base_url += f"&{key}={value}"
    return requests.get(base_url).json()

def get_stops():
    base = "getstops"

    route_ids = ["BB", "NW", "CN"]
    
    for route_id in route_ids:
        prediction_data = get_data(api_key, base, rt = route_id, dir = "NORTHBOUND")
        
        for prediction in prediction_data["bustime-response"]["stops"]:
            print(f"Stop for {route_id}: {prediction['stpid']} - {prediction['stpnm']}")

def get_route_directions():
    base = "getdirections"

    route_ids = ["BB", "NW", "CN"]
    
    for route_id in route_ids:
        prediction_data = get_data(api_key, base, rt = route_id)
        
        for prediction in prediction_data["bustime-response"]["directions"]:
            print(f"Directions: {route_id} - {prediction['name']}")

def get_routes():
    base = "getroutes"

    prediction_data = get_data(api_key, base)
        
    for prediction in prediction_data["bustime-response"]["routes"]:
            print(f"Routes: {prediction['rtnm']} - {prediction['rt']}")
    

if __name__ == "__main__":
    load_dotenv()

    api_key = os.getenv("API_KEY")
    # get_routes()
    # get_route_directions()
    get_stops()

