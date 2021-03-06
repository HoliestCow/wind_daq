
from threading import Thread, Event
import time


class StoppableThread(Thread):
    """
    Thread class with a stop() method. The thread itself has to check
    regularly for the stopped() condition.

    Thread has to look at self._stop_event to know whether to keep going or not.
    """

    def __init__(self, target=None, args=None, name=None):
        super(StoppableThread, self).__init__(
            target=target, args=args, name=name)
        self._stop_event = Event()

    def stop(self):
        self._stop_event.set()

    def stopped(self):
        return self._stop_event.is_set()
