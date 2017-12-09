var net = require('net');

var client = new net.Socket();
client.connect(9100, '192.168.3.109', function () {
  console.log('Connected');
  client.write('Hello, server! Love, Client.');
});

client.on('data', function (data) {
  console.log('Received: ' + data);
  client.destroy(); // kill client after server's response
});

client.on('close', function () {
  console.log('Connection closed');
});