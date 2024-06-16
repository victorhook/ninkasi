import Telemetry from "../Telemetry";

const NINKASI_TELEMETRY_ADDRES = 'ninkasi'
const NINKASI_LOG_PORT = 2348


class LogClient {

    constructor(setApLog, setNinkasiLog) {
        this.socket = null;
        this.setApLog = setApLog;
        this.setNinkasiLog = setNinkasiLog;
    }

    start() {
        this.socket = new WebSocket(`ws://${NINKASI_TELEMETRY_ADDRES}:${NINKASI_LOG_PORT}`);
        this.socket.onopen = this.onOpen.bind(this);
        this.socket.onmessage = this.onMessage.bind(this);
        this.socket.onerror = this.onError.bind(this);
        this.socket.onclose = this.onClose.bind(this);
    }

    onOpen(event) {
        console.log('WebSocket connection opened');
    }

    onMessage(event) {
        console.log(event);
        const log_block = JSON.parse(event.data);

        let text = log_block.msg;

        if (log_block.type == "ap") {
            this.setApLog(text);
        } else if (log_block.type == "ninkasi") {
            this.setNinkasiLog(text);
        }
    }

    onError(error) {
        console.error('WebSocket error:', error);
    }

    onClose(event) {
        console.log('WebSocket connection closed');
    }

}


export default LogClient;
