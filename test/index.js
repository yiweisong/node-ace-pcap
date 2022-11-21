const { EthernetPacketCapture, GetNetworkInterface } = require('../dist/index');
const { struct } = require('./struct');

const HEADER = [0x55, 0x55];

const MIN_ETH_COMMAND_LENGTH = 46;

const hostMAC = '88:e9:fe:52:68:56'; //98:5f:d3:3c:ab:fd

const ip = '192.168.22.140'; //local ip

const iface = GetNetworkInterface(ip);

const filter = `ether src ${hostMAC}`;

const instance = new EthernetPacketCapture({ iface, filter });

instance.on('data', (data) => {
    console.log(data);
})

instance.start();

//instance.setFilter('ether src 98:5f:d3:3c:ab:fd');

//instance.setFilter(`(ether src ${hostMAC} or ether dst ${hostMAC})`)

const command = buildETHCommand(
    "ff:ff:ff:ff:ff:ff",
    hostMAC,
    [0x01, 0xcc]
)
instance.send(command);

setTimeout(() => {
    instance.setFilter(`ether dst ${hostMAC}`)
    const command1 = buildETHCommand(
        hostMAC,
        "ff:ff:ff:ff:ff:ff",
        [0x01, 0xcc]
    )
    instance.send(command1);
}, 5 * 1000);

// setTimeout(() => {
//     instance.setFilter(`ether src ${hostMAC}`)
//     const command2 = buildETHCommand(
//         "ff:ff:ff:ff:ff:ff",
//         hostMAC,
//         [0x01, 0xcc]
//     )
//     instance.send(command2);

// }, 10 * 1000);


function buildArray(length, defaultValue) {
    // eslint-disable-next-line prefer-spread
    return Array.apply(null, Array(length)).map(() => defaultValue);
}

function arrayBufferToArray(buf) {
    const view = new Uint8Array(buf);
    const numberArray = [];
    view.forEach((byte) => {
        numberArray.push(byte);
    });
    return numberArray;
}

function convertStringToBytesArray(value, spliter = ':') {
    const splittedValues = value.split(spliter);
    return splittedValues.map((item) => parseInt(item, 16));
}

function calculateCRC(payload) {
    let crc = 0x1d0f;
    for (const bytedata of payload) {
        crc = crc ^ (bytedata << 8);
        let i = 0;
        while (i < 8) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc = crc << 1;
            }
            i += 1;
        }
    }

    crc = crc & 0xffff;
    const crc_msb = (crc & 0xff00) >> 8;
    const crc_lsb = crc & 0x00ff;
    return [crc_msb, crc_lsb];
}

function buildETHCommand(
    dst,
    src,
    packetType,
    payload = [],
) {
    const pureCommandArray = [
        ...packetType,
        ...arrayBufferToArray(struct('<I').pack(payload.length)),
        ...payload,
    ];

    const packetLength = HEADER.length + pureCommandArray.length + 2;
    const packetLengthArray = arrayBufferToArray(struct('<H').pack(packetLength));
    const command = [].concat(
        convertStringToBytesArray(dst),
        convertStringToBytesArray(src),
        packetLengthArray,
        HEADER,
        pureCommandArray,
        calculateCRC(pureCommandArray),
    );

    const needFillBytesLength = MIN_ETH_COMMAND_LENGTH - packetLength;
    if (needFillBytesLength > 0) {
        command.push(...buildArray(needFillBytesLength, 0));
    }

    return Buffer.from(command);
}