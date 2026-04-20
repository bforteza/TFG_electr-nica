"""
Sistema de log persistente para el webserver. Escribe en ../logs/armario.log
(mismo archivo que la app C++). Usa WatchedFileHandler para detectar rotaciones.
Hilo daemon monitoriza la red cada 30 s (UDP socket trick) y loguea transiciones UP/DOWN.
"""

import logging
import logging.handlers
import os
import socket
import threading
import time

_LOG_DIR  = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'logs'))
_LOG_PATH = os.path.join(_LOG_DIR, 'armario.log')

_logger = logging.getLogger('armario')
_net_up = None  # estado actual de red; None = desconocido


class _Fmt(logging.Formatter):
    _NAMES = {logging.INFO: 'INFO ', logging.ERROR: 'ERROR', logging.WARNING: 'WARN '}

    def format(self, record):
        record.lvl = self._NAMES.get(record.levelno, f'{record.levelname:<5}')
        record.src = f"{getattr(record, 'source', 'WEB'):<7}"
        return super().format(record)


def info(source: str, msg: str) -> None:
    _logger.info(msg, extra={'source': source})


def error(source: str, msg: str) -> None:
    _logger.error(msg, extra={'source': source})


def _check_net() -> bool:
    # Conectar un socket UDP no envía datos pero obliga al kernel a resolver
    # la tabla de rutas. Si la IP resultante no es loopback, hay red disponible.
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.settimeout(0)
        s.connect(('10.254.254.254', 1))
        ip = s.getsockname()[0]
        s.close()
        return ip != '' and not ip.startswith('127.')
    except Exception:
        return False


def _net_monitor() -> None:
    global _net_up
    while True:
        time.sleep(30)
        up = _check_net()
        if up != _net_up:
            _net_up = up
            if up:
                info('WEB', 'Network UP')
            else:
                error('WEB', 'Network DOWN')


def setup() -> None:
    global _net_up
    os.makedirs(_LOG_DIR, exist_ok=True)

    handler = logging.handlers.WatchedFileHandler(_LOG_PATH)
    handler.setFormatter(_Fmt(
        '%(asctime)s [%(lvl)s] [%(src)s] %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S',
    ))
    _logger.setLevel(logging.INFO)
    _logger.addHandler(handler)

    _net_up = _check_net()
    if _net_up:
        info('WEB', 'Network OK')
    else:
        error('WEB', 'Network DOWN at startup')

    threading.Thread(target=_net_monitor, daemon=True, name='net-monitor').start()
