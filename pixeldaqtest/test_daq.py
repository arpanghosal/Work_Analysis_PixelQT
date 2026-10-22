from apscheduler.schedulers.background import  BackgroundScheduler
from apscheduler.events import  EVENT_JOB_ERROR,EVENT_JOB_EXECUTED
from apscheduler.schedulers import base as SchedulerBase
from ispy import * 
from pytz import utc
import time
import datetime
from core.tools.partition import get_partition, get_daqpartition, initialize_partitions
from core.daqtest import TestBase, FAIL, PASS

from testlibrary.testconditions import TestCond

from testlibrary.testbusy import TestBusy 
from testlibrary.testtimeout import TestTimeout

from testlibrary.testfragjson import TestFragError
from testlibrary.testrodmonjson import TestRodMon

from testlibrary.testrodmaster import TestRodMaster
from testlibrary.testrodslave import TestRodSlave

from testlibrary.testbocbcf import TestBocBcf
from testlibrary.testbocbmf import TestBmf
from testlibrary.testboctx import TestTx

from apscheduler.executors.pool import ProcessPoolExecutor
import sys
import subprocess
import datetime


# Start the scheduler
scheduler = BackgroundScheduler()



#jobstores = {
#    'mongo': {'type': 'mongodb'},
#    'default': SQLAlchemyJobStore(url='sqlite:///jobs.sqlite')
#}
executors = {
    'default': {'type': 'threadpool', 'max_workers': 20},
    'processpool': ProcessPoolExecutor(max_workers=5)
}
job_defaults = {
    'coalesce': False,
    'max_instances': 3
}

# .. do something else here, maybe add jobs etc.

scheduler.configure(executors=executors, job_defaults=job_defaults, timezone=utc)
scheduler = BackgroundScheduler()

def my_listener(event):
    if event.exception:
        print ('The job crashed :(')

scheduler.add_listener(my_listener, EVENT_JOB_EXECUTED | EVENT_JOB_ERROR)
import logging

logging.getLogger('apscheduler').setLevel(logging.ERROR)
logging.basicConfig(format='%(asctime)s - %(message)s', datefmt='%d-%b-%y %H:%M:%S')


# Define a custom logger
logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)


# Handlers
c_handler = logging.StreamHandler()
f_handler = logging.FileHandler("file.log")
c_handler.setLevel(logging.DEBUG)
f_handler.setLevel(logging.ERROR)


# Formaters
c_format = logging.Formatter('%(name)s - %(levelname)s - %(message)s')
f_format = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
c_handler.setFormatter(c_format)
f_handler.setFormatter(f_format)

# Add handler
#logger.addHandler(c_handler)
logger.addHandler(f_handler)


######################################################################

class TestEngine:

    def __init__(self, 
            partition='PixelInfr_okepka', 
            daqpartition='SR1-DAQSlice_okepka',
            controller='RCDPixelTtcCrateSr1', 
            emulator=False):
        self.scheduler = scheduler
        self.jobs_running = []
        self.partition = partition
        self.daqpartition = daqpartition
        self.controller = controller
        self.emulator = emulator        # If true, does not propagate any command to controller
#        self.trigcmd = 'trg_command_trigger -p {} -n {}')

    ### Add job to a scheduler inside TestEngine
    def add_job(self, *args, **kwargs):
       self.jobs_running.append(self.scheduler.add_job(*args, **kwargs) )

    def enter_running(self):

        if self.scheduler.state == SchedulerBase.STATE_STOPPED:
            logger.info('Starting scheduler')
#            self.jobs_running = add_jobs_for_running(scheduler)
            self.scheduler.start()
        else:

            logger.debug('Resuming jobs')
            for job in self.jobs_running:
                job.resume()

    def exit_running(self):
        logger.debug('Pause jobs')
        for job in self.jobs_running:
            job.pause()

    def trig_hold(self):
        logger.info('Hold trigger')
        if not self.emulator:
            subprocess.check_output(['bash','-c', "trg_command_trigger -p {} -n {} -c HOLD".format(self.daqpartition, self.controller)])

    def trig_release(self):
        logger.info('Release trigger')
        if not self.emulator:
            subprocess.check_output(['bash','-c', "trg_command_trigger -p {} -n {} -c RESUME".format(self.daqpartition, self.controller)])

    def enter_hold_user(self):
        logger.info('User has held a trigger')



from transitions import Machine
from transitions import State
#states=['stopped', 'running', 'hold', 'hold_user']
states=['stopped', 
        State(name='running', on_enter=['enter_running'], on_exit=['exit_running']),
        State(name='hold', on_enter=['trig_hold'], on_exit=['trig_release']),
        State(name='hold_user', on_enter=['enter_hold_user'])
        ]
#machine.on_enter_running('enter_running')
#machine.on_exit_running('exit_running')
#machine.on_enter_hold('trig_hold')
#machine.on_exit_hold('trig_release')
#machine.on_enter_hold_user('enter_hold_user')

transitions = [
        {'trigger': 'start', 'source': 'stopped', 'dest': 'running'},
        {'trigger': 'hold', 'source': 'running', 'dest': 'hold'},
        {'trigger': 'release', 'source': 'hold', 'dest': 'running'},
        {'trigger': 'hold_user', 'source': 'running', 'dest': 'hold_user'},
        {'trigger': 'stop', 'source': 'running', 'dest': 'stopped'}
        ]


machine = Machine(model=TestEngine(),
        states=states,
        transitions=transitions,
        initial='stopped'
        )



######################################################################
# Example of a dummy test


class TestHoldDummy(TestBase):
    def _init__(self, rod):
        self.requires_is = True

    def initialize():
        pass

    def execute(self):
        logger.debug('First test during hold')
        logger.debug('Second test during hold')

