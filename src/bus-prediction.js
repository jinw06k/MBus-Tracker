// used for netlify deployment
// currently deployed at https://jinwook-shin.netlify.app/templates/bus-prediction 

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

//export const handler = async () => {
// serverless function handler
export const handler = async (event, context) => {
  const base = 'getpredictions';
  const northboundFromCctc = [
    ['BB', 'NORTHBOUND', 'C250'],
    ['NW', 'NORTHBOUND', 'C251'],
    ['CN', 'NORTHBOUND', 'C250'],
  ];

  const stopNames = {
    C250: 'CCTC South',
    C251: 'CCTC North',
  };

  const result = [];

  try {
    for (const [routeId, direction, stopId] of northboundFromCctc) {
      const predictionData = await getData(apiKey, base, { rt: routeId, stpid: stopId, tmres: 's' });

      if (predictionData['bustime-response'] && predictionData['bustime-response']['prd']) {
        for (const prediction of predictionData['bustime-response']['prd']) {
          if (prediction.rtdir === direction) {
            result.push({
              stop_name: stopNames[stopId],
              route_id: routeId,
              direction: direction,
              stop_id: stopId,
              vehicle_id: prediction.vid,
              arrival_time: prediction.prdctdn,
            });
          }
        }
      }
    }

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