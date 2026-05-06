import argparse

from m5.objects import (
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


def minorMakeOpClassSet(op_classes):
    return MinorOpClassSet(
        opClasses=[MinorOpClass(opClass=o) for o in op_classes]
    )

# CVA6 SCALAR FUNCTIONAL UNITS (3 Units)

# CVA6 params are extracted from prior work at: https://upcommons.upc.edu/server/api/core/bitstreams/27f7e723-3a48-430b-baf2-be9920fe145a/content

# Note the latencies are sufficently adjusted according to our model
# Unlike the work of the master thesis that adds an explicit 2-cycle delay to the 4-stage pipeline
# of the MinorCPU, we adjust the delay between the stages itself, especially the fetch2 -> decode and decode -> execute stages 

class CVA6_IntALUFU(MinorFU):
    opClasses = minorMakeOpClassSet(["IntAlu"])
    opLat = 1
    issueLat = 1

class CVA6_IntMulFU(MinorFU):
    opClasses = minorMakeOpClassSet(["IntMult"])
    opLat = 2
    issueLat = 1

class CVA6_IntDivFU(MinorFU):
    opClasses = minorMakeOpClassSet(["IntDiv"])
    opLat = 33
    issueLat = 33

class CVA6_FloatALUFU(MinorFU):
    opClasses = minorMakeOpClassSet(["FloatAdd", "FloatCmp", "FloatCvt"])
    opLat = 1
    issueLat = 1
    timings = [
        MinorFUTiming(description="Scalar Float compare",
            opClasses=minorMakeOpClassSet(["FloatCmp"]), srcRegsRelativeLats=[2], extraAssumedLat=2),
        MinorFUTiming(description="Scalar Float convert",
            opClasses=minorMakeOpClassSet(["FloatCvt"]), srcRegsRelativeLats=[2], extraAssumedLat=2)
    ]

class CVA6_FloatMulFU(MinorFU):
    opClasses = minorMakeOpClassSet(["FloatMult", "FloatMultAcc", "FloatMisc"])
    opLat = 2
    issueLat = 1

class CVA6_FloatDivFU(MinorFU):
    # Divider is sequential and unpipelined, note that issueLat != 1
    # According to the document, the divider latency is just 4 which (I feel) is incredibly inaccurate
    # Typical floating point dividers take around 15 - 25 cycles for a double precision division
    opClasses = minorMakeOpClassSet(["FloatDiv", "FloatSqrt"])
    opLat = 20
    issueLat = 20

class CVA6_MemFU(MinorFU):
    opClasses = minorMakeOpClassSet(["MemRead", "MemWrite", "FloatMemRead", "FloatMemWrite"])
    opLat = 2
    issueLat = 1

class CVA6_MiscFU(MinorFU):
    opClasses = minorMakeOpClassSet(["InstPrefetch", "System"])
    opLat = 1
    issueLat = 1


# ARA VECTOR FUNCTIONAL UNITS (3 Units)
# Base Transfer Delay = 4 cycles (1 accelerator dispatch + 1 interconnect + 1 vector dispatcher + 1 vector sequencer stages)

class Ara_SimdIntAluFU(MinorFU):
    """Handles vector integer, reduction, permutation, and slide operations."""
    # Base Int ALU = 1 cycle. 
    # Penalty = 2 (For in-Lane dispatcher) + 4 (Base Transfer delay) = 6 cycles base
    opClasses = minorMakeOpClassSet(["SimdAdd", "SimdAlu", "SimdCmp", "SimdCvt", "SimdExt", "SimdConfig", "SimdShift", "SimdMisc", "SimdAddAcc", "SimdReduceAdd", "SimdReduceAlu", "SimdReduceCmp", "SimdFloatMisc"])
    opLat = 7
    issueLat = 1
    timings = [
        MinorFUTiming(description="Vector Reductions (Sum)",
            # 6 (Transfer) + 4 (Reduction Penalty log2(1024/64)) = 10
            opClasses=minorMakeOpClassSet(["SimdReduceAdd"]), srcRegsRelativeLats=[2], extraAssumedLat=3)
        # We map permutations (vrgather) and slides to SimdMisc.
        # ARA has a giant barrel shifter so the shifting is independent of positions
        # Gather and shifting take 1 cycle each.
        # Base (2) + 4 (Transfer) + 1 (Slide) = 7 cycles
        # Note on Masked ALU: gem5 does not currently separate Masked vs Unmasked ALU operations 
        # into distinct Python opClasses (essentially, all masked instrucitons are handled by SimdMisc).
    ]

class Ara_SimdIntMulFU(MinorFU):
    """Handles vector multiply and MAC operations."""
    # Base Simd Int multiplier  = 3 cycle. 
    # Penalty = 2 (For in-Lane dispatcher) + 4 (Base Transfer delay) = 6 cycles base
    opClasses = minorMakeOpClassSet(["SimdMult", "SimdMultAcc"])
    opLat = 9
    issueLat = 1
    timings = [
        MinorFUTiming(description="Vector Accumulations (MAC)",
            # 6 (Transfer) + 3 (Mult) + 1 (Add/Sub) = 10
            opClasses=minorMakeOpClassSet(["SimdAddAcc", "SimdMultAcc"]), srcRegsRelativeLats=[2], extraAssumedLat=1),
    ]

class Ara_SimdIntDivFU(MinorFU):
    """Handles vector integer division and sqrt operations."""
    # Base Int Divider = 33 cycle. 
    # Penalty = 2 (For in-Lane dispatcher) + 4 (Base Transfer delay) = 6 cycles base
    # Base Div (33) + 6 (Transfer) = 39
    opClasses = minorMakeOpClassSet(["SimdDiv", "SimdSqrt"])
    opLat = 39
    issueLat = 39

class Ara_SimdFloatALUFU(MinorFU):
    """Handles all vector floating-point ALU operations."""
    # Base Float = 1 cycle. 
    # Penalty = 2 (For in-Lane dispatcher) + 4 (Base Transfer delay) = 6 cycles base
    opClasses = minorMakeOpClassSet(["SimdFloatAdd", "SimdFloatAlu", "SimdFloatCmp", "SimdFloatCvt", "SimdFloatReduceAdd", "SimdFloatReduceCmp", "SimdFloatMisc", "SimdFloatExt"])
    opLat = 7
    issueLat = 1
    timings = [
        MinorFUTiming(description="Vector Float comparison (FloatCmp)",
            # 6 (Transfer) + 3 = 9
            opClasses=minorMakeOpClassSet(["SimdFloatCmp"]), srcRegsRelativeLats=[2], extraAssumedLat=2),
        MinorFUTiming(description="Vector Float convert (FloatCvt)",
            # 6 (Transfer) + 2 = 8
            opClasses=minorMakeOpClassSet(["SimdFloatCmp"]), srcRegsRelativeLats=[2], extraAssumedLat=1),
        MinorFUTiming(description="Vector Float Reductions (VFSum)",
            # 6 (Transfer) + 4 * 4 (Reduction Penalty log2(1024/64) * lat(fadd)) = 22
            opClasses=minorMakeOpClassSet(["SimdFloatReduceAdd"]), srcRegsRelativeLats=[2], extraAssumedLat=12)
    ]

class Ara_SimdFloatMulFU(MinorFU):
    """Handles all vector floating-point multiplications and MAC operations."""
    # Base Float multiplicaiton = 1 cycles. 
    # Penalty = 2 (For in-Lane dispatcher) + 4 (Base Transfer delay) = 6 cycles base
    opClasses = minorMakeOpClassSet(["SimdFloatMult", "SimdFloatMultAcc"])
    opLat = 7
    issueLat = 1
    timings = [
        MinorFUTiming(description="Vector Float Accumulations (VFMAC)",
            # 6 (Transfer) + 1 (Mult) + 1 (Add/Sub) = 8
            opClasses=minorMakeOpClassSet(["SimdFloatMultAcc"]), srcRegsRelativeLats=[2], extraAssumedLat=1)
    ]

class Ara_SimdFloatDivFU(MinorFU):
    """Handles all vector floating-point division and sqrt operations."""
    # Base Float division = 20 cycles. 
    # Penalty = 2 (For in-Lane dispatcher) + 4 (Base Transfer delay) = 6 cycles base
    # Iterative Div (20) + 4 (Transfer) + 2 (ALU Penalty) = 26
    opClasses = minorMakeOpClassSet(["SimdFloatDiv", "SimdFloatSqrt"])
    opLat = 26
    issueLat = 26

class Ara_MemStridedFU(MinorFU):
    """Handles vector memory operations (Unit-stride, Strided)."""
    # Base Mem Issue = 2 cycles. 
    # Penalty +0 (Load/Store) + 4 (Transfer) = 6 cycles base
    opClasses = minorMakeOpClassSet([
        "SimdUnitStrideLoad", "SimdUnitStrideStore", "SimdUnitStrideMaskLoad", "SimdUnitStrideMaskStore",
        "SimdStridedLoad", "SimdStridedStore", "SimdUnitStrideFaultOnlyFirstLoad",
        "SimdWholeRegisterLoad", "SimdWholeRegisterStore", "SimdUnitStrideSegmentedLoad", "SimdUnitStrideSegmentedStore", "SimdStrideSegmentedLoad", "SimdStrideSegmentedStore", "SimdUnitStrideSegmentedFaultOnlyFirstLoad"
    ])
    opLat = 6
    issueLat = 1
    timings = [
        MinorFUTiming(description="Vector Strided Mem",
            # Base (2) + 4 (Transfer) + 1 (Stride Penalty) = 7
            opClasses=minorMakeOpClassSet(["SimdStridedLoad", "SimdStridedStore"]), srcRegsRelativeLats=[1], extraAssumedLat=1)
    ]

class Ara_MemIndexededFU(MinorFU):
    """Handles vector memory operations (Indexed)."""
    # Base Mem Issue = 2 cycles. 
    # Indexed mem acts like a permutation (gather/scatter). 
    # We apply the permutation penalty: Base (2) + 4 (Transfer) + 16 (VLEN=1024/ELEN=64) = 22
    opClasses = minorMakeOpClassSet(["SimdIndexedLoad", "SimdIndexedStore"])
    opLat = 22
    issueLat = 1


class UnifiedCVA6AraFUPool(MinorFUPool):
    funcUnits = [
        CVA6_IntALUFU(), CVA6_IntMulFU(), CVA6_IntDivFU(), CVA6_MemFU(),
        CVA6_FloatALUFU(), CVA6_FloatMulFU(), CVA6_FloatDivFU(),CVA6_MiscFU(),
        Ara_SimdIntAluFU(), Ara_SimdIntMulFU(), Ara_SimdIntDivFU(),
        Ara_SimdFloatALUFU(), Ara_SimdFloatMulFU(), Ara_SimdFloatDivFU(),
        Ara_MemStridedFU(), Ara_MemIndexededFU()
    ]


class HeterogeneousCore(BaseCPUCore):
    def __init__(self, cpu_id):
        core = RiscvMinorCPU(cpu_id=cpu_id)
        core.executeFuncUnits = UnifiedCVA6AraFUPool()

        # Expand Decode/Execute to model CVA6 6-stage depth
        # This delay has been added to model the extra delay in the execute stage (execute+commit)
        core.fetch2ToDecodeForwardDelay = 2
        core.decodeToExecuteForwardDelay = 2

        super().__init__(core=core, isa=ISA.RISCV)
        self.core.isa[0].vlen = 1024
        self.core.isa[0].elen = 64


def run_simulation(binary_path, binary_opts):
    cache_hierarchy = PrivateL1PrivateL2CacheHierarchy(
        l1d_size="32KiB", l1i_size="32KiB", l2_size="512KiB"
    )
    memory = SingleChannelDDR3_1600("1GiB")
    processor = BaseCPUProcessor(cores=[HeterogeneousCore(0)])
    board = SimpleBoard(
        clk_freq="1GHz", processor=processor, memory=memory, cache_hierarchy=cache_hierarchy,
    )
    if binary_opts is not None:
      board.set_se_binary_workload(BinaryResource(local_path=binary_path), arguments=binary_opts)
    else:
      board.set_se_binary_workload(BinaryResource(local_path=binary_path))
    simulator = Simulator(board=board)
    print("Beginning simulation with CVA6 + Ara baseline model!")
    simulator.run()


if __name__ in ("__m5_main__"):
    parser = argparse.ArgumentParser()
    parser.add_argument("binary", type=str, help="Path to the binary to run")
    parser.add_argument("--bin-args", type=str, help="Arguments to the binary to run")
    # Split the space separated string into a list
    args = parser.parse_args()
    # Differentiate whether the arguments to the binary have been passed
    if args.bin_args is not None:
      bin_opts = args.bin_args.split()
    else:
      bin_opts = None
    run_simulation(args.binary, bin_opts)
