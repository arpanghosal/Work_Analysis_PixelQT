#!/usr/bin/env python

import os, sys
import signal           # to catch signals
import subprocess       # to run multiple skimmers
from time import sleep  # mostly for waiting


# A short class to manage processes. To be initialised with the
# limit for the number of processes executed simultaneously.
class ProcessManager():
    def __init__(self, limit):
        self.processes = []
        self.proc_limit = limit
        self.running = True

    # Add a process to the manager, given a command string.
    def addProcess(self, command):
        if not self.running: return
        self.processes.append(subprocess.Popen([command], shell=True, preexec_fn=os.setsid))

    # Kill all processes currently managed.
    def killAll(self):
        self.running = False
        print "Received kill signal. Killing all subprocesses, please wait ..."
        print "If this fails, please kill these IDs manually: %s" % [os.getpgid(p.pid) for p in self.processes]
        for p in self.processes:
            os.killpg(os.getpgid(p.pid), signal.SIGTERM)

    # Remove a finished process from the internal process list.
    def removeFinishedProcs(self):
        for p in self.processes:
            if not p.poll() == None:
                self.processes.remove(p)

    # Wait until the process queue is below the initialised limit
    # of simultaneous processes.
    def waitTillReady(self):
        while True:
            self.removeFinishedProcs()
            if len(self.processes) >= self.proc_limit:
                try:
                    sleep(1)
                except KeyboardInterrupt:
                    self.killAll()
                    sys.exit(1)
            else:
                break
        return

    # Wait until all processes are finished and return.
    def finalise(self):
        while True:
            self.removeFinishedProcs()
            if len(self.processes) > 0:
                try:
                    sleep(1)
                except KeyboardInterrupt:
                    self.killAll()
                    sys.exit(1)
            else:
                break
        return
