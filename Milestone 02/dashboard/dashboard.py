import argparse
import collections
import json
import math
from pathlib import Path
import subprocess
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

records = {name: collections.deque(maxlen=1800) for name in ('a', 'b')}
lock = threading.Lock()
status = 'Waiting for MQTT messages'

def accept(line):
    topic, payload = line.strip().split(' ', 1)
    device = {'elech423/esp32-a/sensor': 'a', 'elech423/esp32-b/sensor': 'b'}.get(topic)
    if device is None:
        return
    data = json.loads(payload)
    temperature, humidity = float(data['temperature']), float(data['humidity'])
    if not (math.isfinite(temperature) and math.isfinite(humidity) and 0 <= humidity <= 100):
        raise ValueError('Invalid measurement')
    with lock:
        records[device].append(dict(time=time.time(), temperature=temperature, humidity=humidity))

def receive(host, stop):
    global status
    executable = Path('C:/Program Files/mosquitto/mosquitto_sub.exe')
    while not stop.is_set():
        status = 'Waiting for MQTT messages'
        with subprocess.Popen([str(executable), '-h', host, '-t', 'elech423/+/sensor', '-v'],
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True,
                              creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0)) as process:
            def shutdown():
                stop.wait()
                if process.poll() is None:
                    process.terminate()
            threading.Thread(target=shutdown, daemon=True).start()
            for line in process.stdout:
                try:
                    accept(line)
                    status = 'Receiving MQTT data'
                except (ValueError, KeyError, TypeError):
                    status = 'Ignored an invalid sensor message'
            error = process.stderr.read().strip()
            status = error or 'MQTT disconnected; retrying'
        stop.wait(3)

class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/api/data':
            with lock:
                body = json.dumps(dict(devices={k: list(v) for k, v in records.items()}, status=status)).encode()
            content_type = 'application/json'
        elif self.path == '/':
            body = Path(__file__).with_name('dashboard.html').read_bytes()
            content_type = 'text/html; charset=utf-8'
        else:
            self.send_error(404)
            return
        self.send_response(200)
        self.send_header('Content-Type', content_type)
        self.send_header('Cache-Control', 'no-store')
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, *_):
        pass

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--broker', required=True)
    parser.add_argument('--port', type=int, default=8080)
    args = parser.parse_args()
    stop = threading.Event()
    threading.Thread(target=receive, args=(args.broker, stop), daemon=True).start()
    server = ThreadingHTTPServer(('127.0.0.1', args.port), Handler)
    print(f'Open http://localhost:{args.port} — keep this window open.', flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        stop.set()
        server.server_close()
