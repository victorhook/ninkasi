from queue import Queue
from threading import Thread
from telemetry import Telemetry, telem_to_bytes
from utils import TcpServer
import socket
from typing import Dict


class TelemetryServer(TcpServer):

    def __init__(self, port: int, telemetry: Telemetry) -> None:
        super().__init__(port, update_thread=False, handle_client_in_thread=False)
        self._clients: Dict[int, socket.socket] = dict()
        self._telemetry = telemetry
        self._tx = Queue()

    def update(self) -> None:
        self._tx.put(telem_to_bytes(self._telemetry))

    def start(self) -> None:
        Thread(target=self._sender, daemon=True).start()
        super().start()

    def _sender(self) -> None:
        while self.is_running():
            data = self._tx.get()
            pop_clients = []

            for client_id, client in self._clients.items():
                try:
                    client.send(data)
                except Exception as e:
                    self.log.error(f'Failed to send telemetry data to client #{client_id}: {e}, closing connection')
                    pop_clients.append(client_id)
                    try:
                        client.close()
                    except Exception as e:
                        pass

            for client_id in pop_clients:
                self._clients.pop(client_id)

    def handle_client(self, client: socket.socket, client_id: int) -> None:
        self._clients[client_id] = client