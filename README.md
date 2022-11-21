# Node Aceinna PCap

This is a native addon to capture packet from a specifed network. Some code is referenced from https://github.com/mscdex/cap.git

> You need to have Node 10.5.0 or later installed.

## How to use

### Install 
```
npm install node-ace-pcap
```

### Sample Code
```

```

const capture = new EthernetPacketCapture(options)
capture.open()
capture.close()
capture.destory()

capture.setFilter('xxx')
capture.getFilter()

capture.on('data',(packet)=>{})
capture.on('close',(packet)=>{})
capture.on('data',(packet)=>{})

question

1. work as stream.duplex?

## C++ How to implement

EthernetPacketCapure
construtor()
pcap_init

Open()
pcap_open_live

Close()
pcap_close

Read()
pcap_loop
or
pcap_next_ex

Write()
pcap_sendpacket

SetFilter()
pcap_setfilter

## usage

```typescript
const ethernetPort: EthernetPort = EthernetPortFactory.create({
  interface: "en0",
  port: "device mac address",
});
ethernetPort.on("data", xxx);
ethernetPort.open();
ethernetPort.close();

const capture = new EthernetCapture();
ethernetPort.bind(capture);
capture.on("data", (src, dst, data) => {
  if (src == "deviceMac" || dst === "deviceMac") {
    this.ports["deviceMac"].emit("data", data);
  }
});
```

```Cpp
class EthernetPacketCapure
```
