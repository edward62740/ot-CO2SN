# ot-CO2SN
 
## Overview

This is a photoacoustic NDIR CO<sub>2</sub> sensor node, utilizing energy harvesting from PV cells to allow for batteryless operation.[^1]<br>
The sensor communicates with the backend/database through 6LoWPAN on a Thread network.

Refer to the functional block diagram below for an overview of the circuit design. <br><br>
![Block Diagram](https://github.com/edward62740/ot-CO2SN/blob/master/Documentation/blockdiagram.jpg)<br>
*Figure 1: CO2SN Functional Block Diagram*
<br><br>
The figures below depict the custom PCB and enclosure.
<br>
<div>
    <img src="https://github.com/edward62740/ot-CO2SN/blob/master/Documentation/co2sn.jpeg" alt="PCB" width="400" />
    <img src="https://github.com/edward62740/ot-CO2SN/blob/master/Documentation/case.jpeg" alt="PCB" width="400" />
</div><br>
*Figure 2a: CO2SN PCB(left), Figure 2b: CO2SN Enclosure (right)*

## Energy Harvesting and Power Management
In general, there are 3 power sources used: the PV cell array (SM141K05TF), supercapacitor (FC0H474ZF), and 2xAA (optional) backup.<br>
The primary assumption is that this sensor will receive only indirect sunlight and/or artificial lighting, hence a larger capacity from other storage technologies such as LIC/Li-ion may not be preferable if the capacity cannot be properly utilized, hence the use of a supercapacitor for the expected multiple discharges (including partial ones) per day.<br>
In the daytime, indoor lighting (approx. 100lux) is largely sufficient to indefinitely sustain a 1.5h sampling rate.
For completely non-battery backed operation, assuming a fully-charged supercapacitor with $1.551\text{C}$, can sustain maximally 10 cycles of CO2 measurements taking up $108\text{mC}$ each. In practice, self-discharge of the supercapacitor and quiescent currents are not negligible, and any sampling rate exceeding 1.5h should be backed up with the 2xAA for night-time operation.<br>

The sensor can also be configured to run completely off batteries.<br>
 
The quiescent power consumption (I<sub>Q</sub>) breakdown by design for all non-trivial loads at T<sub>A</sub>=25 degC are as follows:
| Device   | Current Consumption (Typ) | Conditions          |
|----------|---------------------------|---------------------|
| ST1PS03  | 500nA                     |                     |
| MGM240S  | 2.9µA                     | 256kB RAM on, LFXO  |
| AEM10941 | 0.5µA                     | LDO off             |
| OPT3001  | 0.4µA                     | FS Lux              |
| SCD41    | 0.5µA                     | PD                  |

The supercapacitor (FC0H474ZF) maintains a charge current of <2µA after the initial absorption current. This will be assumed to be the self-discharge current I<sub>SD</sub> for all V<sub>cap</sub> > V<sub>hold</sub>. <br>

Based on empirical data (measured at the V<sub>BACK</sub> node, the average current of the system can be estimated (with the use of the [algorithm](#algorithm)):
<br><br>
$I_{\text{meas}} = \left[\frac{{50.8\text{mA} \times 1.276 \times 2 + 0.12\text{mA} \times (5-1.276) \times 2 + 0.02 \times 4.2}}{{10.02}}\right] = 12.83\text{mA}$<br>
$I_{\text{avg}} = (I_{Q} + I_{SD}) + \left(I_{\text{meas}} \times \frac{{10.03}}{{3600}}\right) \times \{\text{MEASUREMENTS/hr}} \text{  mA}$

For a typical application, such as in a home, CO<sub>2</sub> rates change logarithmically [[1]](#1) provided some ventilation. Hence, sample rate could be adjusted based on the previous measurements' rate of change. <br>

However, in most cases, a sample rate of 30mins-1h (<115µA) is sufficient to detect dangerous changes in indoor CO<sub>2</sub> concentrations. With approx. 200 lux from an indoor LED source in the range 400-1000nm, batteryless operation is achievable. <br>
## Algorithm
This project utilizes a custom [algorithm](https://github.com/edward62740/SCD4x-LPC) designed for ultra-low power applications such as this one, where the manufacturer supplied algorithm is unworkable.<br>

The algorithm is designed to closely mirror the built-in algorithm.

![PCB](https://github.com/edward62740/ot-CO2SN/blob/master/Documentation/current_meas.png)<br>
*Figure 3: Current profile (SCD41 measuring)*


[^1]: This sensor is optionally battery backed by 2xAA cells, to allow for higher sampling frequencies and redundancy.


## References
<a id="1">[1]</a> 
Batog, P., & Badura, M. (2013). Dynamic of Changes in Carbon Dioxide Concentration in Bedrooms. Procedia Engineering, 57, 175–182. https://doi.org/10.1016/j.proeng.2013.04.025