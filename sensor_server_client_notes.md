# Sensor server/client
## Ideas
- Use a config file where a client ID is defined. This ID is used to identify the client to the server
- Encrypt traffic
- Make a socket pool with slots for the clients
- Enable some sort of checking when the client connects to the server. If a client with the same name is already connected, the connection is bounced.
