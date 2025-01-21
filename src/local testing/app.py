from flask import Flask, jsonify, request
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
    """Return predictions filtered by route (if specified) as JSON"""
    base = "getpredictions"
    all_routes = [
        ["BB", "NORTHBOUND", "C250"], ["CN", "NORTHBOUND", "C250"], ["CS", "SOUTHBOUND", "C250"],
        ["DD", "SOUTHBOUND", "C250"], ["OS", "OUTBOUND", "C250"], ["NX", "SOUTHBOUND", "C250"],
        ["NW", "NORTHBOUND", "C251"], ["NX", "NORTHBOUND", "C251"], ["DD", "NORTHBOUND", "C251"]
    ]
    stop_names = {
        "C250": "CCTC South",
        "C251": "CCTC North"    
    }
    route_filter = request.args.get('route_id')

    result = []
    for route_id, direction, stop_id in all_routes:
        if route_filter and route_filter != "ALL" and route_filter != route_id:
            continue
        prediction_data = get_data(api_key, base, rt=route_id, stpid=stop_id, tmres="s")
        
        if "prd" in prediction_data.get("bustime-response", {}):
            for prediction in prediction_data["bustime-response"]["prd"]:
                if prediction["rtdir"] == direction:
                    result.append({
                        "stop_name": stop_names[stop_id],
                        "route_id": route_id,
                        "direction": direction,
                        "stop_id": stop_id,
                        "vehicle_id": prediction['vid'],
                        "arrival_time": int(prediction['prdctdn']) if prediction['prdctdn'].isdigit() else 0
                    })

    result.sort(key=lambda x: x["arrival_time"])
    
    return jsonify(result)


if __name__ == "__main__":
    app.run(debug=True)