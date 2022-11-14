const { join } = require('path');
const nodeGypBuild = require('node-gyp-build');
const addon = nodeGypBuild(join(__dirname, '../'))

module.exports = addon;