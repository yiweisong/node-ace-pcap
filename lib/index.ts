import nodeGypBuild from 'node-gyp-build'
import { promisify, inherits } from 'util'
import { join } from 'path'
import { EventEmitter } from 'events'

const binding = nodeGypBuild(join(__dirname, '../')) as any;

const LiveDeviceCapture = binding.LiveDeviceCapture

inherits(LiveDeviceCapture, EventEmitter);

export type CreateOptions = {
  iface: string;
  filter?: string;
};

export class EthernetPacketCapture extends EventEmitter {
  private binding;

  constructor(options: CreateOptions) {
    super();
    this.binding = new LiveDeviceCapture(options);
    this.binding.on('data', (data: any) => {
      this.emit('data', data)
    })
  }
  public start(): void {
    this.binding.start();
  }
  public stop(): void {
    this.binding.stop();
  }
  public setFilter(filter: string): void {
    this.binding.setFilter(filter);
  }
  public send(data: Buffer): void {
    this.binding.send(data);
  }
}

export const GetNetworkInterface = function (ip: string) {
  return binding.GetNetworkInterface(ip);
}

export const IsDependencyInstalled = function () {
  return binding.IsDependencyInstalled();
}