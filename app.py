from flask import Flask, jsonify
from flask_cors import CORS
import requests
import os
from dotenv import load_dotenv
from datetime import datetime

app = Flask(__name__)
CORS(app)
load_dotenv()
api_key = os.getenv("API_KEY")

def get_data(api_key: str, base: str, **kw):
    """Return JSON data from given request"""
    base_url = f"http://mbus.ltp.umich.edu/bustime/api/v3/{base}?key={api_key}&format=json"
    for key, value in kw.items():
        base_url += f"&{key}={value}"
    return requests.get(base_url).json()

@app.route("/predictions", methods=["GET"])
def get_predictions():
    """Return predictions for Northbound buses from CCTC as JSON"""
    base = "getpredictions"
    northbound_from_cctc = [["BB", "NORTHBOUND", "C250"], ["NW", "NORTHBOUND", "C251"], ["CN", "NORTHBOUND", "C250"]]
    stop_names = {
        "C250": "CCTC South",
        "C251": "CCTC North"    
    }
    

    result = []


    for route_id, direction, stop_id in northbound_from_cctc:
        prediction_data = get_data(api_key, base, rt=route_id, stpid=stop_id, tmres="s")
        
        if "prd" in prediction_data["bustime-response"]:
            for prediction in prediction_data["bustime-response"]["prd"]:
                if prediction["rtdir"] == direction:
                    result.append({
                        "stop_name": stop_names[stop_id],
                        "route_id": route_id,
                        "direction": direction,
                        "stop_id": stop_id,
                        "vehicle_id": prediction['vid'],
                        "arrival_time": prediction['prdctdn']
                    })
    
    return jsonify(result)

if __name__ == "__main__":
    app.run(debug=True)
