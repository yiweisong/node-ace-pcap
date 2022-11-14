//const addon = require('../build/Release/ace-pcap.node')
const { join } = require('path');
const nodeGypBuild = require('node-gyp-build');
const binding = nodeGypBuild(join(__dirname, '../'))

binding.helloNode();