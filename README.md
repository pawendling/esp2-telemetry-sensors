# ESP32 Code for: MQTT, Display, and Sensors 

## DHT22 - Temperature and Humidity 
<img width="643" height="398" alt="image" src="https://github.com/user-attachments/assets/ffa023a5-9583-4e64-8a78-93343470693b" />

| Model                     | DHT22 |
| -- | -- | 
| Power supply              | 3.3-6V DC |
|Output signal              |digital signal via single-bus | 
|Sensing element            |Polymer capacitor | 
|Operating range            |humidity 0-100%RH;   temperature -40~80Celsius | 
| Accuracy                  | humidity +-2%RH(Max +-5%RH);   temperature <+-0.5Celsius |
| Resolution or sensitivity | humidity 0.1%RH; temperature 0.1Celsius | 
| Repeatability             | humidity +-1%RH; temperature +-0.2Celsius | 
| Humidity hysteresis       | +-0.3%RH      |
| Long-term Stability       | +-0.5%RH/year |
| Sensing period            | Average: 2s |
| Interchangeability        | fully interchangeable |
| Dimensions                |  small size 14*18*5.5 mm; big size 22*28*5 mm |
## ENS160 - AQI, VOC, and CO2 
<img width="400" alt="image" src="https://github.com/user-attachments/assets/6abcab34-2f10-4919-ae71-a50aa44fbf93" />

RMOX0 — Ethanol / Alcohols / Human bioeffluents
- Highly sensitive to ethanol, isopropanol, and breath VOCs
- Strong correlation with human occupancy
- Major contributor to eCO₂ estimation
This channel spikes when:
- Someone breathes near the sensor
- Alcohol-based cleaners are used
- Cooking vapors are present

RMOX1 — Reducing gases (Hydrogen, CO, some hydrocarbons)
- Sensitive to hydrogen, carbon monoxide, and light hydrocarbons
- Responds strongly to combustion byproducts
This channel spikes during:
- Gas stove use
- Candle burning
- Smoking
- Combustion leaks

RMOX2 — Aromatic VOCs (Toluene, xylene, solvents)
- Tuned for aromatics and industrial VOCs
- Sensitive to paints, glues, varnishes, 3D printing fumes
This channel spikes when:
- Painting a room
- Using adhesives
- Running a resin 3D printer

RMOX3 — Oxidizing gases (O₃, NO₂) and long‑chain VOCs
- Sensitive to ozone, nitrogen dioxide, and oxidizing pollutants
- Also responds to long‑chain VOCs from plastics, cleaners, and off‑gassing
This channel spikes when:
- Outdoor air with ozone enters
- Strong cleaners are used
- New furniture off‑gasses



## AHT21 - Temperature and Humidity 
<img width="320" height="254" alt="image" src="https://github.com/user-attachments/assets/a3673f4e-21fb-4dab-b495-1b0de5f547c9" />
<br>
<img width="558" height="448" alt="image" src="https://github.com/user-attachments/assets/ec50438b-3d0a-45f2-b0a1-49298f146790" />
<br>
<img width="579" height="482" alt="image" src="https://github.com/user-attachments/assets/e3d99901-b26b-4adc-b9d0-b2c51c995213" />

