from ipc import IPCPartition
import sys

partition = None
daqpartition = None


class PartitionError(Exception):
    """ Errors related to not initialized partitions """
    pass


def initialize_partitions(partition_name, daqpartition_name):

    global partition
    global daqpartition
    partition = IPCPartition(partition_name)
    if not partition.isValid():
        print("Could not initialize partition: "+partition_name)
        sys.exit(1)

    daqpartition = IPCPartition(daqpartition_name)
    if not daqpartition.isValid():
        print("Could not initialize partition: "+ daqpartition_name)
        sys.exit(1)



def get_partition():
    global partition
    if partition == None:
        raise PartitionError('Partition not initialized')
    return partition


def get_daqpartition():
    global daqpartition
    if daqpartition == None:
        raise PartitionError('DAQ partition not initialized')
    return daqpartition



