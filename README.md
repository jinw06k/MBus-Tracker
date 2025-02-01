# On Time Every Time - Live MBus Tracker

This repository contains the source code for a real-time MBus tracker specifically for Northbound buses at the Central Campus Transit Center. It was developed to help me and my roommate stay on schedule for buses heading to North Campus for classes.

[Magic Bus API](https://mbus.ltp.umich.edu/home) is provided by UM Transit at the University of Michigan - Ann Arbor.

## Hardware Requirements
- ESP32
- 7-seg Display
- SN74HC595 Shift Register
- Rotary Encoder
- Resistors (220Ω)
- Wires
- Additional requirements to be updated.

## Local Testing
A Flask app was initially developed to test and monitor API requests.

<img src="/assets/images/old/testing.png" width="500">

## Web Deployment
A JavaScript script was developed and integrated into my personal website, enabling my roommates and me to access it on our phones. [Check it out here!](http://jinwookshin.com/templates/bus-prediction)

<img src="/assets/images/final/MobileDemo.gif" width="400">

## ESP32 Testing #1: Serial Testing
C++ code was written for the ESP32 to test WiFi connectivity and API requests. Below is a video comparing the JSON data retrieved via ESP32 with the deployed web version.

(https://youtu.be/zxtD61-A_Fs)

[![Video Placeholder](https://img.youtube.com/vi/zxtD61-A_Fs/0.jpg)](https://youtu.be/zxtD61-A_Fs)

## ESP32 Testing #2: Rotary Cncoder Testing
Rotary Encoder added - Jan. 28
(https://www.youtube.com/shorts/zzOmB_uZ6Bs)

[![Video Placeholder](https://img.youtube.com/vi/zzOmB_uZ6Bs/0.jpg)](https://www.youtube.com/shorts/zzOmB_uZ6Bs)

## Final Version
The final version of the board allows tracking of three bus routes. A rotary encoder can be used to switch between routes, and the RGB LED displays the representative color of the current route. Two sets of 7-segment displays show the estimated arrival times for the next two available buses. With built-in WiFi connectivity, the ESP32 eliminates the need for a computer connection, making the board fully stand-alone.
(https://youtube.com/shorts/56vkLiOQles)

[![Video Placeholder](https://img.youtube.com/vi/56vkLiOQles/0.jpg)](https://www.youtube.com/shorts/56vkLiOQles)

## Todo

- [x] Local testing
- [x] Deploy to Netlify
- [x] Enhance Mobile View
- [x] ESP32 testing on serial monitor
- [x] Connect to final hardware setup
- [ ] Add alphanumeric display for bus route info
