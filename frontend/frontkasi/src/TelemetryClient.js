import Telemetry from "./Telemetry";

const NINKASI_TELEMETRY_ADDRES = 'ninkasi'
const NINKASI_TELEMETRY_PORT = 2346


class TelemetryClient {

    constructor(setTelemetry) {
        this.socket = null;
        this.setTelemetry = setTelemetry;
    }

    start() {
        this.socket = new WebSocket(`ws://${NINKASI_TELEMETRY_ADDRES}:${NINKASI_TELEMETRY_PORT}`);
        this.socket.binaryType = 'arraybuffer';
        this.socket.onopen = this.onOpen.bind(this);
        this.socket.onmessage = this.onMessage.bind(this);
        this.socket.onerror = this.onError.bind(this);
        this.socket.onclose = this.onClose.bind(this);
    }

    onOpen(event) {
        console.log('WebSocket connection opened');
        this.socket.send("HELLO WORLD!");
    }

    onMessage(event) {
        let arrayBuffer = event.data;
        const telemetry = Telemetry.fromBytes(arrayBuffer);
        try {
            this.setTelemetry(telemetry);
        } catch (e) {
            console.error("Failed to parse telemetry!", e);
            this.setTelemetry(new Telemetry()); // Set to empty to indicate error
        }
    }

    onError(error) {
        console.error('WebSocket error:', error);
    }

    onClose(event) {
        console.log('WebSocket connection closed');
    }

}


export default TelemetryClient;