######################################################################

from collections import OrderedDict 

# collections of actions that can be performed
class Actions:
    def __init__(self, name, rods):
        self.rods = rods
        self.name = name
        self.test_collection = OrderedDict()

    def register_test(self, testClass):
        #self.test_classes.append(testClass)
        self.test_collection[testClass] = [ testClass(rod) for rod in self.rods ]
        
    @staticmethod
    def before_execute():
        pass

    @staticmethod
    def after_execute():
        pass

    def action(self):

        self.before_execute()
        logger.debug("after_execute")

        res = True
        for testClass, tests in self.test_collection.items():
            for test in tests:
                if test.execute() == FAIL:
                    test.error_message()
                    res = False

            if res:
                logger.info(['Test ', testClass.__name__, 'passed'])

        logger.debug("after_execute")
        
        self.after_execute()

        
        
class ActionsSlow(Actions):

    @staticmethod
    def before_execute():
        logger.debug('before_execute')
        testengine.hold()
        logger.debug('Sleeping for 2 secs after hold')
        time.sleep(2)

	####initial value for Xoff verification####

	if (os.path.exists('xoff.txt')):
	    pass
	else:
	  d = ISInfoDictionary(get_daqpartition())
	  IS_val = ISInfoDynAny()
	  d.getValue('DF.ROS.Appl_PixelROS05.ReadoutModule0', IS_val)
	  xoff=IS_val[16]
	  tmp=1
	  xoff = [tmp] + xoff
	  with open("xoff.txt", "w") as f:
	        print >> f, (xoff)

	  


    @staticmethod
    def after_execute():
        logger.debug('after_execute')
        testengine.release()

            






#def assynchronous_actions():
#
#    if am.current_state == AM_running:
#
#        # run tests or check for user action to hold a trigger
#        
#        # user has externally asked to hold the trigger
#        if(is_get_trig_state() == 'IS_TRIG_HOLD'):
#            am.go_hold_user()
#
#            if( hasattr(actions, 'tests_at_hold_user')):
#                actions.test_at_hold_user()
#                return 
#
#        if( hasattr(actions, 'execute_capture')):
#            if(execute_capture()):
#                am.go_hold()
#                actions.execute_tests_capture()
#                am.go_running()
#            return
#
#    if am.current_state == AM_trig_hold_user:
#        if(is_get_trig_state == IS_TRIG_RUNNING):
#            am.go_running()
#


def add_jobs_for_running(scheduler):
    jobs = []

    return jobs


if __name__=='__main__':

    
    import argparse
    import sys, os
    try: import argcomplete
    except: pass
    class CustomFormatter(argparse.ArgumentDefaultsHelpFormatter):
        pass

    class _HelpAction(argparse._HelpAction):
        def __call__(self, parser, namespace, values, option_string=None):
           parser.print_help()
           parser.exit()

    parser = argparse.ArgumentParser(add_help=True, description='Pixel DAQ Test Framework',
            formatter_class=lambda prog: CustomFormatter(prog, max_help_position=30),
            usage="%(prog)s --rods rod1[,rod2, ...] ",
            epilog='User is expected to clone this example and taylor the test to his needs.')

    parser_environ = parser.add_argument_group('environment configuration')
    parser_devices = parser.add_argument_group('device configuration')
    parser_setup = parser.add_argument_group('testing setup')

    parser_environ.add_argument("--part",                   help="Specify daq partition. Default $PIX_PART_INFR_NAME", type=str, default=os.environ['PIX_PART_INFR_NAME'])
    parser_environ.add_argument("--daqpart",                   help="Specify daq partition. Default $PIX_PART_DD_NAME", type=str, default=os.environ['PIX_PART_DD_NAME'])
    parser_environ.add_argument("--controller", choices=('RCDPixelTtcCrateSr1', 'TODO_FORPIT' ), default='RCDPixelTtcCrateSr1', help="Sepcify RCD controller RCDPixelTtcCrateSr1 or  ... for pit ")

    parser_devices.add_argument("--rods", type=str, nargs='+', required=True, help="Specify rods to test")
    parser_devices.add_argument("--rolenable", action='store_true',  default=False, help="Take rod list from list of enabled readout links")

    parser_devices.add_argument("--emulator", action='store_true', default=False, help="Run in emulator mode (no STATE transition propagated to infrastructure)")

    try: argcomplete.autocomplete(parser)
    except: pass
    args = parser.parse_args()

    print("Using rods: ", args.rods)

    
    #initialize partition
    initialize_partitions(args.part, args.daqpart)
    testengine = machine.model
    testengine.emulator = args.emulator

    testengine.partition = args.part
    testengine.daqpartition = args.daqpart

    # User definition of the action environment in which tests are running in them
    rods = args.rods

    actions_fast = Actions(name='actions_fast', rods=rods)
    actions_fast.register_test(TestBusy)
    actions_fast.register_test(TestTimeout)


    actions_slow = ActionsSlow(name='actions_slow', rods=rods)
    #actions_slow.register_test(TestHoldDummy)

    actions_onetime.register_test(TestCond)

    actions_slow.register_test(TestFragError)
    actions_slow.register_test(TestRodMon)

    actions_slow.register_test(TestRodMaster)
    actions_slow.register_test(TestRodSlave)

    actions_slow.register_test(TestBocBcf)
    actions_slow.register_test(TestBmf)
    actions_slow.register_test(TestTx)

    testengine.add_job(actions_fast.action, 'interval', seconds = 2)
    testengine.add_job(actions_slow.action, 'interval', seconds = 20)

    testengine.start()

    while True:
        time.sleep(1)
