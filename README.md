# Node Aceinna PCap

This is a node-addon-api native addon to capture packet from a specifed network. Some code is referenced from https://github.com/mscdex/cap.git

> You need to have Node 10.5.0 or later installed.

## How to use

### Install

```
npm install ace-pcap
```

### Sample Code

> Basic

```Javascript
import { EthernetPacketCapture, GetNetworkInterface } from 'ace-pcap'
const ip = '192.168.22.140'; // IP Address of network for capture packet
const iface = GetNetworkInterface(ip);
const instance = new EthernetPacketCapture({ iface });

instance.on('data', (data) => {
    console.log(data);
})

instance.start();
```

> Filter

```Javascript
import { EthernetPacketCapture, GetNetworkInterface } from 'ace-pcap'
const ip = '192.168.22.140'; // IP Address of network for capture packet
const filter = `ether src 88:e9:fe:52:68:56`;
const iface = GetNetworkInterface(ip);
const instance = new EthernetPacketCapture({ iface, filter });

instance.on('data', (data) => {
  console.log(data);
})

instance.start();

// update filter while capturing
setTimeout(()=>{
  instance.setFilter('ether src 98:5f:d3:3c:ab:fd');
},5000)
```

> Send packet to network

```Javascript
import { EthernetPacketCapture, GetNetworkInterface } from 'ace-pcap'
const ip = '192.168.22.140'; // IP Address of network for capture packet
const iface = GetNetworkInterface(ip);
const instance = new EthernetPacketCapture({ iface });

instance.start();

// update filter while capturing
setTimeout(()=>{
  instance.send(Buffer.from('data to send'))
},5000)
```