# On Time Every Time - Live MBus Tracker

This repository contains the source code for a real-time MBus tracker specifically for Northbound buses at the Central Campus Transit Center. It was developed to help me and my roommate stay on schedule for buses heading to North Campus for classes.

[Magic Bus API](https://mbus.ltp.umich.edu/home) is provided by UM Transit at the University of Michigan - Ann Arbor.

## Hardware Requirements
- ESP32
- Additional requirements to be updated.

## Local Testing
A Flask app was initially developed to test and monitor API requests.
![Testing Image](/assets/images/testing.png)

## Web Deployment
A JavaScript script was developed and integrated into my personal website, enabling my roommates and me to access it on our phones. [Check it out here!](https://jinwook-shin.netlify.app/templates/bus-prediction)
![Web Image](/assets/images/web.png)

## ESP32 Testing
C++ code was written for the ESP32 to test WiFi connectivity and API requests. Below is a video comparing the JSON data retrieved via ESP32 with the deployed web version.
[![Video Placeholder](https://youtu.be/zxtD61-A_Fs)

## Final Version
Details will be updated soon.

## Todo

- [x] Local testing
- [x] Deploy to Netlify
- [x] ESP32 testing on serial monitor
- [ ] Connect to final hardware setup
