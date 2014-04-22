relaycontrol-arduino
====================

MQTT controlled relays (Arduino-side code)

This is designed for a 16 relay board but can be adjusted to suit however many relays you want - you just need to change a few settings yourself.
The board starts with all relays turned off (relayCheck[] at all zeros).
When a message is received via MQTT, the code checks its validity.

- IF the code is less than 0 or greater than 16 while not being 998 or 999, the status is dumped as an error.
- IF the code is between 0 and 16 (inclusive):
- - this index in relayCheck[] is flipped
- - a status message is published with the new status
- IF the code is 998, all relays are turned ON (mostly for testing but the functionality is there) and a status message is published with the new status
- IF the code is 999, a status message is published with the current state of relayCheck[] (eg with relays 1, 4, 6 and 12 turned on: 1001010000010000)

relayCheck[] holds the bit value for all relays (0 for off, 1 for on).



Requires 
--------
PubSubClient.h - download from here: http://knolleary.net/arduino-client-for-mqtt/

Shift register(s) to push the data to the appropriate pins on the relay board(s). Code is easily expandable/contractable for multiple boards/relays



TODO
----
- Code cleanup
- Configuration options at the start for different number of relays


Use this code any way you want. Props would be nice (beer would be nicer) but take it and run with it. I'd love to see what you come up with.