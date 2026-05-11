import socket
from urllib.parse import urlparse

OK = 0
ERR_CONNECTION = 1
ERR_BAD_REQUEST = 2
ERR_NOT_FOUND = 3
ERR_SERVER = 4


class UrlShortenerClient:
    def __init__(self, host="127.0.0.1", port=9000, timeout=5):
        self.host = host
        self.port = port
        self.timeout = timeout

    def encurta(self, url_original):
        response = self._send_command(f"ENCURTA {url_original}")

        if response.startswith("OK "):
            parts = response.split(maxsplit=2)
            if len(parts) >= 3:
                return OK, parts[2]
            return ERR_SERVER, ""

        return self._error_code(response), ""

    def resolve(self, codigo_curto):
        response = self._send_command(f"RESOLVE {codigo_curto}")

        if response.startswith("OK "):
            return OK, response[3:]

        return self._error_code(response), ""

    def remove_url(self, codigo_curto):
        response = self._send_command(f"REMOVE {codigo_curto}")

        if response == "OK":
            return OK

        return self._error_code(response)

    def codigo_curto(self, url_curta):
        path = urlparse(url_curta).path
        return path.rsplit("/", 1)[-1]

    def comando_raw(self, command):
        return self._send_command(command)

    def _send_command(self, command):
        try:
            with socket.create_connection((self.host, self.port), self.timeout) as sock:
                sock.sendall((command + "\n").encode("utf-8"))
                return self._read_line(sock)
        except OSError:
            return "ERR 503 Connection failed"

    def _read_line(self, sock):
        data = bytearray()

        while True:
            chunk = sock.recv(1)
            if not chunk:
                break
            if chunk == b"\n":
                break
            data.extend(chunk)

        return data.decode("utf-8").strip()

    def _error_code(self, response):
        if response.startswith("ERR 400"):
            return ERR_BAD_REQUEST
        if response.startswith("ERR 404"):
            return ERR_NOT_FOUND
        if response.startswith("ERR 502") or response.startswith("ERR 503"):
            return ERR_SERVER
        return ERR_CONNECTION
