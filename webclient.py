#!/usr/bin/python3

# Utility to connect a Zuniq player that implements that 2021 CAIA protocol to
# my game framework, which uses HTTP POST request and HTTP even streams.
#
# Warning: the code here is pretty ugly!
#
# Example use:
#
# read -r session && ./webclient.py --session "$session" ~/caia/zuniq/bin/player
# http://localhost:8080/session.html#gameId=zuniq&gameName=zuniq&sessionId=6dcf428d848ff9a8a60fd1e043f4bf5f&playerId=1&playerKey=e4ed97c547c41d16c87b98c73dbfc46f

import argparse
import json
import re
import socket
import subprocess
import sys
from urllib.parse import urlparse, urljoin, quote, unquote
from urllib.request import urlopen, Request
from urllib.error import HTTPError

argument_parser = argparse.ArgumentParser(description='Connects a Zuniq player using the CodeCup 2021 protocol to the GameFrame server')
argument_parser.add_argument('--session', type=str, nargs=1, help='Session URL', required=True)
argument_parser.add_argument('command', type=str, nargs=1, help='Path to player executable')
argument_parser.add_argument('arg', type=str, nargs='*', help='Player arguments')

GAME_ID = 'zuniq'
SESSION_URL_TEMPLATE = 'api/sessions/{sessionId}?playerKeys={playerKey}'
EVENT_STREAM_SUFFIX = '&format=event-stream'

MOVE_PATTERN = re.compile("^([A-Z])(\d+)([hv])(!)?$")
HEIGHT = 5
WIDTH = 5

def ParseCaiaMove(s):
    '''Parses a Caia move string, and returns pair of the move number and
        boolean that indicates if client calls victory.
        Example: ParseCaiaMove("C1v!") == (27, True)'''
    match = MOVE_PATTERN.match(s)
    assert match
    row, col, dir, win = match.groups()
    row = ord(row) - ord('A')
    col = int(col) - 1
    win = win is not None
    if dir == 'h':
        assert 0 <= row <= HEIGHT and 0 <= col < WIDTH
        move = (WIDTH * 2 + 1)*row + col
    if dir == 'v':
        assert 0 <= row < HEIGHT and 0 <= col <= WIDTH
        move = (WIDTH * 2 + 1)*row + WIDTH + col
    return move, win

def FormatCaiaMove(i):
    '''Formats a move number as a Caia move.
        Example: FormatCaiaMove(27) == "C1v"'''
    assert 0 <= i < HEIGHT * WIDTH * 2 + HEIGHT + WIDTH
    row = i // (WIDTH * 2 + 1)
    col = i % (WIDTH * 2 + 1)
    if col < WIDTH:
        dir = 'h'
    else:
        col -= WIDTH
        dir = 'v'
    return chr(ord('A') + row) + str(col + 1) + dir

def ReadEventStream(f):
    data = b''
    while True:
        line = f.readline()
        if not line:
            break
        if line.startswith(b'data:'):
            if len(line) > 5 and line[5] == b' ':
                data += line[6:]
            else:
                data += line[5:]
        elif not line.strip():
            if data:
                yield data.decode('utf-8')
                data = b''

def PlayGame(command_args, session_url, event_stream_url, player_id, player_key):
    popen = subprocess.Popen(args=command_args, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    moves = []
    for update in ReadEventStream(urlopen(event_stream_url)):
        update = json.loads(update)
        if update['gameId'] != GAME_ID:
            print('Update has incorrect gameId!')
            sys.exit(1)
        new_moves = update['state']['pastMoves']
        assert len(new_moves) >= len(moves)
        assert new_moves[:len(moves)] == moves
        while len(moves) < len(new_moves):
            move = new_moves[len(moves)]
            caia_move = FormatCaiaMove(move)
            print('Sent', caia_move)
            popen.stdin.write((caia_move + '\n').encode('utf-8'))
            popen.stdin.flush()
            moves.append(move)
        if update['activePlayers'] == []:
            popen.stdin.write(b'Quit\n')
            popen.stdin.close()
            popen.wait()
            return moves
        if update['activePlayers'] == [player_id]:
            if not moves:
                popen.stdin.write(b'Start\n')
                popen.stdin.flush()
            caia_move = popen.stdout.readline().decode('utf-8').strip()
            print('Received', caia_move)
            move, win = ParseCaiaMove(caia_move)
            if win:
                print('Player', player_id, 'claims victory!')
            update = {
                'moveCount': len(moves),
                'playerKey': player_key,
                'player': player_id,
                'move': move
            }
            request = Request(session_url)
            request.add_header('Content-Type', 'application/json; charset=utf-8')
            try:
                urlopen(request, json.dumps(update).encode('utf-8'))
            except HTTPError as e:
                print(e)
                print(e.read().decode('utf-8'))
                sys.exit(1)
            moves.append(move)


def Main():
    args = argument_parser.parse_args()
    fragment = urlparse(args.session[0]).fragment
    if not fragment:
        print('Session URL is missing a URL fragment!')
        sys.exit(1)
    params = {}
    for part in fragment.split('&'):
        if '=' in part:
            key, value = map(unquote, part.split('=', 1))
            params[key] = value
    for key in ('gameId', 'sessionId', 'playerId', 'playerKey'):
        if key not in params:
            print('Session URL is missing required parameter [{}]'.format(key))
            sys.exit(1)
    if params['gameId'] != GAME_ID:
        print('gameId must be [{}]'.format(GAME_ID))
        sys.exit(1)
    if params['playerId'] not in ('1', '2'):
        print('playerId must be [1] or [2]')
        sys.exit(1)

    session_url = urljoin(
        args.session[0],
        SESSION_URL_TEMPLATE.format(
            playerKey=quote(params['playerKey']),
            sessionId=quote(params['sessionId'])))
    event_stream_url = session_url + EVENT_STREAM_SUFFIX

    moves = PlayGame(args.command + args.arg, session_url, event_stream_url, params['playerId'], params['playerKey'])
    winner = 2 - len(moves) % 2
    print('Player', winner, 'won')

if __name__ == '__main__':
    Main()
