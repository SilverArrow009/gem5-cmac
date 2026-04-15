# Custom processor model configuration
import argparse
import os

from m5.objects import (
    MinorDefaultFloatSimdFU,
    MinorDefaultFUPool,
    MinorDefaultIntDivFU,
    MinorDefaultIntFU,
    MinorDefaultIntMulFU,
    MinorDefaultMemFU,
    MinorDefaultMiscFU,
    MinorDefaultPredFU,
    MinorFU,
    MinorFUPool,
    MinorFUTiming,
    MinorOpClass,
    MinorOpClassSet,
    RiscvMinorCPU,
)

from gem5.components.boards.simple_board import SimpleBoard
from gem5.components.cachehierarchies.classic.private_l1_private_l2_cache_hierarchy import (
    PrivateL1PrivateL2CacheHierarchy,
)
from gem5.components.memory import SingleChannelDDR3_1600
from gem5.components.processors.base_cpu_core import BaseCPUCore
from gem5.components.processors.base_cpu_processor import BaseCPUProcessor
from gem5.isas import ISA
from gem5.resources.resource import BinaryResource
from gem5.simulate.simulator import Simulator


# Function to create OpClass set from list of strings
def minorMakeOpClassSet(op_classes):
    return MinorOpClassSet(
        opClasses=[MinorOpClass(opClass=o) for o in op_classes]
    )


# Fixed Memory FU that includes segmented load/store
class FixedMinorDefaultMemFU(MinorDefaultMemFU):
    opClasses = minorMakeOpClassSet(
        [
            "MemRead",
            "MemWrite",
            "FloatMemRead",
            "FloatMemWrite",
            "SimdUnitStrideLoad",
            "SimdUnitStrideStore",
            "SimdUnitStrideMaskLoad",
            "SimdUnitStrideMaskStore",
            "SimdStridedLoad",
            "SimdStridedStore",
            "SimdIndexedLoad",
            "SimdIndexedStore",
            "SimdUnitStrideFaultOnlyFirstLoad",
            "SimdWholeRegisterLoad",
            "SimdWholeRegisterStore",
            # Added segmented load/store
            "SimdUnitStrideSegmentedLoad",
            "SimdUnitStrideSegmentedStore",
            "SimdUnitStrideSegmentedFaultOnlyFirstLoad",
            "SimdStrideSegmentedLoad",
            "SimdStrideSegmentedStore",
        ]
    )


# Define custom functional unit for complex operations
class MinorComplexFU(MinorFU):
    opClasses = minorMakeOpClassSet(
        ["SimdComplexMult", "SimdComplexDiv", "SimdComplexMultAcc"]
    )
    timings = [
        MinorFUTiming(description="ComplexSimd", srcRegsRelativeLats=[2])
    ]
    opLat = 6


# Create a custom FU pool by adding the complex FU and fixed mem FU to the default pool
class CustomFUPool(MinorFUPool):
    def __init__(self):
        super().__init__()
        self.funcUnits = [
            MinorDefaultIntFU(),
            MinorDefaultIntFU(),
            MinorDefaultIntMulFU(),
            MinorDefaultIntDivFU(),
            MinorDefaultFloatSimdFU(),
            MinorDefaultPredFU(),
            FixedMinorDefaultMemFU(),
            MinorDefaultMiscFU(),
            MinorComplexFU(),
        ]


class CustomCore(BaseCPUCore):
    def __init__(self, cpu_id):
        core = RiscvMinorCPU(cpu_id=cpu_id)
        # Apply the custom functional unit pool
        core.executeFuncUnits = CustomFUPool()
        super().__init__(core=core, isa=ISA.RISCV)
        # VLEN=1024, ELEN=64 as per requirements
        self.core.isa[0].vlen = 1024
        self.core.isa[0].elen = 64


def run_simulation(binary_path):
    # Cache hierarchy: 32kB L1I, 32kB L1D, 512kB L2
    cache_hierarchy = PrivateL1PrivateL2CacheHierarchy(
        l1d_size="32KiB", l1i_size="32KiB", l2_size="512KiB"
    )

    # Memory: 1GB DDR3_1600
    memory = SingleChannelDDR3_1600("1GiB")

    # Single core processor with Custom core
    processor = BaseCPUProcessor(cores=[CustomCore(0)])

    # Board setup
    board = SimpleBoard(
        clk_freq="1GHz",
        processor=processor,
        memory=memory,
        cache_hierarchy=cache_hierarchy,
    )

    # Set workload
    board.set_se_binary_workload(BinaryResource(local_path=binary_path))

    # Run simulation
    simulator = Simulator(board=board)
    print(f"Beginning simulation of {binary_path} on custom model!")
    simulator.run()


if __name__ == "__m5_main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("binary", type=str, help="Path to the binary to run")
    args = parser.parse_args()
    run_simulation(args.binary)
elif __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("binary", type=str, help="Path to the binary to run")
    args = parser.parse_args()
    run_simulation(args.binary)
