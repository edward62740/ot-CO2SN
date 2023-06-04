# ot-CO2SN [work in progress; U/C]
 
## Overview

This is a photoacoustic NDIR CO<sub>2</sub> sensor node, utilizing energy harvesting from PV cells to allow for batteryless operation.[^1]<br>
The sensor communicates with the backend/database through 6LoWPAN on a Thread network.

Refer to the functional block diagram below for an overview of the custom PCB. <br><br>
![Block Diagram](https://github.com/edward62740/ot-CO2SN/blob/master/Documentation/blockdiag.jpg)<br>
*Figure 1: CO2SN Functional Block Diagram*

## Energy Harvesting and Power Management
In general, there are 3 power sources used: the PV cell, supercapacitor (0.47F), and 2xAA (optional) backup.<br>
The primary assumption is that this sensor will receive only indirect sunlight and/or artificial lighting, hence a larger capacity from other storage technologies such as LIC/Li-ion may not be preferable if the capacity cannot be properly utilized, hence the use of a supercapacitor for the expected 1 discharge per day.<br>
In the daytime, indoor lighting (approx. 100lux) is largely sufficient to indefinitely sustain a 30min sampling rate.
For completely non-battery backed operation, assuming a fully-charged supercapacitor with $1.551\text{C}$, can sustain maximally 10 cycles of CO2 measurements taking up $108\text{mC}$ each. In practice, self-discharge of the supercapacitor and quiescent currents are not negligible, and any sampling rate exceeding 1.5h should be backed up with the 2xAA for night-time operation.<br><br>
 
The idle power consumption breakdown by design at T<sub>A</sub>=25 degC are as such:
| Device   | Current Consumption (Typ) | Conditions    |
|----------|---------------------------|---------------|
| ST1PS03  | 500nA                     |               |
| ST1PS03  | 10nA                      | EN deasserted |
| MGM240S  | 1.8µA                     | 16kB RAM on   |
| TXS0102  | 1µA                       | High-Z        |
| AEM10941 | 0.5µA                     | LDO off       |

## Algorithm
This project utilizes a custom [algorithm](https://github.com/edward62740/SCD4x-LPC) designed for ultra-low power applications such as this one, where the manufacturer supplied algorithm is unworkable.




[^1]: This sensor is optionally battery backed by 2xAA cells, to allow for higher sampling frequencies and redundancy.