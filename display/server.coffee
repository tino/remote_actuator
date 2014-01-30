fs = require 'fs'
express = require 'express'
http = require 'http'

app = express()
server = http.createServer(app)
io = require('socket.io').listen(server)
port = 8080
server.listen(port, '127.0.0.1')

serialport = require("serialport")


serialListener = (io) ->
  serial = new serialport.SerialPort "/dev/tty.usbserial-A9014F5C",
    baudrate: 19200,
    dataBits: 8,
    parity: 'none',
    stopBits: 1,
    flowControl: false
    parser: serialport.parsers.readline("\n", "binary")

  serial.on "open", ->
    console.log "serial connection open"
  serial.on "data", (data) ->
    console.log data
    message = data
    len = message.length
    if message[0..1] == '<<'
      values = message[2...len-2].split(':')
      io.sockets.emit values[1], [values[2], values[3]]
    else
      console.log "Cant understand:#{message[0..1]}.#{message[len-3..len]}"

  return serial


app.use(express.static(__dirname + '/public'));

app.get '/', (req, res) ->
  res.sendfile __dirname + '/index.html'

serial = serialListener(io)

io.sockets.on 'connection', (socket) ->
  socket.emit 'live', 1

  socket.on 'move to', (data) ->
    console.log "move to #{data}"
    buf = new Buffer('AFAZMxxxFA')
    buf.writeUInt8(data % 128, 6)
    buf.writeUInt8(data >> 7, 7)
    console.log buf
    serial.write(buf)

  socket.on 'stop', (data) ->
    console.log "stopping"
    buf = new Buffer('AFAZSxxxFA')
    serial.write(buf)

