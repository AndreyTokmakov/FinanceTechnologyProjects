import json
import logging
import random
import signal
import socket
import string
import sys
import textwrap
import time
from enum import Enum, IntEnum
import websocket
import requests
import multiprocessing.queues

from dataclasses import dataclass
from http import HTTPStatus
from multiprocessing import Process, Queue, Event
from queue import Empty
from typing import Any, List, Tuple, Dict
from requests import Response
from websocket import WebSocket
from Binance.api.api_keys import get_api_key
from Binance.api.common import HTTPHeader

socket_path: str = "/tmp/unix_socket"


class EventType(IntEnum):
    DepthSnapshot = 1
    DepthUpdate = 2


class API(object):
    TESTNET_API_HOST: str = 'https://testnet.binance.vision/api'
    api_key: str = get_api_key()
    default_headers: Dict = {'X-MBX-APIKEY': api_key}

    @staticmethod
    def send_get_request(endpoint: str,
                         headers: Dict = None,
                         params: Dict = None) -> Dict:
        if not headers:
            headers = API.default_headers
        if not params:
            params = {}
        response: Response = requests.get(url=f'{API.TESTNET_API_HOST}/{endpoint}',
                                          headers=headers,
                                          params=params)
        if HTTPStatus.OK == response.status_code:  # HTTP_OK
            content_type: str = response.headers.get(HTTPHeader.ContentType)
            if content_type and 'application/json' in content_type:
                return response.json()
            else:
                return {}
        else:
            return {}

    @staticmethod
    def get_depth(symbol: str = 'BTCUSDT',
                  limit: int = 5000) -> Dict:
        return API.send_get_request(endpoint='v3/depth', params={'symbol': symbol, 'limit': limit})


class BinanceConnector(object):

    MAX_PAYLOAD_SIZE: int = 1024
    BINANCE_TESTNET_HOST: str = 'wss://testnet.binance.vision'
    SOCKET_PATH: str = "/tmp/unix_socket"

    CONNECTOR: str = 'Binance'

    def __init__(self):
        self.symbol: str = 'BTCUSDT'
        self.message_queue: Queue = Queue()
        self.stop_event: Event = Event()

        self.forwarder: Process = Process(target=self.even_forwarder)
        self.api_worker: Process = Process(target=self.api_worker_thread)

        self.web_socket = websocket.WebSocketApp(f'{self.BINANCE_TESTNET_HOST}/stream',
                                                 on_message=self.on_message,
                                                 on_error=self.on_error,
                                                 on_close=self.on_close,
                                                 on_open=self.on_open,
                                                 on_ping=self.on_ping)

        signal.signal(signal.SIGINT, self.signal_handler)

    @staticmethod
    def send_event(channel,
                   event_type: EventType,
                   msg: str):
        data: Dict = {
            'connector': BinanceConnector.CONNECTOR,
            'type': event_type,
            'data': msg
        }
        payload: bytes = json.dumps(data).encode('utf-8')
        channel.sendall(payload)

    def api_worker_thread(self):
        # while True:
        time.sleep(2)
        depth: Dict = API.get_depth(symbol=self.symbol, limit=5000)
        self.message_queue.put((EventType.DepthSnapshot, depth))

    def even_forwarder(self):
        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as unix_socket:
            unix_socket.connect(self.SOCKET_PATH)
            while True:
                try:
                    event_type, data = self.message_queue.get(timeout=0.25)
                    self.send_event(channel=unix_socket, event_type=event_type, msg=data)
                except Empty:
                    if self.stop_event.is_set():
                        break
                    continue

    def on_message(self,
                   ws_socket: WebSocket,
                   message: Any):
        self.message_queue.put((EventType.DepthUpdate, json.loads(message)))

    @staticmethod
    def on_error(ws_socket: WebSocket,
                 error: Any):
        sys.stderr.write(f"Error: {error}")

    @staticmethod
    def on_close(ws_socket: WebSocket,
                 close_status_code: Any,
                 close_msg: Any):
        print(f"WebSocket connection closed: {close_status_code} - {close_msg}")

    def on_open(self,
                ws_socket: WebSocket):
        subscribe_message: Dict = {
            "method": "SUBSCRIBE",
            "params": [
                # f"{self.symbol.lower()}@aggTrade",
                f"{self.symbol.lower()}@depth"
            ],
            "id": 1
        }
        ws_socket.send(json.dumps(subscribe_message))

    @staticmethod
    def on_ping(ws_socket: WebSocket,
                message: Any):
        ws_socket.send(message, websocket.ABNF.OPCODE_PONG)

    def signal_handler(self, sig, frame):
        print('Stopping connector')
        self.stop_event.set()

    def start(self):
        self.forwarder.start()
        self.api_worker.start()
        self.web_socket.run_forever()


# TODO: Redis instead UDP ?? https://opstree.com/blog/2019/04/16/redis-best-practices-and-performance-tuning/

if __name__ == '__main__':
    connector: BinanceConnector = BinanceConnector()
    connector.start()
