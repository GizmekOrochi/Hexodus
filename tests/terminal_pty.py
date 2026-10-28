#!/usr/bin/env python3
"""Integration tests of output bytes; no evdev, GUI, or root required."""
import os, pty, fcntl, termios, struct, subprocess, select
from pathlib import Path
root=Path(__file__).resolve().parents[1]
binary=root/'bin/terminal-probe'
def run(term, sync, size, resized):
    master,slave=pty.openpty()
    def resize(s):fcntl.ioctl(slave,termios.TIOCSWINSZ,struct.pack('HHHH',*s,0,0))
    env=dict(os.environ,TERM=term,LANG='C.UTF-8',HEXODUS_SYNC=sync,HEXODUS_COLORS='auto',HEXODUS_GLYPHS='auto')
    resize(size)
    process=subprocess.Popen([binary],stdin=subprocess.PIPE,stdout=slave,stderr=subprocess.PIPE,env=env)
    output=b''
    try:
        assert select.select([master],[],[],5)[0], 'No first frame'
        output+=os.read(master,65536)
        resize(resized)
        process.communicate(b'\n',timeout=5)
        assert process.returncode==0
        while select.select([master],[],[],0.1)[0]:output+=os.read(master,65536)
        assert b'\x1b[?25l' in output and b'\x1b[?25h' in output
        assert output.endswith(b'\x1b[?1049l') if term!='linux' else output.endswith(b'\x1b[?25h')
        assert (b'\x1b[?2026h' in output)==(sync=='1' and term!='linux')
        assert (b'\x1b[?1049h' in output)==(term!='linux')
        if resized[0]>0:assert ('\x1b[%d;1H'%resized[0]).encode() in output
    finally:
        if process.poll() is None:process.kill();process.wait()
        os.close(master);os.close(slave)
for args in [('xterm-256color','0',(2,3),(4,5)),('xterm-256color','1',(0,0),(1,1)),('linux','1',(3,4),(1,1))]:run(*args)
p=subprocess.run([binary],capture_output=True,env=dict(os.environ,TERM='xterm'))
assert p.returncode==1 and not p.stdout, 'Redirected stdout must fail without escape codes'
print('4 PTY/output scenarios passed')
