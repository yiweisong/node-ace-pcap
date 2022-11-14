/// <reference types="node" />

/*~ Write your module's methods and properties in this class */
export class EthernetPacketCapture {
  constructor(opts: MessageParser.Options);

  receive(buffer: Buffer): MessageParser.ResultList[];
}

/*~ If you want to expose types from your module as well, you can
 *~ place them in this block.
 */
export namespace MessageParser {
  type PacketLengthType = 'uint8' | 'uint32'

  export interface Options {
    key: string;
    user: UserOptions;
    nmea?: NMEAOptions;
    skipCheckCRC?: boolean;
  }

  export interface UserOptions {
    allows: number[];
    packetLengthType: PacketLengthType;
  }

  export interface NMEAOptions {
    allows: string[];
  }

  export interface ResultList {
    packetType: number;
    payload: Buffer | string;
  }
}