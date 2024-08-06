from dataclasses import dataclass, field
from queue import Queue
from threading import Thread
from mavcom import MavCom, MavlinkSubscriber, MAVLink_message, MAVLink, MAVError, AP_SYSTEM_ID, AP_COMPONENT_ID
from utils import TcpServer, get_logger
import socket
from typing import Dict


logger = get_logger(__name__)


class MavProxyClient(MavlinkSubscriber):

    def __init__(self, id: int, sock: socket.socket,
                 send_message_to_ap: callable,
                 on_communication_dead: callable) -> None:
        super().__init__()
        self._id = id
        self._send_message_to_ap = send_message_to_ap
        self._on_communication_dead = on_communication_dead
        self._sock = sock
        # Since we'll act as a proxy client, we will use MAVLink object to write data to the remote TCP connection.
        # This means we must set the source ID and component ID pretending to be AP.
        self._mav = MAVLink(file = sock.makefile('wb'), srcSystem=AP_SYSTEM_ID, srcComponent=AP_COMPONENT_ID)
        self._running = True
        self._tx_to_client = Queue()
        self._parse_errors = 0

        # Set timeout on socket to allow for checking for dead connections
        self._sock.settimeout(1)

        Thread(target=self._sender, daemon=True).start()
        Thread(target=self._receiver, daemon=True).start()

    def handle_message(self, msg: MAVLink_message) -> None:
        self._tx_to_client.put(msg)

    def _receiver(self) -> None:
        while self._running:
            try:
                byte = self._sock.recv(1)
                msg = self._mav.parse_char(byte)
                if msg:
                    self._send_message_to_ap(msg)
            except MAVError:
                self._parse_errors += 1
            except socket.timeout:
                # Passthrough, this will occur since we got 1s timeout, needed to check if we're still connected!
                pass
            except Exception as e:
                logger.warn(f'Error when reading mavlink proxy from #{self._id}: {e}')
                self._running = False

        self._on_communication_dead(self._id)

    def _sender(self) -> None:
        while self._running:
            try:
                # NOTE: We must send the packets with the MAVLink object we have, since it handles the sequencer number!
                msg: MAVLink_message = self._tx_to_client.get()
                self._mav.send(msg)
                self._mav.file.flush()
            except socket.timeout as e:
                # Passthrough, this will occur since we got 1s timeout, needed to check if we're still connected!
                pass
            except Exception as e:
                logger.warn(f'Failed to send telemetry data to client #{self._id}: {e}, closing connection')
                self._running = False


class MavProxy(TcpServer):

    def __init__(self, port: int, mavcom: MavCom) -> None:
        TcpServer.__init__(self, port, update_thread=False, handle_client_in_thread=False)
        self._mavcom = mavcom
        self._clients: Dict[int, MavProxyClient] = dict()

    def send_message_to_ap(self, msg: MAVLink_message) -> None:
        self._mavcom.send_message(msg)

    def on_client_communication_dead(self, id: int) -> None:
        self._mavcom.unsubscribe(self._clients[id])
        self._clients.pop(id)

    # override
    def handle_client(self, client: socket.socket, client_id: int) -> None:
        client = MavProxyClient(client_id, client, self.send_message_to_ap, self.on_client_communication_dead)
        self._mavcom.subscribe(client)
        self._clients[client_id] = client
