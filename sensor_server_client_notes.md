# Sensor server/client
## Ideas
- Use a config file where a client ID is defined. This ID is used to identify the client to the server
- Encrypt traffic
- Make a socket pool with slots for the clients
- Enable some sort of checking when the client connects to the server. If a client with the same name is already connected, the connection is bounced.
- Create monitor capabilities. The monitor has no timeout. 
- Create a monitor service that monitors the server and controls some LED or similar (not important, but cool)

## Plan
- The Sensors connect to the server and transmits the part GPRMC part of NMEA.
- The server stores the data in a database along with the identifier for the sensor and the timefix (225566 : 22:54:46 UTC)
- Use the "STORE" key word as defined in the protocol to store to DB. 
