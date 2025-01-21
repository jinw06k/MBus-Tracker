import fetch from 'node-fetch'

import axios from 'axios'
// const axios = require('axios');
import dotenv from 'dotenv';
dotenv.config();


const apiKey = process.env.API_KEY;

async function getData(apiKey, base, params = {}) {
  const baseUrl = `http://mbus.ltp.umich.edu/bustime/api/v3/${base}?key=${apiKey}&format=json`;
  const urlParams = new URLSearchParams(params).toString();
  const fullUrl = `${baseUrl}&${urlParams}`;

  try {
    const response = await axios.get(fullUrl);
    return response.data;
  } catch (error) {
    console.error('Error fetching data:', error);
    throw error;
  }
}

export const handler = async (event) => {
  const base = 'getpredictions';
  const allRoutes = [
    ['BB', 'NORTHBOUND', 'C250'], ['CN', 'NORTHBOUND', 'C250'], ['CS', 'SOUTHBOUND', 'C250'],
    ['DD', 'SOUTHBOUND', 'C250'], ['OS', 'OUTBOUND', 'C250'], ['NX', 'SOUTHBOUND', 'C250'],
    ['NW', 'NORTHBOUND', 'C251'], ['NX', 'NORTHBOUND', 'C251'], ['DD', 'NORTHBOUND', 'C251']
  ];
  const stopNames = {
    C250: 'CCTC South',
    C251: 'CCTC North',
  };

  const routeFilter = event.queryStringParameters?.route_id;
  const result = [];

  try {
    for (const [routeId, direction, stopId] of allRoutes) {
      if (routeFilter && routeFilter !== 'ALL' && routeFilter !== routeId) {
        continue;
      }

      const predictionData = await getData(apiKey, base, { rt: routeId, stpid: stopId, tmres: 's' });

      if (predictionData['bustime-response']?.prd) {
        for (const prediction of predictionData['bustime-response']['prd']) {
          if (prediction.rtdir === direction) {
            result.push({
              stop_name: stopNames[stopId],
              route_id: routeId,
              direction: direction,
              stop_id: stopId,
              vehicle_id: prediction.vid,
              arrival_time: Number(prediction.prdctdn) || 0,
            });
          }
        }
      }
    }

    result.sort((a, b) => a.arrival_time - b.arrival_time);

    return {
      statusCode: 200,
      body: JSON.stringify(result),
    };
  } catch (error) {
    console.error('Error:', error);
    return {
      statusCode: 500,
      body: JSON.stringify({ error: 'Failed to fetch bus predictions' }),
    };
  }
};